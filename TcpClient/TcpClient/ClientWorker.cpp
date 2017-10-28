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

void ListenLoop(const int& socket)
{
	const unsigned short SIZE = 512;
	char buffer[SIZE];
	unsigned int n;
	int error = 0;
	while (1)
	{
		error = WSAGetLastError();
		if (error != 0)
		{
			printf("Socket error: %d", error);
			break;
		}
		memset(buffer, 0, SIZE);
		n = recv(socket, buffer, SIZE, 0);
		if (n >= 0)
		{
			printf("%s", buffer);
			memset(buffer, 0, SIZE);
			scanf("&s", buffer);
			//send(socket, buffer, strlen(buffer), 0);
		}
		else
		{
			printf("Socket error!");
			break;
		}
	}
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