#pragma once

#include <vector>
// this class manages the sessions and the iocp init & threads

#include "Session.h"

class IOManager
{
public:
	IOManager();
	~IOManager();

	BOOL InitIOCP();
	void StartIOCPThreads();
	void AddSession(Session* session) { mVecSessions.push_back(session); }	
	void CleanUp();
	void PrintStats();
	void BombardData();


	LPFN_CONNECTEX FnConnectEx;
	LPFN_DISCONNECTEX FnDisconnectEx;

	static unsigned int __stdcall IOThread(void *pArgs);

	HANDLE GetCompletionPort() { return mCompletionPort; }

private:
	std::vector<Session*> mVecSessions;
	HANDLE mCompletionPort;
	
};

extern IOManager* g_IoManager;
