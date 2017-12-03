/*
* To change this license header, choose License Headers in Project Properties.
* To change this template file, choose Tools | Templates
* and open the template in the editor.
*/

#include <algorithm>

#include "ServerWorker.h"

ServerWorker::ServerWorker() {
}

ServerWorker::~ServerWorker()
{
	closeSocket();
}

void ServerWorker::init(ThreadData* data) {
	td = data;
}

bool ServerWorker::mainLoop() {
	STATE State = NO_OPERATION;
	string MsgStr;
	string MessageBuf;
	string currentUserName;
	string errMessage;
	int mesId;

	string sendToUserName;
	bool isNameValid;
	bool isSent;
	bool mesFound = false;

	string buf;
	bool RegisterState = false, LoginState = false;

	unsigned short numarg;
	string* args = NULL;
	string* args2 = NULL;
	string ff;
	unsigned long size;
	int cc = 0;
	int n = 0;

	unsigned int unread = 0;

	Message * m = NULL;

	while (true) {
		bool res = ListenRecv(MsgStr);
		if (res)
		{
			State = parse(MsgStr, numarg, args);
			switch (State)
			{
			case START:
				args2 = new string[1];
				args2[0] = API[SERV_OK];
				ff = serialize(ANSWER, 1, args2);
				sendTo(ff);
				delete[] args2;
				break;
			case EXIT:
				//printf("Client with ID: %d is disconnected!\n", socket);
				//args2 = new string[1];
				//args2[0] = API[SERV_OK];
				//sendTo(serialize(ANSWER, 1, args2));
				//delete[] args2;
				//closeSocket();
				return true;
				break;
			case REG:
				if (args != NULL && numarg > 1)
				{
					errMessage = RegisterNewUser(args[0], args[1], RegisterState);
					if (RegisterState)
					{
						args2 = new string[1];
						args2[0] = API[SERV_OK];
						sendTo(serialize(ANSWER, 1, args2));
						delete[] args2;
					}
					else
					{
						args2 = new string[2];
						args2[0] = API[NO_OPERATION];
						args2[1] = errMessage;
						sendTo(serialize(ANSWER, 2, args2));
						delete[] args2;
					}
				}
				else
				{
					args2 = new string[2];
					args2[0] = API[NO_OPERATION];
					args2[1] = "Not valid args.";
					sendTo(serialize(ANSWER, 2, args2));
					delete[] args2;
				}
				break;
			case LOG:
				if (args != NULL && numarg > 1)
				{
					errMessage = LoginNewUser(args[0], args[1], LoginState);
					if (LoginState)
					{
						args2 = new string[1];
						args2[0] = API[SERV_OK];
						sendTo(serialize(ANSWER, 1, args2));
						currentUserName = args[0];
						delete[] args2;
					}
					else
					{
						args2 = new string[2];
						args2[0] = API[NO_OPERATION];
						args2[1] = errMessage;
						sendTo(serialize(ANSWER, 2, args2));
						delete[] args2;
					}
				}
				else
				{
					args2 = new string[2];
					args2[0] = API[NO_OPERATION];
					args2[1] = "Not valid args.";
					sendTo(serialize(ANSWER, 2, args2));
					delete[] args2;
				}
				break;
			case LUG:
				cout << "Logging out." << endl;
				currentUserName = "";
				args2 = new string[1];
				args2[0] = API[SERV_OK];
				sendTo(serialize(ANSWER, 1, args2));
				delete[] args2;
				break;
			case SND:
				cout << "Sending the message." << endl;
				if (args != NULL && numarg > 1)
				{
					m = new Message();
					if (m->deserialize(args[1]) && args[0].size() > 0)
					{
						if (currentUserName.size() > 0) { mesId = AddMessage(m, args[0], currentUserName, errMessage); }
						else { mesId = 0; }
						if (mesId == 0)
						{
							args2 = new string[2];
							args2[0] = API[NO_OPERATION];
							args2[1] = "Error while sending the message [" + errMessage + "]";
							sendTo(serialize(ANSWER, 2, args2));
							delete[] args2;
						}
						else
						{
							args2 = new string[2];
							m->body = "";
							args2[0] = API[SERV_OK];
							args2[1] = m->serialize();
							sendTo(serialize(ANSWER, 2, args2));
							delete[] args2;
						}
					}
					delete m;
				}
				break;
			case DEL_US:
				cout << "Deleting user." << endl;
				if (currentUserName.size() > 0)
				{
					errMessage = DeleteUser(currentUserName);
					currentUserName = "";
					args2 = new string[2];
					if (errMessage.size() == 0)
					{
						args2[0] = API[SERV_OK];
						args2[1] = "";
					}
					else
					{
						args2[0] = API[NO_OPERATION];
						args2[1] = errMessage;
					}
					sendTo(serialize(ANSWER, 2, args2));
					delete[] args2;
				}
				break;
			case DEL_MES:
				cout << "Deleting message." << endl;
				if (args != NULL && numarg > 0)
				{
					if (currentUserName.size() > 0)
					{
						errMessage = DeleteMes(currentUserName, args[0]);
						currentUserName = "";
						args2 = new string[2];
						if (errMessage.size() == 0)
						{
							args2[0] = API[SERV_OK];
							args2[1] = "";
						}
						else
						{
							args2[0] = API[NO_OPERATION];
							args2[1] = errMessage;
						}
						sendTo(serialize(ANSWER, 2, args2));
						delete[] args2;
					}
				}
				break;
			case SH_UNR:
				size = 0;
				cout << "Showing unread messages." << endl;
				if (args != NULL && numarg >= 0)
				{
					if (currentUserName.size() > 0)
					{
						Message** mm;
						mm = ReadAllMes(currentUserName, size);
						bool changes = false;
						if (size>0)
						{
							for (unsigned long i = 0; i<size; i++)
							{
								if (mm[i] != NULL)
								{
									//buf.append(MessageToString(m[i][0]));
									if (mm[i][0].state == MSTATE_UNREAD)
									{
										unread++;
									}
								}
							}
							args2 = new string[unread + 1];
							args2[0] = API[SERV_OK];
							cc = 1;
							for (unsigned int i = 0; i<size; i++)
							{
								if (mm[i][0].state == MSTATE_UNREAD)
								{
									args2[cc] = mm[i][0].serialize();
									changes = true;
									mm[i][0].state = MSTATE_NORMAL;
									cc++;
								}
							}
							if (unread > 0) sendTo(serialize(ANSWER, unread + 1, args2));
							if (changes) WriteMessages(currentUserName, mm, size, true);
							for (unsigned long i = 0; i<size; i++)
							{
								if (mm[i] != NULL)
								{
									delete mm[i];
								}
							}
							delete[] args2;
						}
						else
							cout << "No messages found!\n";
					}
					if (unread == 0)
					{
						args2 = new string[2];
						args2[0] = API[NO_OPERATION];
						args2[1] = "Error while showing unread the messages. No messages found.";
						sendTo(serialize(ANSWER, 2, args2));
						delete[] args2;
					}
				}
				break;
			case SH_ALL:
				size = 0;
				cout << "Showing all messages." << endl;
				if (args != NULL && numarg >= 0)
				{
					if (currentUserName.size() > 0)
					{
						Message** mm;

						mm = ReadAllMes(currentUserName, size);
						bool changes = false;
						if (size>0)
						{
							args2 = new string[size + 1];
							args2[0] = API[SERV_OK];
							for (unsigned long i = 1; i <= size; i++)
							{
								if (mm[i - 1] != NULL)
								{
									args2[i] = mm[i - 1][0].serialize();
									if (mm[i - 1][0].state == MSTATE_UNREAD)
									{
										mm[i - 1][0].state = MSTATE_NORMAL;
										changes = true;
									}
								}
							}
							sendTo(serialize(ANSWER, size + 1, args2));
							if (changes) WriteMessages(currentUserName, mm, size, true);
							for (unsigned long i = 0; i<size; i++)
							{
								if (mm[i] != NULL)
								{
									delete mm[i];
								}
							}
							delete[] args2;
						}
						else
							cout << "No messages found!\n";
					}
					if (size == 0)
					{
						args2 = new string[2];
						args2[0] = API[NO_OPERATION];
						args2[1] = "Error while showing all messages.";
						sendTo(serialize(ANSWER, 2, args2));
						delete[] args2;
					}
				}
				break;
			case SH_EX:
				mesFound = false;
				cout << "Showing the exact message." << endl;
				size = 0;
				if (args != NULL && numarg >= 1)
				{
					if (currentUserName.size() > 0)
					{
						mesId = atoi(args[0].c_str());
						Message** mm;
						mm = ReadAllMes(currentUserName, size);
						bool changes = false;
						if (size>0)
						{
							for (unsigned long i = 0; i<size; i++)
							{
								if (mm[i] != NULL)
								{
									//buf.append(MessageToString(m[i][0]));
									if (mm[i][0].id == mesId)
									{
										args2 = new string[2];
										args2[0] = API[SERV_OK];
										args2[1] = mm[i][0].serialize();
										mesFound = true;
										if (mm[i][0].state == MSTATE_UNREAD)
										{
											mm[i][0].state = MSTATE_NORMAL;
											changes = true;
										}
									}
								}
							}
							if (mesFound) sendTo(serialize(ANSWER, 2, args2));
							if (changes) WriteMessages(currentUserName, mm, size, true);
							for (unsigned long i = 0; i<size; i++)
							{
								if (mm[i] != NULL)
								{
									delete mm[i];
								}
							}
							delete[] args2;
						}
						else
							cout << "No messages found!\n";
					}
					if (!mesFound)
					{
						args2 = new string[2];
						args2[0] = API[NO_OPERATION];
						args2[1] = "Error while showing the messages. No messages found.";
						sendTo(serialize(ANSWER, 2, args2));
						delete[] args2;
					}
				}
				break;
			case RSND:
				cout << "Resending the exact message." << endl;
				mesFound = false;
				size = 0;
				if (args != NULL && numarg >= 2)
				{
					if (currentUserName.size() > 0)
					{
						mesId = atoi(args[0].c_str());
						Message** mm;
						mm = ReadAllMes(currentUserName, size);
						bool changes = false;
						if (size>0)
						{
							args2 = new string[2];
							for (unsigned long i = 0; i<size; i++)
							{
								if (mm[i] != NULL)
								{
									if (mm[i][0].id == mesId)
									{
										mesId = AddMessage(mm[i], args[1], currentUserName, errMessage);
										if (mesId == 0)
										{
											args2[0] = API[NO_OPERATION];
											args2[1] = "Error while resending the messages. Aim user not found.";
											//sendTo(serialize(ANSWER, 2, args2));
										}
										else
										{
											args2[0] = API[SERV_OK];
											args2[1] = mm[i][0].serialize();
										}
										mesFound = true;
									}
								}
							}
							if (mesFound) sendTo(serialize(ANSWER, 2, args2));
							for (unsigned long i = 0; i<size; i++)
							{
								if (mm[i] != NULL)
								{
									delete mm[i];
								}
							}
							delete[] args2;
						}
						else
							cout << "No messages found!\n";
					}
					if (!mesFound && size == 0)
					{
						args2 = new string[2];
						args2[0] = API[NO_OPERATION];
						args2[1] = "Error while resending the messages. Message not found.";
						sendTo(serialize(ANSWER, 2, args2));
						delete[] args2;
					}
				}
				break;
			default:
				cout << "Unknown state: " << State << endl;
				break;
			}
		}
	}
	return true;
}

