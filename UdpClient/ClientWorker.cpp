#include "ClientWorker.h"

ClientWorker::ClientWorker()
{}

ClientWorker::~ClientWorker()
{}

void* ClientWorker::runner(void* args)
{
	string* c = (string*)args;
	if (c != NULL)
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
					//instance.run(host, port);
                                        instance.ListenLoop(host, port);
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
        pthread_create(&tHandle, 0, runner, (void *) params);
        pthread_join(tHandle, NULL);
}

void ClientWorker::stopThread()
{
	pthread_cancel(tHandle);
}

int ClientWorker::readN(int s, char* buf, int remain, int flags) {
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

void ClientWorker::ListenLoop(string host, unsigned short port)
{
    // init network
    
    server = gethostbyname(host.c_str());
    
    bzero((char *) &servOut, sizeof(servOut));
    servOut.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &servOut.sin_addr.s_addr, (size_t) server->h_length);
    servOut.sin_port = htons(port);
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        
	string buffer;
	short state = 0;
	int error = 0;
	int c = 0;
	state = START;
	string buf;
	unsigned int answerCode;
	unsigned short numArgCount;
	string* args = NULL;

	bool isLogged = false;
	string log;
	string pass;
	bool dr = false;
	bool df = false;
	bool ds = false;

	string uname;
	Message m;
	string mes;
	int mesId = -1;

	while (1)
	{
		//error = WSAGetLastError();
		if (error != 0)
		{
			printf("Socket error: %d", error);
			break;
		}
		switch (state)
		{
		case START:
			sendTo(sockfd, serialize(START, 0, NULL));
			ListenRecv(sockfd, buf);
			answerCode = parse(buf, numArgCount, args);
			if (answerCode != NO_OPERATION)
			{
				if (numArgCount > 0 && args != NULL)
				{
					if (strcmp(args[0].c_str(), API[SERV_OK].c_str()) == 0)
					{
						cout << "Connected to server successfully." << endl;
						state = INIT;
						getchar();
					}
					else
					{
						cout << "Smth went wrong [stcmp]" << endl;
						getchar();
					}
				}
				else
				{
					cout << "Smth went wrong [numarg || args]" << endl;
					getchar();
				}
			}
			else
			{
				cout << "Smth went wrong [ansCode]" << endl;
				getchar();
			}
			break;
		case NO_OPERATION:
			sendTo(sockfd, serialize(NO_OPERATION, 0, NULL));
			ListenRecv(sockfd, buf);
			answerCode = parse(buf, numArgCount, args);
			cout << "Enter a valid operation number.\n";
			break;
		case ANSWER:
			cout << "Got the ANSWER message from server." << endl;
			break;
		case INIT:
			df = false;
			while (!df)
			{
				cout << "* MAIL *\n" << "Select the following items:\n" << "2 - Exit\n" << "3 - Register\n" << "4 - Login\n" << OPENT << endl;
				short op; //= getchar() - (int)'0';
				cin >> op;
				df = true;
				switch (op)
				{
				case 2:
					state = EXIT;
					break;
				case 3:
					state = REG;
					break;
				case 4:
					state = LOG;
					break;
				default:
					df = false;
					printf("Not valid operation number.");
					getchar();

					//sendTo(socket, serialize(INIT, 0, NULL));
					//ListenRecv(socket, buf);
					//answerCode = parse(buf, numArgCount, args);
				}
			}
			break;
		case OPCODE:

			break;
		case EXIT:
			sendTo(sockfd, serialize(EXIT, 0, NULL));
			ListenRecv(sockfd, buf);
			answerCode = parse(buf, numArgCount, args);
			return;
			break;
		case REG:
			dr = false;
			while (!dr)
			{
				cout << "You are about to sign up. Enter the <username>: ";
				cin >> log;
				cout << "Enter the <password>: ";
				cin >> pass;
				if (log.size() > 0 && pass.size() > 0)
				{
					string * bufs = new string[2];
					bufs[0] = log;
					bufs[1] = pass;
					sendTo(sockfd, serialize(REG, 2, bufs));
					delete[] bufs;
					ListenRecv(sockfd, buf);
					answerCode = parse(buf, numArgCount, args);
					if (answerCode != NO_OPERATION)
					{
						if (numArgCount > 0 && args != NULL)
						{
							if (args[0].compare(API[SERV_OK]) == 0)
							{
								dr = true;
								state = INIT;
								cout << "User created successfully. Press any key.\n" << endl;
								getchar();
							}
							else
							{
								if (numArgCount > 1)
								{
									cout << "ERROR: while signing up" << args[1] << "]\n";
								}
								else cout << "Error while signing up.";
							}
						}
					}
					else
					{
						if (numArgCount > 0 && args != NULL)
							cout << "ERROR: while creating user err[" << args[0] << "]\n";
						else cout << "Unknown error.\n";
						getchar();
					}
				}
				else
				{
					printf("Login or password missing.\n");
					getchar();
				}
			}
			break;
		case LOG:
			dr = false;
			while (!dr)
			{
				cout << "You are about to sign in. Enter the <username>: ";
				cin >> log;
				cout << "Enter the <password>: ";
				cin >> pass;
				if (log.size() > 0 && pass.size() > 0)
				{
					string * bufs = new string[2];
					bufs[0] = log;
					bufs[1] = pass;
					sendTo(sockfd, serialize(LOG, 2, bufs));
					delete[] bufs;
					ListenRecv(sockfd, buf);
					answerCode = parse(buf, numArgCount, args);
					if (answerCode != NO_OPERATION)
					{
						if (numArgCount > 0 && args != NULL)
						{
							if (args[0].compare(API[SERV_OK]) == 0)
							{
								dr = true;
								state = INSYS;
								cout << "User signed in successfully. Press any key.\n" << endl;
								getchar();
							}
							else
							{
								if (numArgCount > 1)
								{
									cout << "ERROR: while signing in" << args[1] << "]\n";
								}
								else cout << "Error while signing in.";
							}
						}
					}
					else
					{
						if (numArgCount > 0 && args != NULL)
							cout << "ERROR: while logging in err[" << args[0] << "]\n";
						else cout << "Unknown error.\n";
						getchar();
					}
				}
				else
				{
					printf("Login or password missing.\n");
					getchar();
				}
			}
			break;
		case INSYS:
			ds = false;
			while (!ds)
			{
				cout << "* MAIL *\n" << "Select the following items:\n" << "1 - Send message\n" << "2 - Exit\n" << "3 - Register\n" << "4 - Logout\n" << "5 - Delete user\n" << "6 - Show unread messages\n" << "7 - Show all messages\n" << "8 - Show the exact message\n" << "9 - Delete message\n" << "10 - Resend message\n" << endl;
				short op;
				cin >> op;
				ds = true;
				switch (op)
				{
				case 1:
					state = SND;
					break;
				case 2:
					state = EXIT;
					break;
				case 3:
					state = REG;
					break;
				case 4:
					state = LUG;
					break;
				case 5:
					state = DEL_US;
					break;
				case 6:
					state = SH_UNR;
					break;
				case 7:
					state = SH_ALL;
					break;
				case 8:
					state = SH_EX;
					break;
				case 9:
					state = DEL_MES;
					break;
				case 10:
					state = RSND;
					break;
				default:
					ds = false;
					printf("Not valid operation number.");
					getchar();
				}
			}
			break;
		case LUG:
			cout << "Logging out.\n" << endl;
			sendTo(sockfd, serialize(LUG, 0, NULL));
			ListenRecv(sockfd, buf);
			answerCode = parse(buf, numArgCount, args);
			if (args[0].compare(API[SERV_OK]) == 0)
			{
				cout << "Log out successfully. Press any key." << endl;
				state = INIT;
			}
			else cout << "Error while logging out. Press any key." << endl;
			getchar();
			break;
		case SND:
			cout << "Sending the message. Enter the username of the user you would like to send: " << endl;
			cin >> uname;
			cout << "Sending the message. Enter the message to send: " << endl;
			cin >> mes;
			m.clear();
			m.body = mes;
			if (uname.size() > 0)
			{
				string * bufs = new string[2];
				bufs[0] = uname;
				bufs[1] = m.serialize();
				sendTo(sockfd, serialize(SND, 2, bufs));
				delete[] bufs;
				ListenRecv(sockfd, buf);
				answerCode = parse(buf, numArgCount, args);
				if (answerCode != NO_OPERATION)
				{
					if (numArgCount > 0 && args != NULL)
					{
						if (args[0].compare(API[SERV_OK]) == 0)
						{
							if (m.deserialize(args[1]))
							{
								m.body = mes;
								cout << "Message successfully sent to user " << uname << endl;
								cout << MessageToString(m) << endl;
								cout << "\nPress any key." << endl;
								state = INSYS;
								getchar();
							}
						}
						else
						{
							if (numArgCount > 1)
							{
								cout << "ERROR: while sending [" << args[1] << "]\n";
							}
							else cout << "Error while sending.";
							state = INSYS;
						}
					}
				}
				else
				{
					if (numArgCount > 0 && args != NULL)
						cout << "ERROR: while sending err[" << args[0] << "]\n";
					else cout << "Unknown error.\n";
					state = INSYS;
					getchar();
				}
			}
			break;
		case DEL_US:
			cout << "Deleting user." << endl;
			sendTo(sockfd, serialize(DEL_US, 0, NULL));
			ListenRecv(sockfd, buf);
			answerCode = parse(buf, numArgCount, args);
			if (answerCode != NO_OPERATION)
			{
				if (numArgCount > 0 && args != NULL)
				{
					if (args[0].compare(API[SERV_OK]) == 0)
					{
						cout << "User <" << uname << "> deleted successfully." << endl;
						cout << "\nPress any key." << endl;
						state = INIT;
						getchar();
					}
					else
					{
						if (numArgCount > 1)
						{
							cout << "ERROR: while deleting [" << args[1] << "]\n";
						}
						else cout << "Error while deleting.";
						state = INSYS;
					}
				}
			}
			else
			{
				if (numArgCount > 0 && args != NULL)
					cout << "ERROR: while deleting err[" << args[0] << "]\n";
				else cout << "Unknown error.\n";
				state = INSYS;
				getchar();
			}
			break;
		case DEL_MES:
			cout << "Deleting message." << endl;
			cout << "Enter the message ID which you would like to delete:" << endl;
			cin >> mesId;
			if (mesId > 0)
			{
				stringstream sss;
				string bufs[1];
				sss << mesId;
				bufs[0] = sss.str();
				sendTo(sockfd, serialize(DEL_MES, 1, bufs));
				ListenRecv(sockfd, buf);
				answerCode = parse(buf, numArgCount, args);
				if (answerCode != NO_OPERATION)
				{
					if (numArgCount > 0 && args != NULL)
					{
						if (args[0].compare(API[SERV_OK]) == 0)
						{
							cout << "Message <" << mesId << "> deleted successfully." << endl;
							cout << "\nPress any key." << endl;
							state = INIT;
							getchar();
						}
						else
						{
							if (numArgCount > 1)
							{
								cout << "ERROR: while deleting [" << args[1] << "]\n";
							}
							else cout << "Error while deleting.";
							state = INSYS;
						}
					}
				}
				else
				{
					if (numArgCount > 0 && args != NULL)
						cout << "ERROR: while deleting err[" << args[0] << "]\n";
					else cout << "Unknown error.\n";
					state = INSYS;
					getchar();
				}
			}
			break;
		case SH_UNR:
			cout << "Showing unread messages." << endl;
			sendTo(sockfd, serialize(SH_UNR, 0, NULL));
			ListenRecv(sockfd, buf);
			answerCode = parse(buf, numArgCount, args);
			if (answerCode != NO_OPERATION)
			{
				if (numArgCount > 0 && args != NULL)
				{
					if (args[0].compare(API[SERV_OK]) == 0)
					{
						for (int i = 1; i < numArgCount; i++)
						{
							if (m.deserialize(args[i]))
							{
								cout << "Unread message with ID = " << m.id << endl;
								cout << MessageToString(m) << endl;
							}
						}
						cout << "\nPress any key." << endl;
						state = INSYS;
						getchar();

					}
					else
					{
						if (numArgCount > 1)
						{
							cout << "ERROR: while showing unread messages [" << args[1] << "]\n";
						}
						else cout << "Error while showing unread messages.";
						state = INSYS;
					}
				}
			}
			else
			{
				if (numArgCount > 0 && args != NULL)
					cout << "ERROR: while showing unread messages err[" << args[0] << "]\n";
				else cout << "Unknown error.\n";
				state = INSYS;
				getchar();
			}
			break;
		case SH_ALL:
			cout << "Showing all messages." << endl;
			sendTo(sockfd, serialize(SH_ALL, 0, NULL));
			ListenRecv(sockfd, buf);
			answerCode = parse(buf, numArgCount, args);
			if (answerCode != NO_OPERATION)
			{
				if (numArgCount > 0 && args != NULL)
				{
					if (args[0].compare(API[SERV_OK]) == 0)
					{
						for (int i = 1; i < numArgCount; i++)
						{
							if (m.deserialize(args[i]))
							{
								cout << "Message with ID = " << m.id << endl;
								cout << MessageToString(m) << endl;
							}
						}
						cout << "\nPress any key." << endl;
						state = INSYS;
						getchar();

					}
					else
					{
						if (numArgCount > 1)
						{
							cout << "ERROR: while showing all messages [" << args[1] << "]\n";
						}
						else cout << "Error while showing all messages.";
						state = INSYS;
					}
				}
			}
			else
			{
				if (numArgCount > 0 && args != NULL)
					cout << "ERROR: while showing unread messages err[" << args[0] << "]\n";
				else cout << "Unknown error.\n";
				state = INSYS;
				getchar();
			}
			break;
		case SH_EX:
			cout << "Showing the exact message." << endl;
			cout << "Enter the message ID which you would like to get: " << endl;
			cin >> mesId;
			if (mesId > 0)
			{
				stringstream sss;
				string bufs[1];
				sss << mesId;
				bufs[0] = sss.str();
				sendTo(sockfd, serialize(SH_EX, 1, bufs));
				ListenRecv(sockfd, buf);
				answerCode = parse(buf, numArgCount, args);
				if (answerCode != NO_OPERATION)
				{
					if (numArgCount > 0 && args != NULL)
					{
						if (args[0].compare(API[SERV_OK]) == 0)
						{
							if (m.deserialize(args[1]))
							{
								cout << "Message with ID = " << m.id << endl;
								cout << MessageToString(m) << endl;
							}
							cout << "\nPress any key." << endl;
							state = INSYS;
							getchar();

						}
						else
						{
							if (numArgCount > 1)
							{
								cout << "ERROR: while showing unread messages [" << args[1] << "]\n";
							}
							else cout << "Error while showing unread messages.";
							state = INSYS;
						}
					}
				}
				else
				{
					if (numArgCount > 0 && args != NULL)
						cout << "ERROR: while showing unread messages err[" << args[0] << "]\n";
					else cout << "Unknown error.\n";
					state = INSYS;
					getchar();
				}
			}
			break;
		case RSND:
			cout << "Resending the exact message." << endl;
			cout << "Resending the message. Enter the username of the user you would like to send: " << endl;
			cin >> uname;
			cout << "Enter the message ID to resend: " << endl;
			cin >> mesId;
			if (uname.size() > 0)
			{
				stringstream sss;
				string bufs[2];
				sss << mesId;
				bufs[0] = sss.str();
				bufs[1] = uname;
				sendTo(sockfd, serialize(RSND, 2, bufs));
				//delete[] bufs;
				ListenRecv(sockfd, buf);
				answerCode = parse(buf, numArgCount, args);
				if (answerCode != NO_OPERATION)
				{
					if (numArgCount > 1 && args != NULL)
					{
						if (args[0].compare(API[SERV_OK]) == 0)
						{
							if (m.deserialize(args[1]))
							{
								cout << "Message successfully resent to user " << uname << endl;
								cout << MessageToString(m) << endl;
								cout << "\nPress any key." << endl;
								state = INSYS;
								getchar();
							}
						}
						else
						{
							cout << "ERROR: while resending [" << args[1] << "]\n";
							state = INSYS;
						}
					}
				}
				else
				{
					if (numArgCount > 0 && args != NULL)
						cout << "ERROR: while resending err[" << args[0] << "]\n";
					else cout << "Unknown error.\n";
					getchar();
				}
			}
			break;
		default:
			printf("Smth went wrong [non existant state].");
			getchar();
		}
	}
}

