#include "stdafx.h"
#include "Session.h"
#include "IOManager.h"
#include "OverlappedContext.h"


Session::~Session()
{
}

void Session::Initialize()
{
	// create socket
	// bind
	mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (mSocket == INVALID_SOCKET)
	{
		printf_s("Error in creating socket \n");
		return;
	}

	SOCKADDR_IN addr;
	SecureZeroMemory(&addr, sizeof(SOCKADDR_IN));
	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = INADDR_ANY;

	int iResult = 0;
	iResult = setsockopt(mSocket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
	int opt = 1;
	setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(int));

	if (bind(mSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		printf_s("Error in binding Socket \n");
		return;
	}

	

	HANDLE port = g_IoManager->GetCompletionPort();
	if (port != CreateIoCompletionPort((HANDLE)mSocket, port, 0, 0))
	{
		printf_s("Error in assigning IOCP to Socket \n");
		return;
	}

}

void Session::ConnectAndStart()
{
	SOCKADDR_IN addr;
	SecureZeroMemory(&addr, sizeof(SOCKADDR_IN));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(mPort);
	addr.sin_addr.s_addr = inet_addr(mIpStr);

	OverlappedContext* ol = new OverlappedContext(IO_CONNECT, this);



	// call connectEX
	BOOL connectResult = g_IoManager->FnConnectEx(
		mSocket,
		(SOCKADDR*)&addr,
		sizeof(addr),
		NULL,
		0,
		NULL,
		(LPOVERLAPPED)ol
		);

	if (connectResult)
	{
		printf_s("ConnectEx succeeded right away! \n");
	}
	else if (WSAGetLastError() == ERROR_IO_PENDING)
	{
		//printf_s("ConnectEx pending... \n");
	}
	else
	{
		printf_s("ConnectEx failed, code:%d \n", WSAGetLastError());
	}
}

void Session::AddBytesToRecvCnt(unsigned long bytes)
{
	mRcvdBytes += bytes;
}

void Session::AddBytesToSendCnt(unsigned long bytes)
{
	mSentBytes += bytes;
}

void Session::Disconnect()
{
	OverlappedContext* olc = new OverlappedContext(IO_DISCONNECT, this);

	g_IoManager->FnDisconnectEx(mSocket, (LPOVERLAPPED)olc, TF_REUSE_SOCKET, 0);
}

void Session::PostRecv()
{
	OverlappedContext* olc = new OverlappedContext(IO_RECV, this);
	olc->mWsaBuf.len = CLIENT_RECV_BUF_SIZE;
	olc->mWsaBuf.buf = (char*)malloc(sizeof(char)* olc->mWsaBuf.len);

	DWORD recvbytes = 0;
	DWORD flags = 0;

	if (SOCKET_ERROR == WSARecv(mSocket, &olc->mWsaBuf, 1, &recvbytes, &flags, (LPWSAOVERLAPPED)olc, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			delete olc;
			printf_s("Error in Session::PostRecv : %d \n", WSAGetLastError());
			return;
		}
	}
}

void Session::SendData(char* buf, int len)
{
	OverlappedContext* olc = new OverlappedContext(IO_SEND, this);
	olc->mWsaBuf.len = len;
	olc->mWsaBuf.buf = (char*)malloc(sizeof(char)* len);
	memcpy(olc->mWsaBuf.buf, buf, len);

	DWORD sentBytes = 0;
	DWORD flags = 0;

	if (SOCKET_ERROR == WSASend(mSocket, &olc->mWsaBuf, 1, &sentBytes, flags, (LPWSAOVERLAPPED)olc, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			delete olc;
			printf_s("Error in Session::SendData : %d \n", WSAGetLastError());
			return;
		}
	}
}