void ServerWorker::openSem(const string& name)
{
	//sem_t *sem = sem_open(name.c_str(), O_CREAT | O_EXCL, 0644, 1);
	//sem_wait(sem_open(name.c_str(), O_CREAT, 0644, 1));
	///if (sem != NULL)
		//sem_wait(sem);
	//int fd_lock = open(LOCK_FILE, O_CREAT);
	//flock(fd_lock, LOCK_EX);
	// do stuff
	//flock(fd_lock, LOCK_UN);
}

void ServerWorker::closeSem(const string& name)
{
	//sem_t *sem = sem_open(name.c_str(), O_EXCL, 0644, 1);
	//sem_wait(sem_open(name.c_str(), O_CREAT, 0644, 1));
//	if (sem != NULL)
	//	sem_post(sem);
	//sem_post(sem_open(name.c_str(), O_CREAT, 0644, 1));
}

string ServerWorker::GetPasswFilePth(const string& username) {
	string pth = USERS_FOLDER;
	pth += username;
	pth += PASSW_FILE;
	return pth;
}

string ServerWorker::GetMessageFilePth(const string& username) {
	string pth = USERS_FOLDER;
	pth += username;
	pth += MESSAGE_FILE;
	return pth;
}

bool ServerWorker::checkUser(const string& name)
{
	string pth = GetPasswFilePth(name);
	string pass2;
	openSem(name);
	ifstream fin(pth.c_str());
	if (fin.good())
		fin >> pass2;
	fin.close();
	closeSem(name);
	return pass2.size()>0;
}

