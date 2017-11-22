#include "stdinc.h"
#include "CUdpServer.h"

CUdpServer::CUdpServer() {
	//AcceptSock = 0;
	//LastClientID = 0;
	//Mut = CreateMutex(NULL, FALSE, L"UdpServer");
}

CUdpServer::CUdpServer(const CUdpServer& orig) {
}

CUdpServer::~CUdpServer() {
	CloseHandle(Mut);
}

DWORD WINAPI CUdpServer::ListenThread(LPVOID pParam) {
	printf("UDP thread is run\r\n");
	/*ListenThInput* pOldData = (ListenThInput*)pParam;
	ListenThInput pData = (*pOldData);*/
	//delete pOldData;
	ThreadData* l = (ThreadData*)pParam;
	if (l != nullptr)
	{
		ServerWorker w;
		w.init(l);
		bool res = w.mainLoop();
		if (res)
			printf("Client %s terminated successfully!\n", inet_ntoa(l->address.sin_addr));
		else
			printf("Client %s terminated abnormally!\n", inet_ntoa(l->address.sin_addr));
	}
	else
	{
		cout << "Received null data sructure!" << endl;
		return -1;
	}
	//pthread_exit(0);
	return 0;
};


DWORD WINAPI CUdpServer::AcceptThread(LPVOID pParam)
{
	if (pParam != nullptr)
	{
		WSADATA wsaData;
		unsigned short port = *((USHORT*)pParam);
		unsigned int n;
		n = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (n != 0) {
			printf("WSAStartup failed: %ui\n", n);
			return 1;
		}

		//AcceptThInput* pOldData = (AcceptThInput*)pParam;
		//AcceptThInput pData = (*pOldData);
		//delete pOldData;
		SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
		if (s == -1) {
			printf("Socket create failed\r\n");
			return -1;
		}

		ThreadData* clients = nullptr;
		unsigned int cSize = 0;

		sockaddr_in service;
		service.sin_family = AF_INET;
		service.sin_addr.s_addr = INADDR_ANY;
		service.sin_port = htons(port);

		int res;
		res = bind(s, (sockaddr *)& service, sizeof(service));
		if (res != 0)
		{
			printf("Bind failed\r\nFailed to bind %d port.", port);
			port++;
			service.sin_port = htons(port);
			bind(s, (sockaddr *)& service, sizeof(service));
		}
		printf("MAIL SERVER\nPort:%d\r\n", (int)port);

		while (1)
		{
			//printf("Waiting...\n");
			processInput(s, clients, cSize);
			processOutput(s, clients, cSize);

			/*if (recvfrom(s, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromlen) != SOCKET_ERROR)
			{
				string str = buffer;
				//printf("Received message from %s: %s\n", inet_ntoa(from.sin_addr), buffer);
				processInput(str);
				processOutput(s, str);
			}*/
			Sleep(100);
		}
		closesocket(s);
		//if (listen((*pData.pAcceptSock), SOMAXCONN) == -1) {
		//	printf("Listen failed\r\n");
		//}
		return 0;
	}
	return -1;
};


//If client has current IP, then returns client ID, else it creates a new thread, puts new client into DB and returns new ID. 
unsigned int CUdpServer::checkClient(const sockaddr_in& saddr, ThreadData* &clients, unsigned int& cSize)
{
	if (clients != nullptr)
	{
		if (cSize > 0)
		{
			for (int i = 0; i < cSize; i++)
			{
				if (saddr.sin_addr.S_un.S_addr == clients[i].address.sin_addr.S_un.S_addr && saddr.sin_family == clients[i].address.sin_family && saddr.sin_port == clients[i].address.sin_port && saddr.sin_zero == clients[i].address.sin_zero)
				{
					return i; // we found our client
				}
			}
		}
		// let's create a new client thread
		cSize++;
		ThreadData * arr = new ThreadData[cSize];
		if (cSize > 1)
			for (unsigned int i = 0; i < cSize - 1; i++)
				arr[i] = clients[i];
		delete[] clients;

		wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;

		clients = arr;

		clients[cSize - 1].address = saddr;

		clients[cSize - 1].rBuf = new string();

		clients[cSize - 1].rMutexName = converter.from_bytes(inet_ntoa(saddr.sin_addr));
		clients[cSize - 1].rMutexName += L"_";
		clients[cSize - 1].rMutexName += converter.from_bytes(itoa(saddr.sin_port, nullptr, 10));
		clients[cSize - 1].rMutexName += L"_read";

		clients[cSize - 1].sBuf = new string();

		clients[cSize - 1].sMutexName = converter.from_bytes(inet_ntoa(saddr.sin_addr));
		clients[cSize - 1].sMutexName += L"_";
		clients[cSize - 1].sMutexName += converter.from_bytes(itoa(saddr.sin_port, nullptr, 10));
		clients[cSize - 1].sMutexName += L"_write";

		// lock read mutex
		HANDLE m = CreateMutex(NULL, FALSE, clients[cSize - 1].rMutexName.c_str());
		DWORD result;
		result = WaitForSingleObject(m, INFINITE);
		if (result != WAIT_OBJECT_0)
		{
			cout << "Failed to lock mutex!" << endl;
			cSize--;
			return 0;
		}

		DWORD t;
		clients[cSize - 1].tHandle = CreateThread(0, 0, ListenThread, (void*)&clients[cSize - 1], 0, &t);
		clients[cSize - 1].tId = t;

		return cSize;
	}
	return 0;
}

