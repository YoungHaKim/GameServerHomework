#include "stdafx.h"
#include "Player.h"
#include "PlayerManager.h"

PlayerManager* GPlayerManager = nullptr;

PlayerManager::PlayerManager() : mLock(LO_ECONOMLY_CLASS), mCurrentIssueId(0)
{

}

int PlayerManager::RegisterPlayer(std::shared_ptr<Player> player)
{
	FastSpinlockGuard exclusive(mLock);

	mPlayerMap[++mCurrentIssueId] = player;

	return mCurrentIssueId;
}

void PlayerManager::UnregisterPlayer(int playerId)
{
	FastSpinlockGuard exclusive(mLock);

	mPlayerMap.erase(playerId);
}


int PlayerManager::GetCurrentPlayers(PlayerList& outList)
{
	//DONE
	//TODO: mLock�� read���� �����ؼ� outList�� ���� �÷��̾���� ������ ��� total���� ���� �÷��̾��� �� ���� ��ȯ.
	FastSpinlockGuard nonExclusive(mLock, false);
	
	int total = 0;

	for (auto it = mPlayerMap.begin(); it != mPlayerMap.end(); it++) ///# ++it�� �ٲ�� �� ������ �ӽð�ü ��������Ǳ� ����
	{
		++total;
		outList.emplace_back(it->second);
	}

	return total;
}