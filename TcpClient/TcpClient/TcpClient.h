#pragma once
#define PRINT_DEADLOCK
class CTcpClient;

DWORD WINAPI ListenThread(void* pParam);

void CloseThread(HANDLE ThH,bool bTerminate);
void LockCS(CRITICAL_SECTION* mut);
void UnlockCS(CRITICAL_SECTION* mut);

struct ListenThInput
{
    ListenThInput()
    {
        ClientSocket=0;
        pParent=0;
    };

    SOCKET ClientSocket;
    CTcpClient* pParent;
};

struct ThreadInfo
{
    ThreadInfo()
    {
        ThHandle=0;
        bCreated=false;
    };

    HANDLE ThHandle;
	DWORD ThID;
    bool bCreated;
};

class CTcpClient
{
public:
	CTcpClient(void);
	~CTcpClient(void);

	bool Connect(char* IP, USHORT Port);
	void Disconnect(bool bWaitForThreadClosed=true);
	void Send(char* Msg);
	void OnMessage(std::string Msg);

private:
	ThreadInfo ListenThInfo;
    SOCKET ConnectSock;
    CRITICAL_SECTION CS;
	sockaddr_in ConnectAddr;
};
