#include "stdafx.h"
#include "Exception.h"
#include "Log.h"
#include "PacketInterface.h"
#include "ClientSession.h"


#include "MyPacket.pb.h"
#include "FeedCenter.h"


//@{ Handler Helper

typedef void(*HandlerFunc)(ClientSession* session, PacketHeader& pktBase, protobuf::io::CodedInputStream& payloadStream);

static HandlerFunc HandlerTable[MAX_PKT_TYPE];

static void DefaultHandler(ClientSession* session, PacketHeader& pktBase, protobuf::io::CodedInputStream& payloadStream)
{

	printf_s("Default Handler...PKT ID: %d\n", pktBase.mType);
}

struct InitializeHandlers
{
	InitializeHandlers()
	{
		for (int i = 0; i < MAX_PKT_TYPE; ++i)
			HandlerTable[i] = DefaultHandler;
	}
} _init_handlers_;

struct RegisterHandler
{
	RegisterHandler(int pktType, HandlerFunc handler)
	{
		HandlerTable[pktType] = handler;
	}
};

#define REGISTER_HANDLER(PKT_TYPE)	\
	static void Handler_##PKT_TYPE(ClientSession* session, PacketHeader& pktBase, protobuf::io::CodedInputStream& payloadStream); \
	static RegisterHandler _register_##PKT_TYPE(PKT_TYPE, Handler_##PKT_TYPE); \
	static void Handler_##PKT_TYPE(ClientSession* session, PacketHeader& pktBase, protobuf::io::CodedInputStream& payloadStream)


//@}

void ClientSession::OnReceive(size_t len)
{

	/// 패킷 파싱하고 처리
	protobuf::io::ArrayInputStream arrayInputStream(mRecvBuffer.GetBufferStart(), mRecvBuffer.GetContiguiousBytes());
	protobuf::io::CodedInputStream codedInputStream(&arrayInputStream);

	PacketHeader packetheader;

	while (codedInputStream.ReadRaw(&packetheader, HEADER_SIZE))
	{
		const void* payloadPos = nullptr;
		int payloadSize = 0;

		codedInputStream.GetDirectBufferPointer(&payloadPos, &payloadSize);

		if (payloadSize < packetheader.mSize) ///< 패킷 본체 사이즈 체크
			break;

		if (packetheader.mType >= MAX_PKT_TYPE || packetheader.mType <= 0)
		{
			DisconnectRequest(DR_ACTIVE);
			break;;
		}

		/// payload 읽기
		protobuf::io::ArrayInputStream payloadArrayStream(payloadPos, packetheader.mSize);
		protobuf::io::CodedInputStream payloadInputStream(&payloadArrayStream);

		/// packet dispatch...
		HandlerTable[packetheader.mType](this, packetheader, payloadInputStream);

		/// 읽은 만큼 전진 및 버퍼에서 제거
		codedInputStream.Skip(packetheader.mSize); ///< readraw에서 헤더 크기만큼 미리 전진했기때문
		mRecvBuffer.Remove(HEADER_SIZE + packetheader.mSize);

	}
}

/////////////////////////////////////////////////////////////

using namespace MyPacket;

REGISTER_HANDLER(PKT_CS_LOGIN)
{
	//printf_s("Recv Login Request\n");

	LoginRequest loginReq;

	if (false == loginReq.ParseFromCodedStream(&payloadStream))
	{
		session->DisconnectRequest(DR_ACTIVE);  // why disconnect?
		return;
	}

	
	LoginResult loginResult;
	loginResult.set_playerid(loginReq.playerid());
	loginResult.set_playername("RANDOM NAME");
	
	Position* pos(loginResult.mutable_playerpos());
	pos->set_x(1);
	pos->set_y(2);
	pos->set_z(3);

	session->SendRequest(MyPacket::PKT_SC_LOGIN, loginResult);
}

REGISTER_HANDLER(PKT_CS_SERVER_STATUS)
{
	ServerStatus serverStatus;

	if (false == serverStatus.ParseFromCodedStream(&payloadStream))
	{
		session->DisconnectRequest(DR_ACTIVE);  // why disconnect?
		return;
	}

	ServerStatus myServerState;
	myServerState.set_sessioncount(GFeedCenter->GetSessionCount());

	session->SendRequest(MyPacket::PKT_SC_SERVER_STATUS, myServerState);

}

REGISTER_HANDLER(PKT_CS_ORDER)
{
	Order order;
	if (false == order.ParseFromCodedStream(&payloadStream))
	{
		session->DisconnectRequest(DR_ACTIVE);  // why disconnect?
		return;
	}

}
