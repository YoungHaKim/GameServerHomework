#include "stdafx.h"
#include "Exception.h"
#include "Log.h"
#include "EduServer_IOCP.h"
#include "OverlappedIOContext.h"
#include "ClientSession.h"
#include "IocpManager.h"
#include "ClientSessionManager.h"
#include "Player.h"
#include "MyPacket.pb.h"
#include "FeedCenter.h"

#define CLIENT_BUFSIZE	65536

bool ClientSession::mShowConnect;

ClientSession::ClientSession() : Session(CLIENT_BUFSIZE, CLIENT_BUFSIZE), mPlayer(this)
{
	memset(&mClientAddr, 0, sizeof(SOCKADDR_IN));
}

ClientSession::~ClientSession()
{
}

void ClientSession::SessionReset()
{
	TRACE_THIS;

	mConnected = 0;
	mRefCount = 0;
	memset(&mClientAddr, 0, sizeof(SOCKADDR_IN));

	mRecvBuffer.BufferReset();

	mSendBufferLock.EnterLock();
	mSendBuffer.BufferReset();
	mSendBufferLock.LeaveLock();

	LINGER lingerOption;
	lingerOption.l_onoff = 1;
	lingerOption.l_linger = 0;

	/// no TCP TIME_WAIT
	if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (char*)&lingerOption, sizeof(LINGER)))
	{
		printf_s("[DEBUG] setsockopt linger option error: %d\n", GetLastError());
	}
	closesocket(mSocket);

	mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	mPlayer.PlayerReset();
}

bool ClientSession::PostAccept()
{
	CRASH_ASSERT(LThreadType == THREAD_MAIN);

	OverlappedAcceptContext* acceptContext = new OverlappedAcceptContext(this);
	DWORD bytes = 0;
	DWORD flags = 0;
	acceptContext->mWsaBuf.len = 0;
	acceptContext->mWsaBuf.buf = nullptr;

	if (FALSE == MyAcceptEx(*GIocpManager->GetListenSocket(), mSocket, GIocpManager->mAcceptBuf, 0,
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &bytes, (LPOVERLAPPED)acceptContext))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIoContext(acceptContext);
			printf_s("AcceptEx Error : %d\n", GetLastError());

			return false;
		}
	}

	return true;
}

void ClientSession::AcceptCompletion()
{
	TRACE_THIS;

	CRASH_ASSERT(LThreadType == THREAD_IO_WORKER);
	
	if (1 == InterlockedExchange(&mConnected, 1))
	{
		/// already exists?
		CRASH_ASSERT(false);
		return;
	}

	bool resultOk = true;
	do 
	{
		if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)GIocpManager->GetListenSocket(), sizeof(SOCKET)))
		{
			printf_s("[DEBUG] SO_UPDATE_ACCEPT_CONTEXT error: %d\n", GetLastError());
			resultOk = false;
			break;
		}

		int opt = 1;
		if (SOCKET_ERROR == setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(int)))
		{
			printf_s("[DEBUG] TCP_NODELAY error: %d\n", GetLastError());
			resultOk = false;
			break;
		}

		opt = 0;
		if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&opt, sizeof(int)))
		{
			printf_s("[DEBUG] SO_RCVBUF change error: %d\n", GetLastError());
			resultOk = false;
			break;
		}

		int addrlen = sizeof(SOCKADDR_IN);
		if (SOCKET_ERROR == getpeername(mSocket, (SOCKADDR*)&mClientAddr, &addrlen))
		{
			printf_s("[DEBUG] getpeername error: %d\n", GetLastError());
			resultOk = false;
			break;
		}

		HANDLE handle = CreateIoCompletionPort((HANDLE)mSocket, GIocpManager->GetCompletionPort(), (ULONG_PTR)this, 0);
		if (handle != GIocpManager->GetCompletionPort())
		{
			printf_s("[DEBUG] CreateIoCompletionPort error: %d\n", GetLastError());
			resultOk = false;
			break;
		}

	} while (false);


	if (!resultOk)
	{
		DisconnectRequest(DR_ONCONNECT_ERROR);
		return;
	}

	if (ClientSession::mShowConnect)
		printf_s("[DEBUG] Client Connected: IP=%s, PORT=%d\n", inet_ntoa(mClientAddr.sin_addr), ntohs(mClientAddr.sin_port));

	if (false == PreRecv())
	{
		printf_s("[DEBUG] PreRecv error: %d\n", GetLastError());
	}



	GFeedCenter->SubscribeFeed(this);
	
}




void ClientSession::OnDisconnect(DisconnectReason dr)
{
	TRACE_THIS;

	printf_s("[DEBUG] Client Disconnected: Reason=%d IP=%s, PORT=%d \n", dr, inet_ntoa(mClientAddr.sin_addr), ntohs(mClientAddr.sin_port));

	GFeedCenter->UnsubscribeFeed(this);
}

void ClientSession::OnRelease()
{
	TRACE_THIS;

	GClientSessionManager->ReturnClientSession(this);
}

bool ClientSession::SendRequest(short packetType, const protobuf::MessageLite& payload)
{
	if (!IsConnected())
		return false;

	FastSpinlockGuard criticalSection(mSendBufferLock);

	int totalSize = payload.ByteSize() + HEADER_SIZE;
	if (mSendBuffer.GetFreeSpaceSize() < totalSize)
		return false;

	protobuf::io::ArrayOutputStream arrayOutputStream(mSendBuffer.GetBuffer(), totalSize);
	protobuf::io::CodedOutputStream codedOutputStream(&arrayOutputStream);

	PacketHeader header;
	header.mSize = payload.ByteSize();
	header.mType = packetType;

	codedOutputStream.WriteRaw(&header, HEADER_SIZE);
	payload.SerializeToCodedStream(&codedOutputStream);
	
	mSendBuffer.Commit(totalSize);

	OverlappedSendContext* sendContext = new OverlappedSendContext(this);
	DWORD sendbytes = 0;
	DWORD flags = 0;
	sendContext->mWsaBuf.len = (ULONG)mSendBuffer.GetContiguiousBytes();
	sendContext->mWsaBuf.buf = mSendBuffer.GetBufferStart();

	/// start async send
	if (SOCKET_ERROR == WSASend(mSocket, &sendContext->mWsaBuf, 1, &sendbytes, flags, (LPWSAOVERLAPPED)sendContext, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIoContext(sendContext);
			printf_s("Session::FlushSend Error : %d\n", GetLastError());

			DisconnectRequest(DR_SENDFLUSH_ERROR);
			return true;
		}

	}
	
	return true;
}