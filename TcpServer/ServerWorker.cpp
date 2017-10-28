/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "ServerWorker.h"

ServerWorker::ServerWorker() {
}

ServerWorker::~ServerWorker()
{
    closeSocket();
}

void ServerWorker::init(SOCKET s) {
    socket = s;
}

bool ServerWorker::mainLoop() {
    int State = 1;
    string MsgStr;
    string MessageBuf;
    string currentUserName;
    string errMessage;
    int mesId;
    
    string sendToUserName;
    bool isNameValid;
    bool isSent;

    string buf;
    bool RegisterState, LoginState;
    while (true) {
        switch (State) {
                // Main menu
            case 1: 
                sendTo("* MAIL *\n");
                sendTo("Select the following items:\n");
                if (currentUserName.size() > 0) sendTo("1 - Send message\n");
                sendTo("2 - Exit\n");
                if (currentUserName.size() == 0) sendTo("3 - Register\n");
                if (currentUserName.size() > 0) sendTo("4 - Logout\n");
                else sendTo("4 - Login\n");
                if (currentUserName.size() > 0) sendTo("5 - Delete user\n");
                if (currentUserName.size() > 0) sendTo("6 - Show unread messages\n");
                if (currentUserName.size() > 0) sendTo("7 - Show all messages\n");
                if (currentUserName.size() > 0) sendTo("8 - Show the exact message\n");
                if (currentUserName.size() > 0) sendTo("9 - Delete message\n");
                if (currentUserName.size() > 0) sendTo("10 - Resend message\n");
                sendTo("Enter your option: ");
                break;

            case 2:
                sendTo("Enter, whom you would like to send: ");
                State = 2;
                break;

            case 3: printf("Client with ID: %d is disconnect!\n", socket);
                closeSocket();
                return true;
                break;

            case 4:
                if (mesId != 0)
                {
                    buf = "Your message id = ";
                    buf += mesId;
                    buf += "\n";
                    sendTo(buf.c_str());
                }
                else
                    sendTo("An internal error occured while appending your message into base! Please, try again later!\n");
                State =4;
                break;

            case 5: 
                sendTo("Wrong input. Press any key.");
                State = 4;
                break;
            case 6: 
                sendTo("You are about to sign up. Enter the username consisting of <username>@<password>\n");
                sendTo("Enter your option: ");
                State = 5;
                break;
            case 7:
                if (RegisterState == true) 
                    sendTo("Registered successfully. Press any key.");
                else 
                {
                    string buf = "Register failed. ERROR -> ";
                    buf.append(errMessage.c_str());
                    buf.append("Press any key.");
                    sendTo(buf.c_str());
                }
                State = 4;
                break;
            case 8:
                if (currentUserName.size() == 0) 
                {
                    sendTo("You are about to log in. Enter the username consisting of <username>@<password>\n");
                    sendTo("Enter your option: ");
                    State = 6;
                } 
                else 
                {
                    sendTo("Log out successfully.\n");
                    currentUserName = "";
                    State = 4;
                }
                break;
            case 9:
                if (LoginState == true) 
                {
                    string buf = "Login successfully. User -> ";
                    buf.append(currentUserName);
                    buf.append(". \nPress any key.");
                    sendTo(buf.c_str());
                } 
                else 
                {
                    string buf = "Login failed. ERROR -> ";
                    buf.append(errMessage);
                    buf.append("Press any key.");
                    sendTo(buf.c_str());
                }
                State = 4;
                break;
            case 10:
                if (currentUserName.size() > 0) 
                {
                    sendTo("Really delete user? ('Y' - yes, <all other> - no): ");
                    State = 7;
                } 
                else 
                    sendTo("You should be logged in to delete user.\n");
                break;
            case 11:
                if (errMessage.size() == 0) 
                {
                    sendTo("Account deleted successfully. Press any key.\n");
                    State = 4;
                } 
                else 
                {
                    string buf = "ERROR -> ";
                    buf.append(errMessage);
                    sendTo(buf.c_str());
                    sendTo("\nPress any key.");
                    State = 4;
                }
                break;
            case 12:
                sendTo("You wanted to see the list of unread messages\n");
                sendTo("\nPress any key.\n");
                State = 8;
                break;
            case 13:
                if (errMessage.size() == 0) 
                {
                    sendTo("Here is a list of unread messages:\n");
                    sendTo(buf);
                    sendTo("\nPress any key.\n");
                    State = 4;
                } 
                else 
                {
                    string buf = "ERROR -> ";
                    buf.append(errMessage);
                    sendTo(buf.c_str());
                    //sendTo("\nPress any key.\n");
                    State = 4;
                }
                break;
            case 14:
                sendTo("You wanted to see the list of all messages\n");
                sendTo("Press any key.\n");
                State = 9;
                break;
            case 15:
                if (errMessage.size() == 0) 
                {
                    sendTo("Here is a list of all messages:\n");
                    sendTo(buf);
                    sendTo("\nPress any key.\n");
                    State = 4;
                } 
                else 
                {
                    string buf = "ERROR -> ";
                    buf.append(errMessage);
                    sendTo(buf.c_str());
                    sendTo("\nPress any key.");
                    State = 4;
                }
                break;
            case 16:
                sendTo("You wanted to see the exact message. Please, enter the number of your message: \n");
                State = 10;
                break;
            case 17:
                if (errMessage.size() == 0) 
                {
                    sendTo("Here is your message:\n");
                    sendTo(buf);
                    sendTo("\nPress any key.");
                    State = 4;
                } 
                else 
                {
                    string buf = "ERROR -> ";
                    buf.append(errMessage);
                    sendTo(buf.c_str());
                    sendTo("\nPress any key.");
                    State = 4;
                }
                break;
            case 18:
                sendTo("You wanted to delete the exact message. Please, enter the number of your message: \n");
                State = 11;
                break;
            case 19:
                if (errMessage.size() == 0) 
                {
                    sendTo("Message deleted successfully. Press any key.\n");
                    State = 4;
                } 
                else 
                {
                    string buf = "ERROR -> ";
                    buf.append(errMessage);
                    sendTo(buf.c_str());
                    sendTo("\nPress any key.");
                    State = 4;
                }
                break;    
            case 20:
                sendTo("You wanted to resend the exact message. Enter the destination username:\n"); //Please, enter the message number and destination-user in format <number>@<user>:\n");
                State = 12;
                break;
            case 21:
                if (errMessage.size() == 0) 
                {
                    sendTo("Message resent successfully. Press any key.\n");
                    State = 4;
                } 
                else 
                {
                    string buf = "ERROR -> ";
                    buf.append(errMessage);
                    sendTo(buf.c_str());
                    sendTo("\nPress any key.");
                    State = 4;
                }
                break;  
            case 22:
                if (isNameValid) 
                {
                    sendTo("Good. Now enter the message text: \n");
                    State = 13;
                }
                else 
                {
                    sendTo("Username is not valid. Please, try again.\n");
                    State = 4;
                }
                break;
            case 23:
                if (isSent) 
                {
                    sendTo("The message is successfully sent to user -> ");
                    string buf = sendToUserName;
                    sendTo(buf.c_str());
                    sendTo("\nPress any key.");
                    State = 4;
                }
                else 
                {
                    sendTo("The message is not sent.\n");
                    State = 4;
                }
                break;
            case 24:
                if (isNameValid) 
                {
                    sendTo("Enter the message to send: ");
                    State = 13;
                }
                else 
                {
                    sendTo("The destination username is not valid. Please, try again.");
                    State = 4;
                }
                break;
            case 25:
                if (isNameValid) 
                {
                    sendTo("Enter the number of message to resend: ");
                    State = 14;
                }
                else 
                {
                    sendTo("The destination username is not valid. Please, try again.");
                    State = 4;
                }
                break;
        }

        if (State != 8) if (!ListenRecv(MsgStr)) return false;

        switch (State) {
            case 1: 
                if (strcmp((char*) MsgStr.c_str(), "1") == 0) 
                {
                    if (currentUserName.size() > 0) State = 2;
                    else State = 5;
                } 
                else if (strcmp((char*) MsgStr.c_str(), "2") == 0) State = 3;
                else if (strcmp((char*) MsgStr.c_str(), "3") == 0) 
                {
                    if (currentUserName.size() == 0) State = 6;
                    else State = 5;
                } 
                else if (strcmp((char*) MsgStr.c_str(), "4") == 0) State = 8;
                else if (strcmp((char*) MsgStr.c_str(), "5") == 0) 
                {
                    if (currentUserName.size() > 0) State = 10;
                    else State = 5;
                } 
                else if (strcmp((char*) MsgStr.c_str(), "6") == 0) 
                {
                    if (currentUserName.size() > 0) State = 12;
                    else State = 5;
                } 
                else if (strcmp((char*) MsgStr.c_str(), "7") == 0) 
                {
                    if (currentUserName.size() > 0) State = 14;
                    else State = 5;
                } 
                else if (strcmp((char*) MsgStr.c_str(), "8") == 0) 
                {
                    if (currentUserName.size() > 0) State = 16;
                    else State = 5;
                } 
                else if (strcmp((char*) MsgStr.c_str(), "9") == 0) 
                {
                    if (currentUserName.size() > 0) State = 18;
                    else State = 5;
                } 
                else if (strcmp((char*) MsgStr.c_str(), "10") == 0) 
                {
                    if (currentUserName.size() > 0) State = 20;
                    else State = 5;
                } 
                else State = 5;
                break;

            case 2:
                isNameValid = checkUser(MsgStr);
                if (isNameValid) sendToUserName = MsgStr;
                else sendToUserName = "";
                State = 24;
                break;

            case 3:
                break;

            case 4:
                State = 1;
                break;
            case 5:
                errMessage = RegisterNewUser(MsgStr, RegisterState);
                State = 7;
                break;
            case 6:
                errMessage = LoginNewUser(MsgStr, LoginState, currentUserName);
                State = 9;
                break;
            case 7:
                if (strcmp((char*) MsgStr.c_str(), "Y") == 0) {
                    errMessage = DeleteUser(currentUserName);
                    currentUserName = "";
                    State = 11;
                } else
                    State = 1;
                break;
            case 8:
                errMessage = ShowUnreadMes(currentUserName, buf);
                State = 13;
                break;
            case 9:
                errMessage = ShowAllMes(currentUserName, buf);
                State = 15;
                break;
            case 10:
                errMessage = ShowExactMes(currentUserName, buf, MsgStr);
                State = 17;
                break;
            case 11:
                errMessage = DeleteMes(currentUserName, MsgStr);
                State = 19;
                break;
            case 12:
                isNameValid = checkUser(MsgStr);
                if (isNameValid) sendToUserName = MsgStr;
                else sendToUserName = "";
                State = 25;
                break;
            case 13:
                isSent = true;
                MessageBuf = MsgStr;
                if (currentUserName.compare(sendToUserName) != 0) 
                {
                    mesId = AddMessage(MsgStr, currentUserName, MESSAGE_STATES[MSTATE_NORMAL], currentUserName);
                    if (mesId == 0) 
                        isSent = false;
                }
                mesId = AddMessage(MsgStr, sendToUserName, MESSAGE_STATES[MSTATE_UNREAD], currentUserName);
                if (mesId == 0) 
                    isSent = false;
                State = 23;
                break;
            case 14:
                errMessage = ResendMes(currentUserName, MsgStr, sendToUserName);
                State = 21;
                break;
        }
    }
    return true;
}

