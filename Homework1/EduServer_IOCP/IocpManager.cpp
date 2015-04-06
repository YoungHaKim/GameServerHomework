#include "stdafx.h"
#include "IocpManager.h"
#include "EduServer_IOCP.h"
#include "ClientSession.h"
#include "SessionManager.h"

#define GQCS_TIMEOUT	200

__declspec(thread) int LIoThreadId = 0;
IocpManager* GIocpManager = nullptr;

IocpManager::IocpManager() : mCompletionPort(NULL), mIoThreadCount(2), mListenSocket(NULL)
{
}


IocpManager::~IocpManager()
{
}

bool IocpManager::Initialize()
{	
	SYSTEM_INFO sysinfo;
	SecureZeroMemory(&sysinfo, sizeof(sysinfo));
	GetSystemInfo(&sysinfo);

	mIoThreadCount = sysinfo.dwNumberOfProcessors * 2; // just a rule of thumb

	printf_s("mIoThreadCount=%d \n", mIoThreadCount);

	/// winsock initializing
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	/// Create I/O Completion Port
	//TODO: mCompletionPort = CreateIoCompletionPort(...)
	mCompletionPort = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,
		NULL,	// no existing
		0,		// no completion key
		0		// 0 allows as many theads as number of cores
		);

	printf_s("Create completion port: %d \n", mCompletionPort);

	if (mCompletionPort == NULL)
	{
		printf_s("Failed to create io completion port! \n");
		return false;
	}
	
	/// create TCP socket
	//TODO: mListenSocket = ...
	mListenSocket = WSASocket(
		AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP,
		NULL,	// A pointer to a WSAPROTOCOL_INFO structure that defines the characteristics of the socket to be created. 
		0,	//No group operation is performed.
		WSA_FLAG_OVERLAPPED
		);

	if (mListenSocket == INVALID_SOCKET)
	{
		printf_s("Failed to create listen socket! \n");
		return false;
	}
	


	SOCKADDR_IN serveraddr;
	SecureZeroMemory(&serveraddr, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(LISTEN_PORT);


	int opt = 1;
	setsockopt(mListenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(int));

	//TODO:  bind
	if (SOCKET_ERROR == 
			bind(
				mListenSocket, 
				(SOCKADDR*)&serveraddr, 
				sizeof(serveraddr)
				)
		)
		return false;

	printf_s("IOCP Manager Init Success!\n");
	return true;
}


bool IocpManager::StartIoThreads()
{
	/// I/O Thread
	for (int i = 0; i < mIoThreadCount; ++i)
	{
		DWORD dwThreadId;

		HANDLE threadHandle = 
			(HANDLE)_beginthreadex(
				NULL, 
				0, 
				IoWorkerThread, 
				(LPVOID)i, 
				0, 
				(unsigned int*)&dwThreadId
				);

		if (threadHandle == INVALID_HANDLE_VALUE)
		{
			printf_s("Hey failed to create thread! \n");
			return false;
		}

		CloseHandle(threadHandle);

		//TODO: HANDLE hThread = (HANDLE)_beginthreadex...);
	}

	return true;
}


bool IocpManager::StartAcceptLoop()
{
	/// listen
	if (SOCKET_ERROR == listen(mListenSocket, SOMAXCONN))
		return false;


	/// accept loop
	while (true)
	{
		SOCKET acceptedSock = accept(mListenSocket, NULL, NULL);
		if (acceptedSock == INVALID_SOCKET)
		{
			printf_s("accept: invalid socket\n");
			continue;
		}

		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(acceptedSock, (SOCKADDR*)&clientaddr, &addrlen);

		/// 소켓 정보 구조체 할당과 초기화
		ClientSession* client = GSessionManager->CreateClientSession(acceptedSock);

		/// 클라 접속 처리
		if (false == client->OnConnect(&clientaddr))
		{
			client->Disconnect(DR_ONCONNECT_ERROR);
			GSessionManager->DeleteClientSession(client);
		}
	}

	return true;
}

void IocpManager::Finalize()
{
	CloseHandle(mCompletionPort);

	/// winsock finalizing
	WSACleanup();

}


unsigned int WINAPI IocpManager::IoWorkerThread(LPVOID lpParam)
{
	LThreadType = THREAD_IO_WORKER;

	LIoThreadId = reinterpret_cast<int>(lpParam);
	HANDLE hCompletionPort = GIocpManager->GetCompletionPort();

	printf_s("Started thread for completion port %d \n", hCompletionPort);

	while (true)
	{
		DWORD dwTransferred = 0;
		OverlappedIOContext* context = nullptr;
		ClientSession* asCompletionKey = nullptr;

		int ret = GetQueuedCompletionStatus(
				hCompletionPort,					//	_In_   HANDLE CompletionPort,
				&dwTransferred,					//	Out_  LPDWORD lpNumberOfBytes,
				(PULONG_PTR)&asCompletionKey,	//	_Out_  PULONG_PTR lpCompletionKey,
				(LPOVERLAPPED*)&context,			//	_Out_  LPOVERLAPPED *lpOverlapped,
				GQCS_TIMEOUT					//	_In_   DWORD dwMilliseconds
			); 
				

		/// check time out first 
		if (ret == 0 && GetLastError()==WAIT_TIMEOUT)
			continue;

		if (ret == 0 || dwTransferred == 0)
		{
			/// connection closing
			asCompletionKey->Disconnect(DR_RECV_ZERO);
			GSessionManager->DeleteClientSession(asCompletionKey);
			continue;
		}

		if (context == nullptr)
		{
			asCompletionKey->Disconnect(DR_RECV_ZERO);
			GSessionManager->DeleteClientSession(asCompletionKey);

		}

		bool completionOk = true;
		switch (context->mIoType)
		{
		case IO_SEND:
			completionOk = SendCompletion(asCompletionKey, context, dwTransferred);
			break;

		case IO_RECV:
			completionOk = ReceiveCompletion(asCompletionKey, context, dwTransferred);
			break;

		default:
			printf_s("Unknown I/O Type: %d\n", context->mIoType);
			break;
		}

		if ( !completionOk )
		{
			/// connection closing
			asCompletionKey->Disconnect(DR_COMPLETION_ERROR);
			GSessionManager->DeleteClientSession(asCompletionKey);
		}

	}

	return 0;
}

bool IocpManager::ReceiveCompletion(const ClientSession* client, OverlappedIOContext* context, DWORD dwTransferred)
{

	/// echo back 처리 client->PostSend()사용.
	if (client == nullptr)
	{
		printf_s("ReceiveCompletion client returned nullptr! \n");
		return false;
	}

	
	bool sendResult = 
		client->PostSend(context->mBuffer, dwTransferred);
	
	delete context;

	if (!sendResult)
	{
		printf_s("Send to client failed! \n");
		return false;
	}

	return client->PostRecv();
}

bool IocpManager::SendCompletion(const ClientSession* client, OverlappedIOContext* context, DWORD dwTransferred)
{
	if (client == nullptr)
	{
		printf_s("SendCompletion client returned nullptr! \n");
		return false;
	}

	/// 전송 다 되었는지 확인하는 것 처리..
	if (context->mWsaBuf.len != dwTransferred) 
	{
		delete context;
		return false;
	}
	
	delete context;
	return true;
}
