#pragma once


#define CLIENT_RECV_BUF_SIZE 4096

class Session
{
public:
	Session(int port, char* ipStr) : mPort(port), mIpStr(ipStr) {}
	~Session();

	void Initialize();
	void ConnectAndStart();
	void SendRandomData();
	void AddBytesToRecvCnt(unsigned long bytes);
	void AddBytesToSendCnt(unsigned long bytes);


	void PostRecv();
	void SendData(char* buf, int len);
	void Disconnect();

	unsigned long GetRecvCnt() { return mRcvdBytes; }
	unsigned long GetSentCnt() { return mSentBytes; }

	SOCKET GetSocket() { return mSocket; }

private:
	int mPort;
	char* mIpStr;

	unsigned long mSentBytes;
	unsigned long mRcvdBytes;

	SOCKET mSocket;
	
};