void CUdpServer::processInput(const SOCKET& s, ThreadData* &clients, unsigned int& cSize)
{
	struct sockaddr_in from;
	int fromlen = sizeof(from);
	char buffer[1024];
	ZeroMemory(buffer, sizeof(buffer));
	if (recvfrom(s, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromlen) != SOCKET_ERROR)
	{
		unsigned int pos = checkClient(from, clients, cSize);
		HANDLE m = CreateMutex(NULL, FALSE, clients[pos].rMutexName.c_str());
		// write data into read buffer of clients[pos]
		if (clients[pos].rBuf != nullptr) 
			clients[pos].rBuf->append(buffer);
		// unlock read mutex of clients[pos]
		CloseHandle(m);
		// wait for 50 ms
		Sleep(50);
		// lock read mutex of clients[pos]
		m = CreateMutex(NULL, FALSE, clients[pos].rMutexName.c_str());
		DWORD result;
		result = WaitForSingleObject(m, INFINITE);
		if (result != WAIT_OBJECT_0)
		{
			cout << "Failed to lock mutex!" << endl;
		}
	}
}

void CUdpServer::processOutput(const SOCKET& s, ThreadData* clients, const unsigned int& cSize)
{
	//sendto(s, str.c_str(), str.length(), 0, (sockaddr*)&from, fromlen);
	if (cSize > 0 && clients != nullptr)
	{
		for (int i = 0; i < cSize; i++)
		{
			//Lock write mutex for each client; 

			//read data from sBuf;
			
			//clean sBuf;

			//unlock mutex;

		}
	}
}

void CUdpServer::LockMutex(HANDLE& m)
{
	m = CreateMutex(NULL, FALSE, L"UdpServer");
	DWORD result;
	result = WaitForSingleObject(m, INFINITE);
	if (result != WAIT_OBJECT_0)
		cout << "Failed to lock mutex!" << endl;
}

void CUdpServer::UnlockMutex(HANDLE& m)
{
	CloseHandle(m);
}

void CUdpServer::StartAccept(USHORT port)
{
	LockMutex(Mut);

	//AcceptThInfo.ThPort = Port;
	//AcceptThInfo.bCreated = true;

	//AcceptThInput* ThInput = new AcceptThInput;
	/*ThInput->ThPort = Port;
	ThInput->pAcceptSock = &AcceptSock;
	ThInput->pParent = this;

	//pthread_create(&AcceptThInfo.ThHandle, 0, AcceptThread, (void *)ThInput);*/
	DWORD t;
	USHORT* pp = new USHORT;
	*pp = port;
	serverThread = CreateThread(0, 0, AcceptThread, (void *)pp, 0, &t);
	UnlockMutex(Mut);
}

//void CUdpServer::StartListenTh(SOCKET Sock)
//{
	/*LockMutex(Mut);
	ClientInfo CliInfo;
	CliInfo.ID = LastClientID++;
	CliInfo.ClientSocket = Sock;

	ListenThInput* pThInput = new ListenThInput;
	pThInput->CliID = CliInfo.ID;
	pThInput->ClientSocket = Sock;
	pThInput->pParent = this;

	//pthread_create(&(CliInfo.ClientThreadInfo.ThHandle), NULL, ListenThread, pThInput);

	DWORD t;
	HANDLE tHandle = CreateThread(0, 0, ListenThread, pThInput, 0, &t);
	clientsSize++;
	HANDLE * arr = new HANDLE[clientsSize];
	if (clientsSize > 1)
		for (unsigned int i = 0; i < clientsSize - 1; i++)
			arr[i] = clients[i];
	clients = arr;
	clients[clientsSize - 1] = tHandle;
	UnlockMutex(Mut);*/
	//}
