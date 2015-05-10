#pragma once

class Session;

enum IOType
{
	IO_NONE,
	IO_SEND,
	IO_RECV,
	IO_RECV_ZERO,
	IO_ACCEPT,
	IO_CONNECT,
	IO_DISCONNECT
};

struct OverlappedContext
{
	OverlappedContext(IOType ioType, Session* owner) : mIOType(ioType), mSession(owner)
	{
		SecureZeroMemory(&mOverLapped, sizeof(OVERLAPPED));
		SecureZeroMemory(&mWsaBuf, sizeof(WSABUF));
	}

	WSAOVERLAPPED mOverLapped;
	IOType mIOType;
	WSABUF mWsaBuf;
	Session* mSession;
};

