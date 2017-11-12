#include "ClientWorker.h"

ClientWorker::ClientWorker()
{}

ClientWorker::~ClientWorker()
{}

DWORD WINAPI ClientWorker::runner(LPVOID args)
{
	string* c = (string*)args;
	if (c != nullptr)
	{
		string s = *c;
		string host = "";
		unsigned short port = 0;
		bool divOccured = true;
		string buf = "";
		if (s.size() > 0)
		{
			for (unsigned long i = 0; i < s.size(); i++)
			{
				if (s[i] == DELIM)
					divOccured = false;
				else
				{
					if (divOccured)
						host += s[i];
					else
						buf += s[i];
				}
			}
			if (host.size() > 0 && buf.size() > 0)
			{
				port = atoi(buf.c_str());
				if (port > 0)
				{
					static ClientWorker instance;
					instance.run(host, port);
				}
				else
					printf("Port is not valid.");
			}
			else
				printf("Missing host or port definition.");
		}
		else
			printf("Missing the data.");
	}
	return 0;
}

void ClientWorker::startThread(string* params)
{
	DWORD t;
	tHandle = CreateThread(0, 0, runner, params, 0, &t);
}

void ClientWorker::stopThread()
{
	CloseHandle(tHandle);
}

int ClientWorker::readN(SOCKET s, char* buf, int remain, int flags) {
	int rcb = 0;
	int rc;
	while (remain)
	{
		rc = recv(s, buf + rcb, remain, flags);
		if (rc < 1) {
			return rc;
		}
		rcb += rc;
		remain -= rc;
	}

	return rcb;
}

void ClientWorker::ListenLoop(const int& socket)
{
	string buffer;
	short state = 0;
	int error = 0;
	int c = 0;
	state = STATE::START;
	string buf;
	unsigned int answerCode;
	unsigned short numArgCount;
	string* args;

	while (1)
	{
		error = WSAGetLastError();
		if (error != 0)
		{
			printf("Socket error: %d", error);
			break;
		}
		switch (state)
		{
		case STATE::START:
			sendTo(socket, serialize(STATE::START, 0, nullptr));
			ListenRecv(socket, buf);
			answerCode = parse(buf, numArgCount, args);
			if (answerCode != STATE::NO_OPERATION)
			{
				if (numArgCount > 0 && args != nullptr)
				{
					if (strcmp(args[0].c_str(), API[STATE::SERV_OK].c_str()) == 0)
					{
						cout << "Connected to server successfully." << endl;
						state = STATE::INIT;
					}
					else 
						cout << "Smth went wrong [stcmp]" << endl;
				}
				else
					cout << "Smth went wrong [numarg || args]" << endl;
			}
			else
				cout << "Smth went wrong [ansCode]" << endl;
			break;
		case STATE::NO_OPERATION:

			break;
		case STATE::ANSWER:

			break;
		case STATE::INIT:
			cout << "* MAIL *\n" << "2 - Exit\n" << "3 - Register\n" << "4 - Login\n" << endl;
			break;
		case STATE::OPCODE:

			break;
		case STATE::EXIT:

			break;
		case STATE::REG:

			break;
		case STATE::LOG:

			break;
		default:
			printf("Smth went wrong.");
		}
	}
}

string ClientWorker::serialize(unsigned int opcode, unsigned short numarg, const string * ss)
{
	stringstream sstr;
	sstr << opcode << DELIM_PARSE << numarg << DELIM_PARSE;
	if (numarg > 0 && ss != nullptr)
		for (int i = 0; i < numarg - 1; i++)
		{
			string temp = ss[i];
			std::replace(temp.begin(), temp.end(), DELIM_PARSE, ' ');
			sstr << temp << DELIM_PARSE;
		}
	return sstr.str();
}

unsigned int ClientWorker::parse(const string& input, unsigned short& numarg, string* args)
{
	unsigned int res = STATE::NO_OPERATION;
	if (input.size() > 0)
	{
		stringstream buf;
		numarg = 0;
		// find all delimeters
		for (int i = 0; i < input.size(); i++)
		{
			if (input[i] == DELIM_PARSE)
				numarg++;
		}
		// find all parts
		if (numarg > 0)
		{
			args = new string[numarg-1];
			string opcodeBuf;
			unsigned short cc = 0;
			for (int i = 0; i < input.size(); i++)
			{
				if (input[i] == DELIM_PARSE)
				{
					if (cc == 0)
						opcodeBuf = buf.str();
					else
						args[cc - 1] = buf.str();
					cc++;
					buf.clear();
				}
				else
					buf << input[i];
			}
			// args[0] is operation code
			res = atoi(opcodeBuf.c_str());
			numarg--;
		}
	}	
	return res;
}

void ClientWorker::run(string host, unsigned short port)
{
	printf("Starting new client thread with HOST=%s, PORT=%u\n", host.c_str(), port);
	isRunning = true;
	int n;
	WSADATA wsaData;
	n = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (n != 0) {
		printf("WSAStartup failed: %d\n", n);
	}
	else
	{
		int sockfd;
		struct sockaddr_in serv_addr;
		struct hostent *server;

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			perror("ERROR opening socket");
		}
		else
		{
			server = gethostbyname(host.c_str());
			if (server == NULL) {
				fprintf(stderr, "ERROR, no such host\n");
			}
			else
			{
				memset((char*)&serv_addr, 0, sizeof(serv_addr));
				serv_addr.sin_family = AF_INET;
				memcpy((char *)&serv_addr.sin_addr.s_addr, server->h_addr, (size_t)server->h_length);
				serv_addr.sin_port = htons(port);
				if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {

				}
				else
				{
					ListenLoop(sockfd);
					n = shutdown(sockfd, SD_BOTH);
					if (n == SOCKET_ERROR) {
						printf("shutdown failed: %d\n", WSAGetLastError());
						closesocket(sockfd);
						WSACleanup();
					}
				}
				closesocket(sockfd);
				WSACleanup();
				isRunning = false;
			}
		}
	}
}

void ClientWorker::sendTo(SOCKET socket, const string& message)
{
	cout << "Send to server: " << message << endl;
	int res = 0;
	int size = message.size();
	stringstream ss;
	ss << size;
	string s = ss.str();
	//sprintf(s.c_str(), "%d", size);
	while (s.size() < 10)
	{
		s.insert(s.begin(), '0');
	}
	s += message;
	res = send(socket, s.c_str(), s.size(), 0);
	//printf("String to send: %s", s.c_str());
	if (res != s.size() )
		printf("Send failed: %d != %d!\n", res, s.size());
}

bool ClientWorker::ListenRecv(SOCKET socket, std::string& MsgStr)
{
	char* c = new char[10];
	unsigned int size = 0;
	int ssize = recv(socket, c, 10, 0);
	//printf("Size buffer: %s\n", c);
	if (ssize == 10)
	{
		size = atoi(c);
		//char* recvbuf = (char*)malloc((size)*sizeof(char));
		char* recvbuf = new char[size];

		int res = recv(socket, recvbuf, size, 0);
		//printf("String received: %s\n", recvbuf);

		//printf("Received %d symbols!\n", res);
		if (res > 0)
		{
			MsgStr.clear();
			for (int i = 0; i < res; i++)
				if (recvbuf[i] != '\n' && recvbuf[i] != '\r' && recvbuf[i] != '\t' && recvbuf[i] != '\0')
					MsgStr.push_back(recvbuf[i]);
		}
	}
	else
		return false;
	cout << "Recieved: " << MsgStr << endl;
	return true;
}
