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


		for (auto ses : mSessionVector)
		{
			if (ses == nullptr) break;

			if (ses->IsConnected())
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
					continue;;
				}
			}
		}


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
	for (auto it = mSessionVector.begin();
		it != mSessionVector.end();
		++it)
	{
		if ((*it)->GetSocket() == session->GetSocket())
		{
			it = mSessionVector.erase(it);
			break;
		}
	}
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


