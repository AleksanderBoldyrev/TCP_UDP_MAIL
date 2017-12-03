#pragma once
#pragma once

#include "stdinc.h"

#include "ServerWorker.h"

typedef unsigned short  USHORT;
//typedef int             SOCKET;
typedef int             ClientID;

#define PRINT_DEADLOCK

#define TIMEOUT_MS      100     /* Seconds between retransmits */
#define UDP_DG_LEN      512     // UNICODE symbol count in one packet.
#define TECH_DG_SIZE    10      // Char count, in which the length of the packet and it's number are secured.

using namespace std;

class CUdpServer;

//void CloseThread(pthread_t ThH, bool bTerminate);
//void LockMutex(pthread_mutex_t* mut);
//void UnlockMutex(pthread_mutex_t* mut);

/*struct AcceptThInput
{
	AcceptThInput()
	{
		ThPort = 0;
		pParent = 0;
		pAcceptSock = 0;
	};

	USHORT ThPort;
	SOCKET* pAcceptSock;
	CUdpServer* pParent;
};*/

/*struct ListenThInput
{
	ListenThInput()
	{
		ClientSocket = 0;
		pParent = 0;
		CliID = 0;
		State = 1;
		CurQst = 1;
		result = 0;
	};

	SOCKET ClientSocket;
	ClientID CliID;
	CUdpServer* pParent;
	int State;
	int CurQst;
	int result;
};*/

/*struct ThreadInfo
{
	ThreadInfo()
	{
		ThHandle = 0;
		ThPort = 0;
		bCreated = false;
	};

	HANDLE ThHandle;
	USHORT ThPort;
	bool bCreated;
};

struct ClientInfo
{
	ClientInfo()
	{
		ID = 0;
		ClientSocket = 0;
	};

	ClientID ID;
	SOCKET ClientSocket;
	ThreadInfo ClientThreadInfo;
};*/

//bool ListenProc(ListenThInput& pData, int& State, int& CurQst, int& result);
//bool ListenRecv(ClientID& From, std::string& MsgStr, ListenThInput& pData);
//bool register_user(char* user_name, CUdpServer* pData, ClientID id);

//typedef std::map<ClientID, ClientInfo> ClientsMapArray;
//typedef std::map<ClientID, ClientInfo>::iterator ClientsMapArrayIt;
//typedef std::pair<ClientID, ClientInfo> ClientsPair;

class CUdpServer {
public:
	CUdpServer();
	CUdpServer(const CUdpServer& orig);
	virtual ~CUdpServer();
	void StartAccept(USHORT Port);
	//void StartListenTh(SOCKET Sock);
private:
	//ThreadInfo AcceptThInfo;
	//ClientID LastClientID;
	ThreadData* clients;
	//SOCKET AcceptSock;
	HANDLE Mut;
	HANDLE serverThread;
	static bool LockMutex(HANDLE& m);
	static void UnlockMutex(HANDLE& m);
	static bool LockMutex(const wstring& name, HANDLE& m);
	unsigned int clientsSize = 0;
	static DWORD WINAPI AcceptThread(LPVOID pParam);
	static DWORD WINAPI ListenThread(LPVOID pParam);
	static void processInput(const SOCKET& s, ThreadData* &clients, unsigned int& cSize);
	static void processOutput(const SOCKET& s, ThreadData* clients, const unsigned int& cSize);
	static unsigned int checkClient(const sockaddr_in& saddr, ThreadData* &clients, unsigned int& cSize);
};