#include "stdafx.h"
#include "SQLStatement.h"
#include "Log.h"
#include "PlayerDBContext.h"
#include "DBHelper.h"
#include "ClientSession.h"
#include "FeedCenter.h"


//done: CreatePlayerDataContext 구현
bool CreatePlayerDataContext::OnSQLExecute()
{
	DBHelper dbHelper;

	dbHelper.BindParamText(mPlayerName);

	dbHelper.BindResultColumnInt(&mPlayerID);

	if (dbHelper.Execute(SQL_CreatePlayer))
	{
		if (dbHelper.FetchRow())
		{
			return true;
		}
	}

	return false;
}

void CreatePlayerDataContext::OnSuccess()
{
	mSessionObject->mPlayer.ResponseCreated(mPlayerID, mPlayerName);
}

void CreatePlayerDataContext::OnFail()
{
	char errorStr[256];
	sprintf_s(errorStr, "Failed to create player name : %s", mPlayerName);
	EVENT_LOG(errorStr, mPlayerID);
}


//done: DeletePlayerDataContext 구현
bool DeletePlayerDataContext::OnSQLExecute()
{
	DBHelper dbHelper;

	dbHelper.BindParamInt(&mPlayerID);

	dbHelper.BindResultColumnInt(&mDeletedRows);

	if (dbHelper.Execute(SQL_DeletePlayer))
	{
		if (dbHelper.FetchRow())
		{
			return true;
		}
	}

	return false;
}

void DeletePlayerDataContext::OnSuccess()
{
	mSessionObject->mPlayer.ResponseDeleted(mPlayerID, mDeletedRows);
}

void DeletePlayerDataContext::OnFail()
{	
	EVENT_LOG("Failed to delete player id", mPlayerID);
}


bool LoadPlayerDataContext::OnSQLExecute()
{
	DBHelper dbHelper;

	dbHelper.BindParamInt(&mPlayerId);

	dbHelper.BindResultColumnText(mPlayerName, MAX_NAME_LEN);
	dbHelper.BindResultColumnFloat(&mPosX);
	dbHelper.BindResultColumnFloat(&mPosY);
	dbHelper.BindResultColumnFloat(&mPosZ);
	dbHelper.BindResultColumnBool(&mIsValid);
	dbHelper.BindResultColumnText(mComment, MAX_COMMENT_LEN);

	if (dbHelper.Execute(SQL_LoadPlayer))
	{
		if (dbHelper.FetchRow())
		{
			return true;
		}
	}

	return false;
}

void LoadPlayerDataContext::OnSuccess()
{
	//done: 플레이어 로드 성공시 처리하기
	mSessionObject->mPlayer.ResponseLoad(mPlayerId, mPosX, mPosY, mPosZ, mIsValid, mPlayerName, mComment);
}

void LoadPlayerDataContext::OnFail()
{
	EVENT_LOG("LoadPlayerDataContext fail", mPlayerId);
}


bool UpdatePlayerPositionContext::OnSQLExecute()
{
	DBHelper dbHelper;
	int result = 0;

	dbHelper.BindParamInt(&mPlayerId);
	dbHelper.BindParamFloat(&mPosX);
	dbHelper.BindParamFloat(&mPosY);
	dbHelper.BindParamFloat(&mPosZ);

	dbHelper.BindResultColumnInt(&result);

	if (dbHelper.Execute(SQL_UpdatePlayerPosition))
	{
		if (dbHelper.FetchRow())
		{
			return result != 0;
		}
	}

	return false;
}

void UpdatePlayerPositionContext::OnSuccess()
{
	mSessionObject->mPlayer.ResponseUpdatePosition(mPosX, mPosY, mPosZ);
}




bool UpdatePlayerCommentContext::OnSQLExecute()
{
	DBHelper dbHelper;
	int result = 0;
	dbHelper.BindParamInt(&mPlayerId);
	dbHelper.BindParamText(mComment);

	dbHelper.BindResultColumnInt(&result);

	if (dbHelper.Execute(SQL_UpdatePlayerComment))
	{
		if (dbHelper.FetchRow())
		{
			return result != 0;
		}
	}

	return false;
}

void UpdatePlayerCommentContext::SetNewComment(const wchar_t* comment)
{
	wcscpy_s(mComment, comment);
}

void UpdatePlayerCommentContext::OnSuccess()
{
	mSessionObject->mPlayer.ResponseUpdateComment(mComment);
}



bool UpdatePlayerValidContext::OnSQLExecute()
{
	DBHelper dbHelper;
	int result = 0;

	dbHelper.BindParamInt(&mPlayerId);
	dbHelper.BindParamBool(&mIsValid);

	dbHelper.BindResultColumnInt(&result);

	if (dbHelper.Execute(SQL_UpdatePlayerValid))
	{
		if (dbHelper.FetchRow())
		{
			return result !=0 ;
		}
	}

	return false;
}

void UpdatePlayerValidContext::OnSuccess()
{
	mSessionObject->mPlayer.ResponseUpdateValidation(mIsValid);
}




bool SendFeedDataContext::OnSQLExecute()
{
	return m_FeedCreationSuccess;
}

void SendFeedDataContext::OnSuccess()
{
	// send data to socket!
	mSessionObject->SendRequest(MyPacket::PKT_SC_FEED, *mFeedObj);
}

void SendFeedDataContext::OnFail()
{
	printf_s("Failed to run feed generation\n");
}

void SendFeedDataContext::SetFeedData(MyPacket::Feed* feedObj)
{
	if (GFeedCenter == nullptr)
		return;

	mFeedObj = feedObj;

	m_FeedCreationSuccess = true;
}
