#pragma once

#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <stdint.h>
#include <string>
#include <sstream> 
#include <algorithm>

#include "API.h"

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
	bool isRunning = true;
	void sendTo(SOCKET s, const string& message);
	bool ListenRecv(SOCKET s, std::string& MsgStr);
	string serialize(unsigned int opcode, unsigned short numarg, const string* ss);
	unsigned int /*opcode*/ parse(const string& input, unsigned short& numarg, string* args);
private:
	void run(string host, unsigned short port);
	int readN(SOCKET s, char* buf, int remain, int flags);
	void ListenLoop(const int& socket);
	HANDLE tHandle;
};
