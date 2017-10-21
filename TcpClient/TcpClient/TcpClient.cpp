#include "stdinc.h"
#include "TcpClient.h"

CTcpClient::CTcpClient(void)
{
	InitializeCriticalSectionAndSpinCount(&CS, 0x00000400);
	
	WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != NO_ERROR)
	{
		printf("Can't init WSA\r\n");
		WSACleanup();
    }

	ConnectSock=0;
}

CTcpClient::~CTcpClient(void)
{
	WSACleanup();
	DeleteCriticalSection(&CS);
}

bool CTcpClient::Connect(char* IP, USHORT Port)
{
	Disconnect();
	ConnectAddr.sin_family = AF_INET;
    ConnectAddr.sin_addr.s_addr = inet_addr(IP);
    ConnectAddr.sin_port = htons(Port);

	ConnectSock = socket(AF_INET, SOCK_STREAM, 0);
    if (ConnectSock == INVALID_SOCKET)
    {
        printf("Socket create failed\r\n");
        return false;
    }

    int res=0;
    res = connect(ConnectSock, (sockaddr *) & ConnectAddr, sizeof (ConnectAddr));
    if (res == SOCKET_ERROR) 
    {
		printf("Connect to %s:%d failed\r\n",IP,(int)Port);
        return false;
    } else  printf("Client connected\r\n");

	ListenThInput* pThInput=new ListenThInput;
	pThInput->ClientSocket=ConnectSock;
	pThInput->pParent=this;
	
	ListenThInfo.bCreated=true;
	ListenThInfo.ThHandle=CreateThread(NULL,0,ListenThread,pThInput,NULL,&(ListenThInfo.ThID));
	return true;
}

void CTcpClient::Disconnect(bool bWaitForThreadClosed)
{
	if(ConnectSock!=0)
	{
		if(shutdown(ConnectSock,SD_BOTH)==SOCKET_ERROR) {};
		if(closesocket(ConnectSock)==SOCKET_ERROR) {};
	}

	if((bWaitForThreadClosed) && (ListenThInfo.ThHandle!=0))
	{
		CloseThread(ListenThInfo.ThHandle,true);
	};

	ListenThInfo.bCreated=false;
	ListenThInfo.ThHandle=NULL;
	ListenThInfo.ThID=0;

	ConnectSock=0;
}

void CTcpClient::Send(char* Msg)
{
    int res=0;
    res=send(ConnectSock,(char*)Msg,strlen(Msg)+1,0);
    if (res == SOCKET_ERROR) printf("Send failed\r\n");
}

void CTcpClient::OnMessage(std::string Msg)
{
	printf("MSG: %s\r\n",(char*)Msg.c_str());
}

DWORD WINAPI ListenThread(void* pParam)
{
    printf("Listen thread started\r\n");
    ListenThInput* pOldData=(ListenThInput*)pParam;
    ListenThInput pData=(*pOldData);
    delete pOldData;

    char recvbuf[1024];
    std::string MessageBuf;

    int res;
    bool bQuit=false;
    while(!bQuit)
    {
        res=recv(pData.ClientSocket, recvbuf, 1024, 0);

        if(res==0) {printf("Connection closed\r\n"); bQuit=true;}
        else if(res==-1) {printf("Receive failed\r\n"); bQuit=true;}
        else
        {
            for(int i=0;i<res;i++)
                MessageBuf.push_back(recvbuf[i]);

            int t=0;
            std::string Str;
            while((t=MessageBuf.find_first_of('\0'))!=-1)
            {
                Str=MessageBuf.substr(0,t);
                //Str.replace('\r',' ');
                MessageBuf.erase(0,t+1);
                pData.pParent->OnMessage(Str);
            };
        };
    }
    pData.pParent->Disconnect(false);
	return 0;
};


void LockCS(CRITICAL_SECTION* mut)
{
#ifdef PRINT_DEADLOCK
	bool bDeadlock=false;
	if(mut->LockCount!=-1) 
	{
		printf("--TCPIO DEADLOCKED\r\n");
		bDeadlock=true;
	}
#endif
EnterCriticalSection(mut);
#ifdef PRINT_DEADLOCK
	if(bDeadlock) 
	{
		printf("--TCPIO UNLOCKED\r\n");
	}
#endif
};

void UnlockCS(CRITICAL_SECTION* mut)
{
	LeaveCriticalSection(mut);
};

void CloseThread(HANDLE ThH,bool bTerminate)
{
	printf("Waiting to thread close\r\n");
    DWORD dw=WaitForSingleObject(ThH,50);
    if(dw==WAIT_TIMEOUT)
    {
		if(bTerminate) 
		{
			printf("Thread terminated\r\n");
			TerminateThread(ThH,0);
		};
    };
    CloseHandle(ThH);
}
