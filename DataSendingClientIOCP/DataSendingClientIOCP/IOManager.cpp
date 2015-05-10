#include "stdafx.h"
#include "IOManager.h"
#include "OverlappedContext.h"

IOManager* g_IoManager = nullptr;

IOManager::IOManager()
{
	mVecSessions.reserve(500);
}

IOManager::~IOManager()
{
	for (auto it : mVecSessions)
	{
		delete it;
	}

	mVecSessions.clear();
}

unsigned int WINAPI IOManager::IOThread(void *pArgs)
{

	InterlockedIncrement(&g_ThreadCnt);

	while (true)
	{
		DWORD cbTransferred;
		SOCKET socket;
		OverlappedContext* pOV = nullptr;

		BOOL result = GetQueuedCompletionStatus(
			g_IoManager->GetCompletionPort(), 
			&cbTransferred, 
			(LPDWORD)&socket,
			(LPOVERLAPPED*)&pOV, 
			150
			);

		if (result == FALSE)
		{
			if (WSAGetLastError() != WSA_WAIT_TIMEOUT)
			{
				printf_s(
					"GQCS failed, code: %d \n", WSAGetLastError());
				break;
			}
			else
				continue;
		}

		if (pOV == nullptr)
		{
			printf_s(
				"GQCS failed, overlapped is null, code: %d \n", WSAGetLastError());
			break;
		}

		Session* currentSession = pOV ? pOV->mSession : nullptr;

		if (currentSession == nullptr)
		{
			printf_s("Major error! current session is nullptr!! \n");
			break;
		}

		if (cbTransferred == 0)
		{
			// check if disconnected, which means we read 0 or failed to send any bytes

			if (pOV->mIOType == IO_SEND
				|| pOV->mIOType == IO_RECV)
			{
				currentSession->Disconnect();
				//Disconnect();
			}

			if (socket == -1)
			{
				break;  // closing program
			}
		}

		switch (pOV->mIOType)
		{
		case IO_CONNECT:
			currentSession->PostRecv();
			printf_s("Connection established \n");
			break;

		case IO_RECV:
			currentSession->PostRecv();
			currentSession->AddBytesToRecvCnt(cbTransferred);

			//printf_s("Recv:[%.*s] \n", cbTransferred, pOV->mWsaBuf.buf);
			currentSession->SendData(pOV->mWsaBuf.buf, cbTransferred);
			break;

		case IO_SEND:
			currentSession->AddBytesToSendCnt(cbTransferred);
			//printf_s("Sent complete, bytes sent = %d \n", cbTransferred);
			break;

		case IO_DISCONNECT:
			printf_s("Disconnected \n");
			break;

		default:
			break;
		}

		delete pOV;
	}

	InterlockedDecrement(&g_ThreadCnt);

	return 0;
}



BOOL IOManager::InitIOCP()
{
	// start socket
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	if (WSAStartup(version, &wsaData) == SOCKET_ERROR)
	{
		printf_s("Error in starting winsock %d \n", WSAGetLastError());
		return FALSE;
	}

	mCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (mCompletionPort == NULL)
	{
		printf_s("Error in creating io completion port \n");
		return FALSE;
	}

	SOCKET tmpSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	GUID guidConnectEx = WSAID_CONNECTEX;
	DWORD bytes = 0;
	if (SOCKET_ERROR == WSAIoctl(tmpSock, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidConnectEx, sizeof(GUID), &FnConnectEx, sizeof(LPFN_CONNECTEX), &bytes, NULL, NULL))
	{
		printf_s("Error in getting connectEx func ptr \n");
		return FALSE;
	}

	GUID guidDisConnectEx = WSAID_DISCONNECTEX;
	if (SOCKET_ERROR == WSAIoctl(tmpSock, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidDisConnectEx, sizeof(GUID), &FnDisconnectEx, sizeof(LPFN_DISCONNECTEX), &bytes, NULL, NULL))
	{
		printf_s("Error in getting disconnectEx func ptr \n");
		return FALSE;
	}

	closesocket(tmpSock);

	return TRUE;
}

void IOManager::StartIOCPThreads()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	for (DWORD i = 0; i < si.dwNumberOfProcessors * 2; ++i)
	{
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, IOThread, NULL, 0, NULL);
		CloseHandle(hThread);
	}
}

void IOManager::CleanUp()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	for (DWORD i = 0; i < si.dwNumberOfProcessors * 2; ++i)
	{
		PostQueuedCompletionStatus(GetCompletionPort(), 0, -1, NULL);
	}

	while (g_ThreadCnt > 0)
		Sleep(150);

	for (auto it : mVecSessions)
	{
		closesocket(it->GetSocket());
	}	

	WSACleanup();
}

void IOManager::PrintStats()
{
	for (auto it : mVecSessions)
	{
		std::cout << "Sent: " << it->GetSentCnt() << " Recv: " << it->GetRecvCnt() << std::endl;
	}
}

void IOManager::BombardData()
{
	for (auto it : mVecSessions)
	{
		char buf[CLIENT_RECV_BUF_SIZE];

		for (int i = 0; i < CLIENT_RECV_BUF_SIZE; ++i)
		{
			buf[i] = i % 128;
		}

		it->SendData(buf, CLIENT_RECV_BUF_SIZE);
	}
}