string ServerWorker::RegisterNewUser(const string &uname, const string &passw, bool &res) {
	int stat;
	string mes;
	res = false;
	if (uname.length() > 0 && passw.length() > 0) {//data.size() > 0) {
												   /*string uname, passw;
												   bool b = 0;
												   for (int i = 0; i < data.size(); i++) {
												   if (data[i] == '@')
												   b = 1;
												   else {
												   if (b) passw.push_back(data[i]);
												   else uname.push_back(data[i]);
												   }
												   }*/

		ifstream fin(GetPasswFilePth(uname).c_str());
		if (!fin.good()) {
			string pth = "./users/";
			_mkdir(pth.c_str());//, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			pth += uname;

			stat = _mkdir(pth.c_str());//, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			if (stat == 0) {
				printf("Dir %s created successfully.\n", uname.c_str());
				pth += PASSW_FILE;
				ofstream out(pth.c_str());
				if (out.good()) {
					out << passw;
					out.close();
					res = true;
					ofstream mFile(GetMessageFilePth(uname).c_str());
					mFile.close();
				}
				else {
					printf("ERROR: Password is not saved.\n");
					mes.append("Internal server issue. Please, try again.\n");
				}
			}
			else {
				printf("ERROR: Failed to create dir. ErrCode = %d\n", stat);
				mes.append("Internal server issue. Please, try again.\n");
			}
		}
		else {
			printf("ERROR: User already exists.\n");
			mes.append("Username is already used by another user. Please, choose other option for username.\n");
		}
	}
	return mes;
}

