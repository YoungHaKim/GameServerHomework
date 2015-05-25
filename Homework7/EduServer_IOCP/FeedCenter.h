#pragma once

#include "Feed.h"
#include "MyPacket.pb.h"
/*
continously generates a random feed for a given number of products
broadcasts the feed to each session that is connected
*/

#define PRODUCT_COUNT_MAX 3

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

	static std::vector<ClientSession*> mSessionVector;
	HANDLE mThreadHandle;

	static bool mRunning;

	static Feed* mRecentFeed[PRODUCT_COUNT_MAX];
};

extern FeedCenter* GFeedCenter;