string ClientWorker::MessageToString(const Message& m)
{
	stringstream res;
	res << "ID: ";
	res << m.id;
	res << "\n";

	res << "TIME: ";
	res << m.date_time;
	res << "\n";

	res << "FROM: ";
	res << m.username;
	res << "\n";

	res << "LEN: ";
	res << m.len;
	res << "\n";

	res << "STATE: ";
	res << MESSAGE_STATES[m.state];
	res << "\n";

	res << "BODY: ";
	res << m.body;
	res << "\n";
	return res.str();
}

string ClientWorker::serialize(unsigned int opcode, unsigned short numarg, const string * ss)
{
	stringstream sstr;
	sstr << API[opcode] << DELIM_PARSE << numarg << DELIM_PARSE;
	if (numarg > 0 && ss != NULL)
		for (int i = 0; i < numarg; i++)
		{
			string temp = ss[i];
			std::replace(temp.begin(), temp.end(), DELIM_PARSE, ' ');
			sstr << temp << DELIM_PARSE;
		}
	return sstr.str();
}

unsigned int ClientWorker::parse(const string& input, unsigned short& numarg, string* &args)
{
	unsigned int res = NO_OPERATION;
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
			args = new string[numarg - 1];
			string opcodeBuf;
			unsigned short cc = 0;
			for (int i = 0; i < input.size(); i++)
			{
				if (input[i] == DELIM_PARSE)
				{
					if (cc == 0)
					{
						opcodeBuf = buf.str();
					}
					else if (cc > 1)
					{
						args[cc - 2] = buf.str();
					}
					cc++;
					buf.str(std::string());
				}
				else
				{
					buf << input[i];
				}
			}
			// args[0] is operation code
			res = parseOpCode(opcodeBuf);
			numarg -= 2;
		}
	}
	return res;
}

