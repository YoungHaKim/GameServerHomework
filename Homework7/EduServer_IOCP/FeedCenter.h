#pragma once

#include "Feed.h"
#include "MyPacket.pb.h"
#include "FastSpinlock.h"
/*
continously generates a random feed for a given number of products
broadcasts the feed to each session that is connected
*/

#define PRODUCT_COUNT_MAX 11

class ClientSession;


class FeedCenter
{
public:
	FeedCenter();
	~FeedCenter();

	void Start();
	void Finalize();

	static void PopulateFeedObject(OUT MyPacket::Feed& data);

	void SubscribeFeed(ClientSession* session);
	void UnsubscribeFeed(ClientSession* session);

private:
	static unsigned int WINAPI FeedGenerationThread(void*);

	static void SendFeedOnce(ClientSession* ses);	

	static std::vector<ClientSession*> mSessionVector;

	using c_iter = std::vector<ClientSession*>::const_iterator;
	c_iter begin() const { return mSessionVector.begin(); }
	c_iter end() const { return mSessionVector.end(); }


	HANDLE mThreadHandle;

	static bool mRunning;
	static FastSpinlock mLock;

	static FeedStruct* mRecentFeed[PRODUCT_COUNT_MAX];
};

extern FeedCenter* GFeedCenter;
