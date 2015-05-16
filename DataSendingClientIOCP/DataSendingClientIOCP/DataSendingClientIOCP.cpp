// DataSendingClientIOCP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Session.h"
#include "IOManager.h"

// the idea is to create X number of client sessions
// connect to a specified server
// send and recv data for Y seconds
// record how many bytes were sent and recvd
// show the stats
// end the program

#define IP "127.0.0.1"
#define PORT 9990
#define SECONDS_TO_SEND 2
#define NUMBER_OF_SESSIONS 10
#define FREQUENCY_OF_DATA_SENDS_PER_SECOND 5  // set this to a number greater than or equal to 1



int _tmain(int argc, _TCHAR* argv[])
{
	printf_s("Welcome to the data sending test client\n");
	printf_s("This program will connect to IP:[%s:%d] \n", IP, PORT);
	printf_s("Opening %d sessions \n", NUMBER_OF_SESSIONS);
	printf_s("And will send data for %d seconds \n", SECONDS_TO_SEND);
	printf_s("Press Enter to Let it Rip! \n");
	getchar();
	
	g_IoManager = new IOManager();
	g_IoManager->InitIOCP();
	g_IoManager->StartIOCPThreads();

	for (int i = 0; i < NUMBER_OF_SESSIONS; ++i)
	{
		// Create sessions
		// connect to target
		Session* mySession = new Session(PORT, IP);

		g_IoManager->AddSession(mySession);

		mySession->Initialize();
		mySession->ConnectAndStart();
	}

	ULONGLONG tickValueAtStart = GetTickCount64();   //Retrieves the number of milliseconds that have elapsed since the system was started.
	ULONGLONG tickValueAtFinish = tickValueAtStart + SECONDS_TO_SEND * 1000;

	// for Y seconds
	// keep sending some garbage data
	// keep track of how many bytes sent/recv per session
	while (GetTickCount64() < tickValueAtFinish)
	{
		g_IoManager->BombardData();
		Sleep(1000 / FREQUENCY_OF_DATA_SENDS_PER_SECOND);
	}
	
	printf_s("Hit Enter to Close connections \n");
	getchar();

	// show stats after 60 seconds
	
	g_IoManager->PrintStats();
	g_IoManager->CleanUp();
	

	printf_s("Hit Enter to Exit \n");
	getchar();

	return 0;
}