void ServerWorker::openSem(const string& name)
{
    //sem_wait(sem_open(name.c_str(), O_CREAT, 0644, 1));
}

void ServerWorker::closeSem(const string& name)
{
    //sem_post(sem_open(name.c_str(), O_CREAT, 0644, 1));
}

void ServerWorker::sendTo(const string& message) {
    int res = 0;
    res = send(socket, message.c_str(), message.size() + 1, 0);
    if (res != message.size() + 1)
        printf("Send failed: %d != %d!\n", res, message.size());
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

string ServerWorker::RegisterNewUser(const string &data, bool &res) {
    int stat;
    string mes;
    res = false;
    if (data.size() > 0) {
        string uname, passw;
        bool b = 0;
        for (int i = 0; i < data.size(); i++) {
            if (data[i] == '@')
                b = 1;
            else {
                if (b) passw.push_back(data[i]);
                else uname.push_back(data[i]);
            }
        }

        ifstream fin(GetPasswFilePth(uname).c_str());
        if (!fin.good()) {
            string pth = "./users/";
            pth += uname;

            stat = mkdir(pth.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
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
                } else {
                    printf("ERROR: Password is not saved.\n");
                    mes.append("Internal server issue. Please, try again.\n");
                }
            } else {
                printf("ERROR: Failed to create dir. ErrCode = %d\n", stat);
                mes.append("Internal server issue. Please, try again.\n");
            }
        } else {
            printf("ERROR: User already exists.\n");
            mes.append("Username is already used by another user. Please, choose other option for username.\n");
        }
    }
    return mes;
}

