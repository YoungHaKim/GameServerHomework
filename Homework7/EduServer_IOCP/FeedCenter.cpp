#include "stdafx.h"
#include "FeedCenter.h"
#include "ClientSession.h"
#include "MyPacket.pb.h"
#include "ThreadLocal.h"
#include "IocpManager.h"
#include "PlayerDBContext.h"

#define  NUMBER_OF_CONNECTIONS 100

FeedCenter* GFeedCenter = nullptr;

bool							FeedCenter::mRunning = false;
std::vector<ClientSession*>		FeedCenter::mSessionVector;
MyPacket::Feed*					FeedCenter::mRecentFeed[PRODUCT_COUNT_MAX];
FastSpinlock					FeedCenter::mLock;

FeedCenter::FeedCenter()
{
	mSessionVector.reserve(NUMBER_OF_CONNECTIONS);	
	
	for (size_t i = 0; i < PRODUCT_COUNT_MAX; i++)
	{
		mRecentFeed[i] = new MyPacket::Feed();

		for (int j = 0; j < DEPTH_MAX; j++)
		{
			mRecentFeed[i]->add_askdepth();
			mRecentFeed[i]->add_biddepth();
		}
	}

	mRecentFeed[0]->set_productcode("KR4101KC0000");
	mRecentFeed[1]->set_productcode("KR4201KC2500");
	mRecentFeed[2]->set_productcode("KR4201KC2520");
	mRecentFeed[3]->set_productcode("KR4201KC2550");
	mRecentFeed[4]->set_productcode("KR4201KC2570");
	mRecentFeed[5]->set_productcode("KR4201KC2600");	
	mRecentFeed[6]->set_productcode("KR4301KC2520");
	mRecentFeed[7]->set_productcode("KR4301KC2550");
	mRecentFeed[8]->set_productcode("KR4301KC2570");
	mRecentFeed[9]->set_productcode("KR4301KC2600");
	mRecentFeed[10]->set_productcode("KR4301KC2500");
	
}


FeedCenter::~FeedCenter()
{
	mSessionVector.clear();
}

void FeedCenter::Start()
{
	mRunning = true;	

	for (int i = 0; i < FEED_THREAD_COUNT; ++i)
	{
		mThreadHandle[i] = (HANDLE)_beginthreadex(nullptr, 0, FeedGenerationThread, (LPVOID)(i), 0, nullptr);
	}
}

void FeedCenter::Finalize()
{
	mRunning = false;
	if (mThreadHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(mThreadHandle);
	}
}

void FeedCenter::SendFeedOnce(ClientSession* ses)
{
	DWORD completionKey = CK_DB_RESULT;
	DWORD dwTransferred = sizeof(SendFeedDataContext);

	SendFeedDataContext* context = new SendFeedDataContext(ses);
	context->SetFeedData(mRecentFeed[rand() % PRODUCT_COUNT_MAX]);

	DatabaseJobContext* dbContext = reinterpret_cast<DatabaseJobContext*>(context);

	// let's create a db context, and post it to the iocp queue
	if (FALSE == PostQueuedCompletionStatus(GIocpManager->GetCompletionPort(), dwTransferred, completionKey, (LPOVERLAPPED)dbContext))
	{
		printf_s("Error in posting DB job result to Queue: %d \n", GetLastError());
	}
}

unsigned int WINAPI FeedCenter::FeedGenerationThread(void* lParam)
{
	LWorkerThreadId = reinterpret_cast<int>(lParam);

	while (mRunning)
	{	
		// generate feed
		if (LWorkerThreadId == 0)
		{
			for (int index = 0; index < PRODUCT_COUNT_MAX; ++index)
			{
				MyPacket::Feed* feed = mRecentFeed[index];

				for (int i = 0; i < DEPTH_MAX; i++)
				{
					MyPacket::Depth* curBidDepth = feed->mutable_biddepth(i);
					curBidDepth->set_count(2 + i);
					curBidDepth->set_price(0.01 + (rand() % 300) * 0.01 - i * 0.01);
					curBidDepth->set_qty(20 + i);

					MyPacket::Depth* curAskDepth = feed->mutable_askdepth(i);
					curAskDepth->set_count(1 + i);
					curAskDepth->set_price(feed->mutable_biddepth(0)->price() + i * 0.01);
					curAskDepth->set_qty(10 + i);
				}
			}
		}

		int count = mSessionVector.size();

		for (int i = count - 1; i >= 0; --i)
		{
			ClientSession* ses = mSessionVector[i];

			if (ses == nullptr) break;

			if (ses->IsConnected())
			{
				SendFeedOnce(ses);				
			}
			else
			{
				if (i >= 0)
				{
					//printf_s("Session disconnected, removing from feedcenter\n");
					mSessionVector.erase(mSessionVector.begin() + i);
					continue;
				}
			}
		}
		
		//for (auto it = mSessionVector.begin();
		//	it != mSessionVector.end();
		//	++it)
		//{
		//	ClientSession* ses = *it;
		//	
		//	if (ses == nullptr) break;

		//	if (ses->IsConnected())
		//	{
		//		DWORD completionKey = CK_DB_RESULT;
		//		DWORD dwTransferred = sizeof(SendFeedDataContext);

		//		SendFeedDataContext* context = new SendFeedDataContext(ses);
		//		context->SetFeedData(nullptr);

		//		DatabaseJobContext* dbContext = reinterpret_cast<DatabaseJobContext*>(context);
		//		
		//		// let's create a db context, and post it to the iocp queue
		//		if (FALSE == PostQueuedCompletionStatus(GIocpManager->GetCompletionPort(), dwTransferred, completionKey, (LPOVERLAPPED)dbContext))
		//		{
		//			printf_s("Error in posting DB job result to Queue: %d \n", GetLastError());
		//			continue;;
		//		}
		//	}
		//	else
		//	{
		//		//it = mSessionVector.erase(it);
		//		continue;
		//	}
		//}


		Sleep(30);
	}
	return 0;
}

void FeedCenter::SubscribeFeed(ClientSession* session)
{
	mSessionVector.emplace_back(session);
}

void FeedCenter::UnsubscribeFeed(ClientSession* session)
{
	/*for (auto it = mSessionVector.begin();
		it != mSessionVector.end();
		++it)
		{
		if ((*it)->GetSocket() == session->GetSocket())
		{
		it = mSessionVector.erase(it);
		break;
		}
		}*/
}