string ServerWorker::LoginNewUser(const string &uname, const string &passw, bool &res) {
	//int stat;
	string username = "";
	string mes;
	res = false;
	if (uname.length() > 0 && passw.length() > 0) {//if (data.size() > 0) {
												   /*string uname, passw, pass2;
												   bool b = 0;
												   for (int i = 0; i < data.size(); i++) {
												   if (data[i] == '@')
												   b = 1;
												   else {
												   if (b) passw.push_back(data[i]);
												   else uname.push_back(data[i]);
												   }
												   }*/
		string pass2;
		string pth = GetPasswFilePth(uname);
		openSem(uname);
		ifstream fin(pth.c_str());
		if (fin.good()) {
			fin >> pass2;
			if (pass2.compare(passw) == 0) {
				res = true;
				printf("Successfully logged in! User: %s\n", uname.c_str());
			}
			else {
				printf("ERROR: Password is not correct or there is no access to the pass. path = \"%s\"", pth.c_str());
				mes.append("Internal server issue. Please, try again.\n");
			}
		}
		else {
			printf("ERROR: Could not load file %s.\n", pth.c_str());
			mes.append("Internal server issue. Please, try again.\n");
		}
		fin.close();
		closeSem(uname);
	}
	return mes;
}

string ServerWorker::DeleteUser(const string& username) {
	string buf;
	time_t seconds = time(NULL);
	tm* timeinfo = localtime(&seconds);

	buf.append(asctime(timeinfo));
	int position = buf.find('\n');
	buf.replace(position, 1, "");
	position = buf.find(" ");
	while (position != string::npos) {
		buf.replace(position, 1, "_");
		position = buf.find(" ", position + 1);
	}
	buf.append("_");
	buf.append(username);
	string oldname = USERS_FOLDER;
	oldname.append(username);
	string newname = USERS_FOLDER;
	newname.append(buf);
	int res = rename(oldname.c_str(), newname.c_str());
	if (res != 0)
		return "An error occured while removing existing user! Please, try again later!";
	return "";
}