string ServerWorker::LoginNewUser(const string &data, bool &res, string& username) {
    //int stat;
    username = "";
    string mes;
    res = false;
    if (data.size() > 0) {
        string uname, passw, pass2;
        bool b = 0;
        for (int i = 0; i < data.size(); i++) {
            if (data[i] == '@')
                b = 1;
            else {
                if (b) passw.push_back(data[i]);
                else uname.push_back(data[i]);
            }
        }
        string pth = GetPasswFilePth(uname);
        openSem(uname);
        ifstream fin(pth.c_str());
        if (fin.good()) {
            fin >> pass2;
            if (pass2.compare(passw) == 0) {
                res = true;
                username = uname;
                printf("Successfully logged in! User: %s\n", uname.c_str());
            } else {
                printf("ERROR: Password is not correct or there is no access to the pass. path = \"%s\"", pth.c_str());
                mes.append("Internal server issue. Please, try again.\n");
            }
        } else {
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

unsigned long ServerWorker::AddMessage(const string& message, const string& username, const int &state, const string& from) {
    string buf;
    unsigned long lastId = LastMesID(username) + 1;
    if (lastId > 0) {
        ofstream out(GetMessageFilePth(username).c_str(), ios_base::app);
        if (out.good()) {
            time_t seconds = time(NULL);
            tm* timeinfo = localtime(&seconds);

            out << MES_ID << lastId << endl;
            out << MES_ADDR << from << endl;
            out << MES_DATE_TIME << asctime(timeinfo); // << endl;
            out << MES_LEN << message.size() << endl;
            out << MES_STATE << MESSAGE_STATES[state] << endl;
            out << message << endl;
        }
        else
            lastId = 0;
        out.close();
    }

    return lastId;
}

bool ServerWorker::WriteMessages(const string& username, Message** m, const unsigned long& size, bool ioMode)//const Message** m, const unsigned long& size)
{
    if (m!=NULL && size>=0)
    {
        ofstream out;
        if (ioMode) out.open(GetMessageFilePth(username).c_str(), ios_base::trunc);
        else out.open(GetMessageFilePth(username).c_str(), ios_base::app);
        if (out.good())
        {
            if (size > 0) 
                for (unsigned long i =0; i<size; i++)
                {
                    if (m[i]!=NULL)
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
        for (unsigned long i=0; i<size; i++)
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
        for (unsigned long i=0; i<size; i++)
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
        for (unsigned long i=0; i<size; i++)
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
        for (unsigned long i=0; i<size; i++)
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
    if (res && m!=NULL) 
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
    if (size>0 && buf!=NULL)
    {
        for (unsigned long i = 0; i < size; i++)
            if (buf[i]!=NULL)
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
                    mes = (Message**)realloc(mes, res*sizeof(Message*));
                    mes[res-1] = new Message();
                    buf.replace(0,string(MES_ID).size(), "");
                    mes[res-1][0].id = strtoul(buf.c_str(), NULL, 10);
                    printf("ID = %d\n", mes[res-1][0].id);
                    state++;
                    break;
                case 1: // user name
                    buf.replace(0,string(MES_ADDR).size(), "");
                    mes[res-1][0].username = buf;
                    printf("USERNAME = %s\n", mes[res-1][0].username.c_str());
                    state++;
                    break;
                case 2: // date_time
                    buf.replace(0,string(MES_DATE_TIME).size(), "");
                    mes[res-1][0].date_time.append(buf);
                    printf("DATE = %s\n", mes[res-1][0].date_time.c_str());
                    state++;
                    break;
                case 3: // len
                    buf.replace(0,string(MES_LEN).size(), "");
                    mes[res-1][0].len = strtoul(buf.c_str(), NULL, 10);
                    size = mes[res-1][0].len;
                    printf("SIZE = %d\n", mes[res-1][0].len);
                    state++;
                    break;
                case 4: // state
                    buf.replace(0,string(MES_STATE).size(), "");
                    mes[res-1][0].state = strtoul(buf.c_str(), NULL, 10);
                    printf("STATE = %d\n", mes[res-1][0].state);
                    state++;
                    break;
                case 5: // body
                    mes[res-1][0].body += buf;
                    mes[res-1][0].body += "\n";
                    size -= buf.size()+1;
                    if (size<=0)
                    {
                        state = 0;
                        printf("BODY = %s\n", mes[res-1][0].body.c_str());
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
                    if (mes!=NULL)
                        if (mes->id == id)
                        {
                            res = true;
                            return mes;
                        }
                    delete mes;
                    mes = new Message();
                    buf.replace(0,string(MES_ID).size(), "");
                    mes->id = strtoul(buf.c_str(), NULL, 10);
                    printf("ID = %d\n", mes->id);
                    state++;
                    break;
                case 1: // user name
                    buf.replace(0,string(MES_ADDR).size(), "");
                    mes->username = buf;
                    printf("USERNAME = %s\n", mes->username.c_str());
                    state++;
                    break;
                case 2: // date_time
                    buf.replace(0,string(MES_DATE_TIME).size(), "");
                    mes->date_time = buf;
                    printf("DATE = %s\n", mes->date_time.c_str());
                    state++;
                    break;
                case 3: // len
                    buf.replace(0,string(MES_LEN).size(), "");
                    mes->len = strtoul(buf.c_str(), NULL, 10);
                    size = mes->len;
                    printf("SIZE = %d\n", mes->len);
                    state++;
                    break;
                case 4: // state
                    buf.replace(0,string(MES_STATE).size(), "");
                    mes->state = strtoul(buf.c_str(), NULL, 10);
                    printf("STATE = %d\n", mes->state);
                    state++;
                    break;
                case 5: // body
                    mes->body += buf;
                    mes->body += "\n";
                    size -= buf.size()+1;
                    if (size<=0)
                    {
                        state = 0;
                        printf("BODY = %s\n", mes->body.c_str());
                    }
                    break;
            }
        }
    }
    inp.close();
    if (mes!=NULL)
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
    if (size>0 && buf!=NULL)
    {
        for (unsigned long i = 0; i < size; i++)
            if (buf[i]!=NULL)
                if (buf[i][0].id == id)
                {
                    for (unsigned long t = i; t<size-1; t++)
                    {
                        buf[t] = buf[t+1];
                    }
                    res = true;
                    break;
                }
        if (res == true)
        {
            size--;
            if (size > 0) 
            {
                buf = (Message **) realloc(buf, (size)*sizeof(Message*));
                if (buf == NULL)
                    res = false;
            }
        }
    }
    if (res)
        res = WriteMessages(username, buf, size, true);
    for (unsigned long i=0; i<size; i++)
        {
            if (buf[i] != NULL)
            {
                delete buf[i];
            }
        }
    return res;
}

bool ServerWorker::ListenRecv(std::string& MsgStr) {
    char recvbuf[8192];

    int res = recv(socket, recvbuf, 8192, 0);

    printf("Received %d symbols!\n", res);
    if (res > 0) {
        MsgStr.clear();
        for (int i = 0; i < res; i++)
            if (recvbuf[i] != '\n' && recvbuf[i] != '\r' && recvbuf[i] != '\t' && recvbuf[i] != '\0')
                MsgStr.push_back(recvbuf[i]);
    }
    return true;
};

void ServerWorker::closeSocket()
{
    sendTo("Closing connection.");
    if (shutdown(socket, SHUT_RDWR) == -1)
        printf("Socket #%d shutdown failed\n", socket);
    if (close(socket) == -1)
        printf("Socket #%d close failed\n", socket);
}