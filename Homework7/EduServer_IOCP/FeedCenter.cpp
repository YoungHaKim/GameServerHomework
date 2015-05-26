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
FeedStruct*							FeedCenter::mRecentFeed[PRODUCT_COUNT_MAX];
FastSpinlock					FeedCenter::mLock;

FeedCenter::FeedCenter()
{
	mSessionVector.reserve(NUMBER_OF_CONNECTIONS);	

	// be sure you don't exceed PRODUCT_MAX_COUNT
	for (int i = 0; i < PRODUCT_COUNT_MAX; ++i)
	{
		mRecentFeed[i] = new FeedStruct();

	}

	mRecentFeed[0]->mProductCode = "KR4101KC0000";
	mRecentFeed[1]->mProductCode = "KR4201KC2500";
	mRecentFeed[2]->mProductCode = "KR4301KC2500";
	mRecentFeed[3]->mProductCode = "KR4201KC2520";
	mRecentFeed[4]->mProductCode = "KR4301KC2520";
	mRecentFeed[5]->mProductCode = "KR4201KC2550";
	mRecentFeed[6]->mProductCode = "KR4301KC2550";
	mRecentFeed[7]->mProductCode = "KR4201KC2570";
	mRecentFeed[8]->mProductCode = "KR4301KC2570";
	mRecentFeed[9]->mProductCode = "KR4201KC2600";
	mRecentFeed[10]->mProductCode = "KR4301KC2600";
}


FeedCenter::~FeedCenter()
{
	mSessionVector.clear();
}

void FeedCenter::Start()
{
	mRunning = true;	


	mThreadHandle = (HANDLE)_beginthreadex(nullptr, 0, FeedGenerationThread, nullptr, 0, nullptr);
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
	context->SetFeedData(nullptr);

	DatabaseJobContext* dbContext = reinterpret_cast<DatabaseJobContext*>(context);

	// let's create a db context, and post it to the iocp queue
	if (FALSE == PostQueuedCompletionStatus(GIocpManager->GetCompletionPort(), dwTransferred, completionKey, (LPOVERLAPPED)dbContext))
	{
		printf_s("Error in posting DB job result to Queue: %d \n", GetLastError());
	}
}

unsigned int WINAPI FeedCenter::FeedGenerationThread(void* lParam)
{
	while (mRunning)
	{	
		// generate feed
		for (int index = 0; index < PRODUCT_COUNT_MAX; ++index)
		{
			FeedStruct* feed = mRecentFeed[index];

			for (int i = 0; i < DEPTH_MAX; i++)
			{
				DepthStruct* curBidDepth = feed->mBidDepths[i];
				curBidDepth->mCount = 2 + i;
				curBidDepth->mPrc = 0.01 + (rand() % 300) * 0.01 - i * 0.01;
				curBidDepth->mQty = 20 + i;

				DepthStruct* curAskDepth = feed->mAskDepths[i];
				curAskDepth->mCount = 1 + i;
				curAskDepth->mPrc = feed->mBidDepths[0]->mPrc + i * 0.01;
				curAskDepth->mQty = 10 + i;
			}
		}

		int count = mSessionVector.size();

		for (int i = count - 1; i >= 0; --i)
		{
			ClientSession* ses = mSessionVector[i];

			if (ses == nullptr) break;

			if (ses->IsConnected())
			{
				int numFeedsToBroadcast = 2; // rand() % PRODUCT_COUNT_MAX;

				for (int j = 0; j < numFeedsToBroadcast; ++j)
				{
					SendFeedOnce(ses);
				}
			}
			else
			{
				//printf_s("Session disconnected, removing from feedcenter\n");
				mSessionVector.erase(mSessionVector.begin() + i);
				continue;
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

void FeedCenter::PopulateFeedObject(OUT MyPacket::Feed& packet)
{
	// Generate random Feed
	// copy into char array

	int index = rand() % PRODUCT_COUNT_MAX;
	FeedStruct* feed = mRecentFeed[index];

	packet.set_productcode(feed->mProductCode);
	
	for (int i = 0; i < DEPTH_MAX; i++)
	{
		DepthStruct* curAskDepth = feed->mAskDepths[i];
				
		MyPacket::Depth* askDepth = packet.add_askdepth();

		askDepth->set_count(curAskDepth->mCount);
		askDepth->set_price(curAskDepth->mPrc);
		askDepth->set_qty(curAskDepth->mQty);

		DepthStruct* curBidDepth = feed->mBidDepths[i];

		MyPacket::Depth* bidDepth = packet.add_biddepth();

		bidDepth->set_count(curBidDepth->mCount);
		bidDepth->set_price(curBidDepth->mPrc);
		bidDepth->set_qty(curBidDepth->mQty);
	}

}


