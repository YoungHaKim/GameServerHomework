#pragma once

#include "Session.h"
#include "Player.h"
#include "PacketInterface.h"

class ClientSessionManager;


class ClientSession : public Session, public ObjectPool<ClientSession>
{
public:
	ClientSession();
	virtual ~ClientSession();

	void SessionReset();

	bool PostAccept();
	void AcceptCompletion();

	bool SendRequest(short packetType, const protobuf::MessageLite& payload);
	
	virtual void OnDisconnect(DisconnectReason dr);
	virtual void OnRelease();
	virtual void OnReceive(size_t len);

public:
	Player			mPlayer;
	static bool mShowConnect;

private:
	
	SOCKADDR_IN		mClientAddr ;

	
	friend class ClientSessionManager;
} ;