int ClientWorker::parseOpCode(const string& buf)
{
	if (API_SIZE > 0)
		for (int i = 0; i < API_SIZE; i++)
			if (buf.compare(API[i]) == 0)
				return i;
	return NO_OPERATION;
}

void ClientWorker::sendTo(int socket, const string& message)
{
	cout << "Send to server: " << message << endl;
	int res = 0;
	int size = message.size();
	stringstream ss;
	ss << size;
	string s = ss.str();
	while (s.size() < 10)
	{
		s.insert(s.begin(), '0');
	}
	s += message;
        int num = ++lastPacketNumSend;
		stringstream ss1;
		ss1 << num;
		string s1 = ss1.str();
		while (s1.size() < 10)
		{
		s1.insert(s1.begin(), '0');
                }
		s1 += s;
		s = s1;
	res = sendto(socket, s.c_str(), s.size(), 0 , (struct sockaddr *) &servOut, sizeof(servOut));
	printf("String to send: %s", s.c_str());
	if (res != s.size())
		printf("Send failed: %i != %i!\n", res, (int)s.length());
}

bool ClientWorker::ListenRecv(int socket, std::string& MsgStr)
{
	char buffer[4096];
        size_t len = 10;
        size_t res = 0;
        stringstream buf;
        MsgStr.clear();
		/*while (MsgStr.length() < len)
		{*/
			
				res = recvfrom(socket, buffer, 4096, 0, (struct sockaddr *) &servIn, &servIn_size);
                                buf << buffer;
                                string s = buf.str();
                                s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
                                s.erase(std::remove(s.begin(), s.end(), '\0'), s.end());
                                
                                string str = s;
                                size_t num = 10;
                                if (s.length() > num)
                                {
                                        string c = str.substr(0, num);
                                        str = str.substr(num, str.length() - 1);
                                        num = atoi(c.c_str());
                                }
                                if (lastPacketNumRecv + 1 == num)
                                {
                                        s = str;
                                        lastPacketNumRecv = num;
                                
                                        MsgStr+=s;
                                        // parse len
                                        if (MsgStr.length() >= len)
                                        {
                                                string c = MsgStr.substr(0, len);
                                                MsgStr = MsgStr.substr(len, MsgStr.length() - 1);
                                                len = atoi(c.c_str());
                                        }
                                }
                                else cout << "Packet order mismatch!\n";
		//}
	cout << "Recieved: " << MsgStr << endl;
	return true;
}