unsigned long ServerWorker::AddMessage(Message* message, const string& username, const string& from, string& err) {
	unsigned long lastId = LastMesID(username) + 1;
	bool isNameValid = false;
	isNameValid = checkUser(username);
	if (isNameValid)
	{
		if (lastId > 0 && message != NULL) {
			time_t seconds = time(NULL);
			tm* timeinfo = localtime(&seconds);
			message->date_time = asctime(timeinfo);
			message->id = lastId;
			message->state = MSTATE_UNREAD;
			message->username = from;
			message->len = message->body.length();
			WriteToFile(username, message);
		}
	}
	else return 0;
	return lastId;
}

void ServerWorker::WriteToFile(const string& username, Message* message)
{
	openSem(username);
	ofstream out(GetMessageFilePth(username).c_str(), ios_base::app);
	if (out.good() && message != NULL) {
		out << MES_ID << message->id << endl;
		out << MES_ADDR << message->username << endl;
		out << MES_DATE_TIME << message->date_time; // << endl;
		out << MES_LEN << message->len << endl;
		out << MES_STATE << MESSAGE_STATES[message->state] << endl;
		out << message->body << endl;

	}
	out.close();
	closeSem(username);
}

bool ServerWorker::WriteMessages(const string& username, Message** m, const unsigned long& size, bool ioMode)//const Message** m, const unsigned long& size)
{
	if (m != NULL && size >= 0)
	{
		ofstream out;
		if (ioMode) out.open(GetMessageFilePth(username).c_str(), ios_base::trunc);
		else out.open(GetMessageFilePth(username).c_str(), ios_base::app);
		if (out.good())
		{
			if (size > 0)
				for (unsigned long i = 0; i<size; i++)
				{
					if (m[i] != NULL)
					{
						out << MES_ID << m[i][0].id << endl;
						out << MES_ADDR << m[i][0].username << endl;
						out << MES_DATE_TIME << m[i][0].date_time << endl;
						out << MES_LEN << m[i][0].len << endl;
						out << MES_STATE << m[i][0].state << endl;
						out << m[i][0].body << endl;
					}
				}
		}
		else
		{
			out.close();
			return false;
		}
		out.close();
		return true;
	}
	return false;
}

string ServerWorker::ShowUnreadMes(const string& username, string& buf)
{
	buf.clear();
	Message** m;
	unsigned long size;
	int unrCount = 0;
	bool changes = false;
	m = ReadAllMes(username, size);
	if (size>0)
	{
		for (unsigned long i = 0; i<size; i++)
		{
			if (m[i] != NULL)
			{
				if (m[i][0].state == MSTATE_UNREAD)
				{
					buf.append(MessageToString(m[i][0]));
					m[i][0].state = MSTATE_NORMAL;
					unrCount++;
					changes = true;
				}
			}
		}
		if (changes) WriteMessages(username, m, size, true);
		for (unsigned long i = 0; i<size; i++)
		{
			if (m[i] != NULL)
			{
				delete m[i];
			}
		}
	}
	else
		return "No messages found!\n";

	if (unrCount == 0)
		return "No unread messages.";
	else
		return "";
}

string ServerWorker::ShowAllMes(const string& username, string& buf)
{
	buf.clear();
	Message** m;
	unsigned long size;
	m = ReadAllMes(username, size);
	bool changes = false;
	if (size>0)
	{
		for (unsigned long i = 0; i<size; i++)
		{
			if (m[i] != NULL)
			{
				buf.append(MessageToString(m[i][0]));
				if (m[i][0].state == MSTATE_UNREAD)
				{
					m[i][0].state = MSTATE_NORMAL;
					changes = true;
				}
			}
		}
		if (changes) WriteMessages(username, m, size, true);
		for (unsigned long i = 0; i<size; i++)
		{
			if (m[i] != NULL)
			{
				delete m[i];
			}
		}
	}
	else
		return "No messages found!\n";
	return "";
}

string ServerWorker::ShowExactMes(const string& username, string& buf, const string& mesNumber)
{
	string err;
	buf.clear();
	Message* m = NULL;
	bool res = false;
	unsigned long pos = strtoul(mesNumber.c_str(), NULL, 10);
	if (pos>0)
		m = ReadOneMes(username, pos, res);
	if (res && m != NULL)
		buf = MessageToString(*m);
	else
		err = "The message is not found.";
	if (m != NULL) delete m;
	return err;
}

