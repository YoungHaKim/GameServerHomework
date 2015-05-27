#pragma once

#include "Feed.h"
#include "MyPacket.pb.h"
#include "FastSpinlock.h"
/*
continously generates a random feed for a given number of products
broadcasts the feed to each session that is connected
*/

#define DEPTH_MAX 30
#define PRODUCT_COUNT_MAX 11
#define FEED_THREAD_COUNT 1

class ClientSession;


class FeedCenter
{
public:
	FeedCenter();
	~FeedCenter();

	void Start();
	void Finalize();

	//static void PopulateFeedObject(OUT MyPacket::Feed& data);

	void SubscribeFeed(ClientSession* session);
	void UnsubscribeFeed(ClientSession* session);

	int GetSessionCount() const { return mSessionCount; }

private:
	static unsigned int WINAPI FeedGenerationThread(void*);

	static void SendFeedOnce(ClientSession* ses);	

	static std::vector<ClientSession*> mSessionVector;

	using c_iter = std::vector<ClientSession*>::const_iterator;
	c_iter begin() const { return mSessionVector.begin(); }
	c_iter end() const { return mSessionVector.end(); }


	HANDLE mThreadHandle[FEED_THREAD_COUNT];

	int mSessionCount = 0;

	///# 아래 녀석들 static으로 한 이유가 뭘까나? C언어 인터페이스로 쓰는 경우 빼고는 없어보이는데...
	static bool mRunning;
	static FastSpinlock mLock;

	static MyPacket::Feed* mRecentFeed[PRODUCT_COUNT_MAX];
};

extern FeedCenter* GFeedCenter;
