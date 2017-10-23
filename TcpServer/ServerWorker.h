/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SeverWorker.h
 * Author: root
 *
 * Created on October 22, 2017, 3:13 AM
 */
#pragma once

#ifndef SERVERWORKER_H
#define SERVERWORKER_H

#include "stdinc.h"

#define USERS_FOLDER "./users/"
#define MESSAGE_FILE "/messages"
#define PASSW_FILE   "/password"

#define MES_ID          "<id>"
#define MES_ADDR        "<from>"
#define MES_DATE_TIME   "<date/time>"
#define MES_LEN         "<len>"
#define MES_STATE       "<state>"             // message read/unread/deleted

const int MESSAGE_STATES[] = {0, 1, 2 };

#define MSTATE_NORMAL 0     // position of normal value
#define MSTATE_DELETED 1    // position of deleted value
#define MSTATE_UNREAD 2     // position of unread value

typedef unsigned short  USHORT;
typedef int             SOCKET;

using namespace std;

class Message
{
public:
    unsigned long id;
    string username;
    string date_time;
    unsigned long len;
    int state;
    string body;
};

class ServerWorker
{
public:
    ServerWorker();
    ~ServerWorker();
    
    void init(SOCKET ClientSocket);
    bool mainLoop();
private:
    void sendTo(const string& message);
    SOCKET socket;
    string GetPasswFilePth(const string& username);
    string GetMessageFilePth(const string& username);
    string LoginNewUser(const string &data, bool &res, string& username);
    string RegisterNewUser(const string &data, bool &res);
    string DeleteUser(const string& username);
    unsigned long AddMessage(const string& message, const string& username, const int &state);
    string ShowUnreadMes(const string& username, string& buf);
    string ShowAllMes(const string& username, string& buf);
    string ShowExactMes(const string& username, string& buf, const string& mesNumber);
    string DeleteMes(const string& username, const string& mesNumber);
    string ResendMes(const string& username, const string& mesNumber);
    string MessageToString(const Message& m);
    Message** ReadAllMes(const string& username, unsigned long& size);
    Message* ReadOneMes(const string& username, const unsigned long& id, bool& res);
    bool DeleteOneMes(const string& username, const unsigned long& id);
    bool WriteMessages(const string& username, Message** m, const unsigned long& size);//const Message** m, const unsigned long& size);
    bool checkUser(const string& name);
    void openSem(const string& name);
    void closeSem(const string& name);
    
    bool ListenRecv(std::string& MsgStr);
    void closeSocket();
};


#endif /* SEVERWORKER_H */

