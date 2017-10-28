#pragma once

#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string>

#define DELIM ':'

using namespace std;

class ClientWorker
{
public:
	ClientWorker();
	~ClientWorker();

	void startThread(string* params);
	void stopThread();
	static DWORD WINAPI runner(LPVOID args);
	bool isRunning = false;
private:
	void run(string host, unsigned short port);
	//int socket;
	int readN(SOCKET s, char* buf, int remain, int flags);
	HANDLE tHandle;
};