string ServerWorker::DeleteMes(const string& username, const string& mesNumber)
{
	unsigned long num = strtoul(mesNumber.c_str(), NULL, 10);
	if (num>0)
		if (!DeleteOneMes(username, num))
			return "Internal error occured!";
	return "";
}

string ServerWorker::ResendMes(const string& from, const string& mesNumber, const string& to)
{
	unsigned long num = strtoul(mesNumber.c_str(), NULL, 10);
	if (num>0)
	{
		Message* m = NULL;
		bool res = false;
		m = ReadOneMes(from, num, res);
		if (res && m != NULL)
		{
			m->id = LastMesID(to) + 1;
			res = WriteMessages(to, &m, 1, false);
			if (m != NULL) delete m;
			if (!res) return "Couldn't open the file of destination user.";
		}
		else
			return "Couldn't find the message.";
	}
	return "";
}

string ServerWorker::MessageToString(const Message& m)
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
	res << m.state;
	res << "\n";

	res << "BODY: ";
	res << m.body;
	res << "\n";
	return res.str();
}

unsigned long ServerWorker::LastMesID(const string& username)
{
	//Don't add mutex here, blocking should be in the function, which calls this one above!!!
	unsigned long res = 0;
	unsigned long size = 0;
	Message ** buf = ReadAllMes(username, size);
	if (size>0 && buf != NULL)
	{
		for (unsigned long i = 0; i < size; i++)
			if (buf[i] != NULL)
			{
				if (res < buf[i][0].id)
					res = buf[i][0].id;
				delete buf[i];
			}
	}
	return res;
}

Message** ServerWorker::ReadAllMes(const string& username, unsigned long& res)
{
	Message** mes = NULL;
	res = 0;
	string buf;
	long size = 0;
	ifstream inp(GetMessageFilePth(username).c_str());
	int state = 0;
	if (inp.good()) {
		while (!inp.eof()) {
			buf.clear();
			std::getline(inp, buf);
			if (buf.size()>0)
				//printf("read: %s\n", buf);
				switch (state)
				{
				case 0: // id
					res++;
					mes = (Message**)realloc(mes, res * sizeof(Message*));
					mes[res - 1] = new Message();
					buf.replace(0, string(MES_ID).size(), "");
					mes[res - 1][0].id = strtoul(buf.c_str(), NULL, 10);
					printf("ID = %d\n", mes[res - 1][0].id);
					state++;
					break;
				case 1: // user name
					buf.replace(0, string(MES_ADDR).size(), "");
					mes[res - 1][0].username = buf;
					printf("USERNAME = %s\n", mes[res - 1][0].username.c_str());
					state++;
					break;
				case 2: // date_time
					buf.replace(0, string(MES_DATE_TIME).size(), "");
					mes[res - 1][0].date_time.append(buf);
					printf("DATE = %s\n", mes[res - 1][0].date_time.c_str());
					state++;
					break;
				case 3: // len
					buf.replace(0, string(MES_LEN).size(), "");
					mes[res - 1][0].len = strtoul(buf.c_str(), NULL, 10);
					size = mes[res - 1][0].len;
					printf("SIZE = %d\n", mes[res - 1][0].len);
					state++;
					break;
				case 4: // state
					buf.replace(0, string(MES_STATE).size(), "");
					mes[res - 1][0].state = strtoul(buf.c_str(), NULL, 10);
					printf("STATE = %d\n", mes[res - 1][0].state);
					state++;
					break;
				case 5: // body
					mes[res - 1][0].body += buf;
					mes[res - 1][0].body += "\n";
					size -= buf.size() + 1;
					if (size <= 0)
					{
						state = 0;
						printf("BODY = %s\n", mes[res - 1][0].body.c_str());
					}
					break;
				}
		}
	}
	inp.close();

	return mes;
}

