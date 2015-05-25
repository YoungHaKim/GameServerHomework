#pragma once

#define DEPTH_MAX 10

struct Depth
{
	int mCount;
	int mQty;
	double mPrc;
};

struct Feed
{
	std::string mProductCode;
	Depth mBidDepths[DEPTH_MAX];
	Depth mAskDepths[DEPTH_MAX];
};

