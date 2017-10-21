#pragma once


#include "stdinc.h"
#include <iostream>
#include <fstream>
#include <string>
#include "time.h"

#define USERS_FOLDER "./users/"
#define MESSAGE_FILE "/messages"
#define PASSW_FILE   "/password"

#define MES_ID          "<id>"
#define MES_ADDR        "<from>"
#define MES_DATE_TIME   "<date/time>"
#define MES_LEN         "<len>"
#define MES_STATE       "<state>"             // message read/unread/deleted

typedef unsigned short  USHORT;
typedef int             SOCKET;
typedef int             ClientID;

#define PRINT_DEADLOCK

using namespace std;
        
class CTcpServer;
void* AcceptThread(void* pParam);
void* ListenThread(void* pParam);

void CloseThread(pthread_t ThH,bool bTerminate);
void LockMutex(pthread_mutex_t* mut);
void UnlockMutex(pthread_mutex_t* mut);

struct AcceptThInput
{
    AcceptThInput()
    {
        ThPort=0;
        pParent=0;
        pAcceptSock=0;
    };

    USHORT ThPort;
    SOCKET* pAcceptSock;
    CTcpServer* pParent;
};

struct ListenThInput
{
    ListenThInput()
    {
        ClientSocket=0;
        pParent=0;
        CliID=0;
        State=1;
        CurQst=1;
        result=0;
    };

    SOCKET ClientSocket;
    ClientID CliID;
    CTcpServer* pParent;
    int State;
    int CurQst;
    int result;
};

struct ThreadInfo
{
    ThreadInfo()
    {
        ThHandle=0;
        ThPort=0;
        bCreated=false;
    };

    pthread_t ThHandle;
    USHORT ThPort;
    bool bCreated;
};

struct ClientInfo
{
    ClientInfo()
    {
        ID=0;
        ClientSocket=0;
    };

    ClientID ID;
    SOCKET ClientSocket;
    ThreadInfo ClientThreadInfo;
};

bool ListenProc(ListenThInput& pData,int& State,int& CurQst,int& result);
bool ListenRecv(ClientID& From, std::string& MsgStr,ListenThInput& pData);
bool register_user(char* user_name,CTcpServer* pData,ClientID id);

typedef std::map<ClientID,ClientInfo> ClientsMapArray;
typedef std::map<ClientID,ClientInfo>::iterator ClientsMapArrayIt;
typedef std::pair<ClientID,ClientInfo> ClientsPair;

class CTcpServer {
public:
    CTcpServer();
    CTcpServer(const CTcpServer& orig);
    virtual ~CTcpServer();

    void Destroy();

    void StartAccept(USHORT Port);
    void StopAccept(bool bWaitForThreadClosed=true);

    void StartListenTh(SOCKET Sock);
    void StopListenTh(ClientID ID, bool bWaitForThreadClosed=true);

    void DisconnectClient(ClientID ID);
    void DisconnectAll();

    void OnMessage(ClientID From, std::string MsgStr, int State, int CurQst, int result);
    char* removeLastSymbol(char* str);
    string RegisterNewUser(const string &data, bool &res);
    string LoginNewUser(const string &data, bool &res, string& username);
    string DeleteUser(const string& username);
    string GetPasswFilePth(const string& username);
    string GetMessageFilePth(const string& username);
    unsigned long AddMessage(const string& message, const string& username);
    
    void SendTo(const char* MsgStr, const ClientID CliID);
    std::vector<char*> users;
private:

    ClientsMapArray ClientsMap;
    ThreadInfo AcceptThInfo;
    ClientID LastClientID;
    SOCKET AcceptSock;
    pthread_mutex_t Mut;
};