Message* ServerWorker::ReadOneMes(const string& username, const unsigned long& id, bool& res)
{
	Message* mes = NULL;
	res = false;
	string buf;
	long size = 0;
	ifstream inp(GetMessageFilePth(username).c_str());
	int state = 0;
	if (inp.good()) {
		while (!inp.eof()) {
			buf.clear();
			std::getline(inp, buf);
			if (buf.size()>0)
				switch (state)
				{
				case 0: // id
					if (mes != NULL)
						if (mes->id == id)
						{
							res = true;
							return mes;
						}
					delete mes;
					mes = new Message();
					buf.replace(0, string(MES_ID).size(), "");
					mes->id = strtoul(buf.c_str(), NULL, 10);
					printf("ID = %d\n", mes->id);
					state++;
					break;
				case 1: // user name
					buf.replace(0, string(MES_ADDR).size(), "");
					mes->username = buf;
					printf("USERNAME = %s\n", mes->username.c_str());
					state++;
					break;
				case 2: // date_time
					buf.replace(0, string(MES_DATE_TIME).size(), "");
					mes->date_time = buf;
					printf("DATE = %s\n", mes->date_time.c_str());
					state++;
					break;
				case 3: // len
					buf.replace(0, string(MES_LEN).size(), "");
					mes->len = strtoul(buf.c_str(), NULL, 10);
					size = mes->len;
					printf("SIZE = %d\n", mes->len);
					state++;
					break;
				case 4: // state
					buf.replace(0, string(MES_STATE).size(), "");
					mes->state = strtoul(buf.c_str(), NULL, 10);
					printf("STATE = %d\n", mes->state);
					state++;
					break;
				case 5: // body
					mes->body += buf;
					mes->body += "\n";
					size -= buf.size() + 1;
					if (size <= 0)
					{
						state = 0;
						printf("BODY = %s\n", mes->body.c_str());
					}
					break;
				}
		}
	}
	inp.close();
	if (mes != NULL)
	{
		if (mes->id == id)
		{
			res = true;
			return mes;
		}
		else
			delete mes;
	}
	return NULL;
}

bool ServerWorker::DeleteOneMes(const string& username, const unsigned long& id)
{
	bool res = false;
	unsigned long size = 0;
	Message ** buf = ReadAllMes(username, size);
	if (size>0 && buf != NULL)
	{
		for (unsigned long i = 0; i < size; i++)
			if (buf[i] != NULL)
				if (buf[i][0].id == id)
				{
					for (unsigned long t = i; t<size - 1; t++)
					{
						buf[t] = buf[t + 1];
					}
					res = true;
					break;
				}
		if (res == true)
		{
			size--;
			if (size > 0)
			{
				buf = (Message **)realloc(buf, (size) * sizeof(Message*));
				if (buf == NULL)
					res = false;
			}
		}
	}
	if (res)
		res = WriteMessages(username, buf, size, true);
	for (unsigned long i = 0; i<size; i++)
	{
		if (buf[i] != NULL)
		{
			delete buf[i];
		}
	}
	return res;
}

bool ServerWorker::ListenRecv(std::string& MsgStr)
{
	if (td != nullptr)
	{
		/*HANDLE m = CreateMutex(NULL, FALSE, td->rMutexName.c_str());
		DWORD result;
		result = WaitForSingleObject(m, INFINITE);
		if (result != WAIT_OBJECT_0)
		{
			cout << "Failed to lock mutex!" << endl;
			return false;
		}*/
		MsgStr.clear();
		//size_t len = 10;
		while (MsgStr.length() == 0)//< TECH_DG_SIZE)
		{
			HANDLE m;
			if (LockMutex(td->rMutexName, m) && td->rBuf != nullptr)
			{
				MsgStr += *td->rBuf;
				td->rBuf->clear();
				// parse len
				/*if (MsgStr.length() >= len)
				{
					string c = MsgStr.substr(0, len);
					MsgStr = MsgStr.substr(len, MsgStr.length() - 1);
					len = atoi(c.c_str());
				}*/
			}
			UnlockMutex(m);
			Sleep(100);
		}
		return true;
		/*char c[10];
		unsigned int size = 0;
		int res = recv(socket, c, 10, 0);
		if (res == 10)
		{
			size = atoi(c);
			char* recvbuf = new char[size];
			int res = recv(socket, recvbuf, size, 0);
			printf("String received: %s\n", recvbuf);
			printf("Received %d symbols!\n", res);
			if (res > 0)
			{
				MsgStr.clear();
				for (int i = 0; i < res; i++)
					if (recvbuf[i] != '\n' && recvbuf[i] != '\r' && recvbuf[i] != '\t' && recvbuf[i] != '\0')
						MsgStr.push_back(recvbuf[i]);
			}
		}
		else return false;*/
	}
	else
		return false;
}

void ServerWorker::sendTo(const string& message) {
	//int res = 0;
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
	//printf("String to send: %s\n", s.c_str());
	///res = send(socket, s.c_str(), s.size(), 0);
	//if (res != s.size())
		//printf("Send failed: %d != %d!\n", s.c_str(), s.size());*/
	if (td != nullptr)
	{
		/*HANDLE m = CreateMutex(NULL, FALSE, td->sMutexName.c_str());
		DWORD result;
		result = WaitForSingleObject(m, INFINITE);*
		if (result != WAIT_OBJECT_0
		{
			cout << "Failed to lock mutex!" << endl;
			
		}else*/
		HANDLE m;
		bool act = false;
		while (!act)
		{
			if (LockMutex(td->sMutexName, m) && td->sBuf != nullptr)
			{
				//*td->sBuf = message;
				*td->sBuf = s;
				act = true;
			}
			UnlockMutex(m);
		}
	}
}


string ServerWorker::serialize(STATE opcode, unsigned short numarg, const string * ss)
{
	stringstream sstr;
	sstr << API[opcode] << DELIM_PARSE << numarg << DELIM_PARSE;
	if (numarg > 0 && ss != NULL)
		for (int i = 0; i <= numarg - 1; i++)
		{
			string temp = ss[i];
			std::replace(temp.begin(), temp.end(), DELIM_PARSE, ' ');
			sstr << temp << DELIM_PARSE;
		}
	return sstr.str();
}

STATE ServerWorker::parse(const string& input, unsigned short& numarg, string* &args)
{
	STATE res = NO_OPERATION;
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

STATE ServerWorker::parseOpCode(const string& buf)
{
	STATE res = NO_OPERATION;

	if (buf.compare(API[0]) == 0)
		return SERV_OK;
	else if (buf.compare(API[1]) == 0)
		return NO_OPERATION;
	else if (buf.compare(API[2]) == 0)
		return ANSWER;
	else if (buf.compare(API[3]) == 0)
		return START;
	else if (buf.compare(API[4]) == 0)
		return INIT;
	else if (buf.compare(API[5]) == 0)
		return OPCODE;
	else if (buf.compare(API[6]) == 0)
		return EXIT;
	else if (buf.compare(API[7]) == 0)
		return REG;
	else if (buf.compare(API[8]) == 0)
		return LOG;
	else if (buf.compare(API[9]) == 0)
		return LUG;
	else if (buf.compare(API[10]) == 0)
		return SND;
	else if (buf.compare(API[11]) == 0)
		return DEL_US;
	else if (buf.compare(API[12]) == 0)
		return DEL_MES;
	else if (buf.compare(API[13]) == 0)
		return SH_UNR;
	else if (buf.compare(API[14]) == 0)
		return SH_ALL;
	else if (buf.compare(API[15]) == 0)
		return SH_EX;
	else if (buf.compare(API[16]) == 0)
		return RSND;
	else if (buf.compare(API[17]) == 0)
		return INSYS;

	return res;
}

void ServerWorker::closeSocket()
{
	/*sendTo("Closing connection.");
	if (shutdown(socket, SD_BOTH) < 0)
		printf("Socket #%d shutdown failed\n", socket);
	if (closesocket(socket) < 0)
		printf("Socket #%d close failed\n", socket);*/
	//WSACleanup();
	// TODO:
}

bool ServerWorker::LockMutex(HANDLE& m)
{
	m = CreateMutex(NULL, FALSE, L"UdpServer");
	DWORD result;
	result = WaitForSingleObject(m, 100);
	if (result != WAIT_OBJECT_0)
	{
		cout << "Failed to lock mutex!" << endl;
		return false;
	}
	return true;
}

void ServerWorker::UnlockMutex(HANDLE& m)
{
	CloseHandle(m);
}

bool ServerWorker::LockMutex(const wstring& name, HANDLE& m)
{
	m = CreateMutex(NULL, FALSE, name.c_str());
	DWORD result;
	result = WaitForSingleObject(m, 100);
	if (result != WAIT_OBJECT_0)
	{
		cout << "Failed to lock mutex!" << endl;
		return false;
	}
	return true;
}