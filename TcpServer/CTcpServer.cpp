#include "stdinc.h"
#include "CTcpServer.h"

CTcpServer::CTcpServer() {
    AcceptSock = 0;
    LastClientID = 0;
    pthread_mutex_init(&Mut, 0);
    char* tmp = (char*) calloc(50, sizeof (char));
    FILE* fusers = fopen("./reg/users.txt", "r");
    while (!feof(fusers)) {
        tmp = (char*) calloc(50, sizeof (char));
        fgets(tmp, 50, fusers);
        tmp = removeLastSymbol(tmp);
        tmp = removeLastSymbol(tmp);
        if (!strcmp(tmp, "") == 0) {
            memmove(tmp, tmp, strlen(tmp));
            users.push_back(tmp);
        }
    }
    fclose(fusers);
}

char* CTcpServer::removeLastSymbol(char* str) {
    int len = strlen(str);
    char* str2 = (char*) calloc(50, sizeof (char));
    if (len > 0) {
        memmove(str2, str, len - 1);
    }
    return str2;
}

CTcpServer::CTcpServer(const CTcpServer& orig) {
}

CTcpServer::~CTcpServer() {
    pthread_mutex_destroy(&Mut);
}

void CTcpServer::StartAccept(USHORT Port) // Функция создания потока для приема подкл.
{
    StopAccept();
    LockMutex(&Mut);

    AcceptThInfo.ThPort = Port;
    AcceptThInfo.bCreated = true;

    AcceptThInput* ThInput = new AcceptThInput;
    ThInput->ThPort = Port;
    ThInput->pAcceptSock = &AcceptSock;
    ThInput->pParent = this;

    pthread_create(&AcceptThInfo.ThHandle, 0, AcceptThread, (void *) ThInput); //создание потока для приема

    UnlockMutex(&Mut);
}

void CTcpServer::StopAccept(bool bWaitForThreadClosed) //Функция закрытия потока подкл.
{
    LockMutex(&Mut);

    if (AcceptThInfo.bCreated == true) {
        if (AcceptSock) {
            if (bWaitForThreadClosed) {
                if (shutdown(AcceptSock, SHUT_RDWR) == -1)
                    printf("Socket shutdown failed\r\n");
                if (close(AcceptSock) == -1)
                    printf("Socket close failed\r\n");

                //pthread_cancel(AcceptThInfo.ThHandle);
                pthread_join(AcceptThInfo.ThHandle, NULL);
            }
            AcceptSock = 0;
        }
    }
    AcceptThInfo.ThHandle = 0;
    AcceptThInfo.ThPort = 0;
    AcceptThInfo.bCreated = false;
    UnlockMutex(&Mut);
}

void CTcpServer::StartListenTh(SOCKET Sock) //Функция создания потока работы с клиентом
{
    LockMutex(&Mut);
    ClientInfo CliInfo;
    CliInfo.ID = LastClientID++;
    CliInfo.ClientSocket = Sock;

    ListenThInput* pThInput = new ListenThInput;
    pThInput->CliID = CliInfo.ID;
    pThInput->ClientSocket = Sock;
    pThInput->pParent = this;

    pthread_create(&(CliInfo.ClientThreadInfo.ThHandle), NULL, ListenThread, pThInput);

    ClientsMap.insert(ClientsPair(CliInfo.ID, CliInfo));
    UnlockMutex(&Mut);
}

void CTcpServer::StopListenTh(ClientID ID, bool bWaitForThreadClosed) //Функция закрытия потока работы с клиентом
{
    LockMutex(&Mut);

    ClientsMapArrayIt it = ClientsMap.find(ID);

    if (it != ClientsMap.end()) {
        if (bWaitForThreadClosed) {
            ClientInfo Info = (*it).second;
            if (shutdown(Info.ClientSocket, SHUT_RDWR) == -1)
                printf("Socket shutdown failed\r\n");
            if (close(Info.ClientSocket) == -1)
                printf("Socket close failed\r\n");

            //pthread_cancel(Info.ClientThreadInfo.ThHandle);
            pthread_join(Info.ClientThreadInfo.ThHandle, NULL);
        }

        ClientsMap.erase(it);
    };

    UnlockMutex(&Mut);
};

/*Функция отключения клиента*/
void CTcpServer::DisconnectClient(ClientID ID) {
    //LockMutex(&Mut);

    ClientsMapArrayIt it = ClientsMap.find(ID);

    if (it != ClientsMap.end()) {
        ClientInfo Info = (*it).second;
        if (shutdown(Info.ClientSocket, SHUT_RDWR) == -1)
            printf("Socket shutdown failed\r\n");
        if (close(Info.ClientSocket) == -1)
            printf("Socket close failed\r\n");

        //pthread_cancel(Info.ClientThreadInfo.ThHandle);
        //pthread_join(Info.ClientThreadInfo.ThHandle, NULL);
        //UnlockMutex(&Mut);
        //ClientsMap.erase(it);
    };

    //UnlockMutex(&Mut);
}

/*Функция отключения всех клиентов*/
void CTcpServer::DisconnectAll() {
    //LockMutex(&Mut);

    ClientsMapArrayIt it = ClientsMap.begin();

    while (it != ClientsMap.end()) {
        ClientInfo Info = (*it).second;
        if (shutdown(Info.ClientSocket, SHUT_RDWR) == -1)
            printf("Socket shutdown failed\r\n");
        if (close(Info.ClientSocket) == -1)
            printf("Socket close failed\r\n");

        //pthread_cancel(Info.ClientThreadInfo.ThHandle);
        pthread_join(Info.ClientThreadInfo.ThHandle, NULL);
        it++;
    };
    ClientsMap.clear();

    //UnlockMutex(&Mut);
}

void* AcceptThread(void* pParam) //поток приема подключений
{
    AcceptThInput* pOldData = (AcceptThInput*) pParam;
    AcceptThInput pData = (*pOldData);
    delete pOldData;
    //создание сокета
    (*pData.pAcceptSock) = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ((*pData.pAcceptSock) == -1) {
        printf("Socket create failed\r\n");
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(pData.ThPort);

    int res;
    res = bind((*pData.pAcceptSock), (sockaddr *) & service, sizeof (service));
    /*if (res == -1)
    {
        printf("Bind failed\r\n");
        service.sin_port = htons(pData.ThPort+1);
        bind((*pData.pAcceptSock), (sockaddr *) & service, sizeof (service));
    }*/
    if (res!=0)
    {
    //while (res != 0) {
        printf("Bind failed\r\nFailed to bind %d port.", pData.ThPort);
        pData.ThPort += 1;
        service.sin_port = htons(pData.ThPort);
        bind((*pData.pAcceptSock), (sockaddr *) & service, sizeof (service));
    }
    //else printf("TEST SERVER\nPort:%d\r\n",(int)pData.ThPort);
    printf("TEST SERVER\nPort:%d\r\n", (int) pData.ThPort);
    if (listen((*pData.pAcceptSock), SOMAXCONN) == -1) {
        printf("Listen failed\r\n");
    }

    SOCKET AcceptSocket;
    bool bQuit = false;
    while (!bQuit) {
        AcceptSocket = accept((*pData.pAcceptSock), 0, 0);
        if (AcceptSocket == -1) {
            printf("Accept closed\r\n");
            bQuit = true;
        } else {
            printf("New client accepted\r\n");
            pData.pParent->StartListenTh(AcceptSocket); //вызов функции работы с клиентом
        };
    }

    pData.pParent->StopAccept(false); //вызов функции закрытия потока
    return 0;
};

void* ListenThread(void* pParam) {
    printf("Listen thread is run\r\n");
    ListenThInput* pOldData = (ListenThInput*) pParam;
    ListenThInput pData = (*pOldData);
    delete pOldData;

    char recvbuf[1024];
    std::string MessageBuf;

    int State = pData.State;
    int CurQst = pData.CurQst;
    int result = pData.result;

    int res;
    bool bQuit = false;

    ListenProc(pData, State, CurQst, result);

    pData.pParent->StopListenTh(pData.CliID, false);
};

void CloseThread(pthread_t ThH, bool bTerminate) {
    if (bTerminate) pthread_cancel(ThH);
    pthread_join(ThH, NULL);
}

void LockMutex(pthread_mutex_t* mut) {
#ifdef PRINT_DEADLOCK
    bool bDeadlock = false;
    if (mut->__data.__lock != 0) {
        printf("--TCPIO DEADLOCKED\r\n");
        bDeadlock = true;
    }
#endif
    pthread_mutex_lock(mut);
#ifdef PRINT_DEADLOCK
    if (bDeadlock) {
        printf("--TCPIO UNLOCKED\r\n");
    }
#endif
}

void UnlockMutex(pthread_mutex_t* mut) {
    pthread_mutex_unlock(mut);
}

void CTcpServer::SendTo(const char* MsgStr, ClientID CliID) {
    LockMutex(&Mut);

    ClientsMapArrayIt it = ClientsMap.find(CliID);

    if (it != ClientsMap.end()) {
        ClientInfo Info = (*it).second;

        int res = 0;
        res = send(Info.ClientSocket, (char*) MsgStr, strlen(MsgStr) + 1, 0);
        if (res == -1) printf("Send failed\r\n");
    };

    UnlockMutex(&Mut);
};

string GetPasswFilePth(const string& username)
{
    string pth = USERS_FOLDER;
    pth += username;
    pth += PASSW_FILE;
    return pth;
}
string GetMessageFilePth(const string& username)
{
    string pth = USERS_FOLDER;
    pth += username;
    pth += MESSAGE_FILE;
    return pth;
}

string RegisterNewUser(const string &data, bool &res) {
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

string LoginNewUser(const string &data, bool &res, string& username) {
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
        ifstream fin(GetPasswFilePth(uname).c_str());
        if (fin.good()) {
            fin >> pass2;
            fin.close();
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
    }
    return mes;
}

string DeleteUser(const string& username) {
    string buf;
    time_t seconds = time(NULL);
    tm* timeinfo = localtime(&seconds);
    
    buf.append(asctime(timeinfo));
    int position = buf.find('\n');
    buf.replace( position, 1, "");
    position = buf.find(" ");
    while ( position != string::npos ) 
    {
        buf.replace( position, 1, "_" );
        position = buf.find(" ", position + 1 );
    } 
    buf.append("_");
    buf.append(username);
    string oldname = USERS_FOLDER;
    oldname.append(username);
    string newname = USERS_FOLDER;
    newname.append(buf);
    int res = rename(oldname.c_str(), newname.c_str());
    if (res!=0)
        return "An error occured while removing existing user! Please, try again later!";
    return "";
}

unsigned long AddMessage(const string& message, const string& username)
{
    string buf;
    unsigned long lastId = 1;
    ifstream inp(GetMessageFilePth(username).c_str());
    if (inp.good())
    {
        inp >> buf;
        if (buf.size() > 0)
        {
            size_t position = buf.find_last_of("<id>");
            if (position != string::npos)
            {
                position+=string(MES_ID).size();
                size_t lastpos = buf.find('\n', position);
                if (lastpos!=string::npos)
                {
                    string numStr = buf.substr(position, lastpos - position);
                    lastId = strtoul(numStr.c_str(), NULL, 10);
                    cout << "Converted number: " << lastId;
                }
                else
                    lastId = 0;
            }
        }
    }
    inp.close();
    
    if (lastId > 0)
    {
        ofstream out(GetMessageFilePth(username).c_str(), ios_base::app);
        if (out.good())
        {
            out << 
        }
        out.close();       
    }
    
    return lastId;
}

// Функция вывода меню сервера

bool ListenProc(ListenThInput& pData, int& State, int& CurQst, int& result) {
    bool bQuit = false;
    ClientID From;
    string MsgStr;
    string MessageBuf;
    string currentUserName;
    string errMessage;
    int mesId;
    
    string buf;
    bool RegisterState, LoginState;
    char* reg_name = (char*) calloc(50, sizeof (char));
    while (!bQuit) {
        switch (State) {
                // Main menu
            case 1: pData.pParent->SendTo("* MAIL *\n", pData.CliID);
                    pData.pParent->SendTo("Select the following items:\n", pData.CliID);
                    if (currentUserName.size() > 0) pData.pParent->SendTo("1 - Add message\n", pData.CliID);
                    pData.pParent->SendTo("2 - Exit\n", pData.CliID);
                    if (currentUserName.size() == 0) pData.pParent->SendTo("3 - Register\n", pData.CliID);
                    if (currentUserName.size() > 0) pData.pParent->SendTo("4 - Logout\n", pData.CliID);
                    else pData.pParent->SendTo("4 - Login\n", pData.CliID);
                    if (currentUserName.size() > 0) pData.pParent->SendTo("5 - Delete user\n", pData.CliID);
                break;

            case 2: pData.pParent->SendTo("Enter your message: ", pData.CliID);
                break;

            case 3: printf("Client with ID: %d is disconnect!\r\n", pData.CliID);
                //pData.pParent->DisconnectClient(pData.CliID);
            pData.pParent->StopListenTh(pData.CliID, true);
                break;

            case 4:
                buf = "Your message is: ";
                buf += MessageBuf;
                pData.pParent->SendTo(buf.c_str(), pData.CliID);
                break;

            case 5: //pData.pParent->SendTo("Wrong input. Press any key.", pData.CliID);
                State = 4;
                break;
            case 6: pData.pParent->SendTo("You are about to sign up. Enter the username consisting of <username>@<password>\n", pData.CliID);
                State = 5;
                break;
            case 7:
                if (RegisterState == true) pData.pParent->SendTo("Registered successfully. Press any key.", pData.CliID);
                else 
                {
                    string buf = "Register failed. ERROR -> ";
                    buf.append(errMessage.c_str());
                    buf.append("Press any key.");
                    pData.pParent->SendTo(buf.c_str(), pData.CliID);
                }
                State = 4;
                break;
            case 8:
                if (currentUserName.size() == 0) 
                {
                    pData.pParent->SendTo("You are about to log in. Enter the username consisting of <username>@<password>\n", pData.CliID);
                    State = 6;
                }
                else
                {
                    pData.pParent->SendTo("Log out successfully.\n", pData.CliID);
                    currentUserName = "";
                    State = 4;
                }
                break;
            case 9:
                if (LoginState == true) 
                {
                    string buf = "Login successfully. User -> ";
                    buf.append(currentUserName);
                    buf.append("Press any key.");
                    pData.pParent->SendTo(buf.c_str(), pData.CliID);                     
                }
                else 
                {
                    string buf = "Login failed. ERROR -> ";
                    buf.append(errMessage);
                    buf.append("Press any key.");
                    pData.pParent->SendTo(buf.c_str(), pData.CliID);
                }
                State = 4;
                break;
            case 10:
                if (currentUserName.size() > 0) 
                {
                    pData.pParent->SendTo("Really delete user? ('Y' - yes, <all other> - no): ", pData.CliID);
                    //pData.pParent->SendTo("Deleting your account.\n", pData.CliID);
                    State = 7;
                }
                else pData.pParent->SendTo("You should be logged in to delete user.\n", pData.CliID);
                break;
            case 11:
                if (errMessage.size() ==0) 
                {
                    pData.pParent->SendTo("Account deleted successfully. Press any key.\n", pData.CliID);
                    State = 4;
                }
                else 
                {
                    string buf = "ERROR -> ";
                    buf.append(errMessage);
                    pData.pParent->SendTo(buf.c_str(), pData.CliID);
                    State = 4;
                }
                break;    
                /*case 4: pData.pParent->SendTo("*** Testing ***\n\nSelect a test:\n\n1 - <First class>\n2 - <Informatika>\n0 - In main menu\n\0", pData.CliID);
                    break;
                // Тест First Class                    
                case 5: pData.pParent->SendTo("\n*** TEST <First class> ***\n\n\0", pData.CliID);
                        if(CurQst==1){
                            pData.pParent->SendTo("Question #1. What is 2x2 ?\n1)3\n2)4\n3)6\n\0", pData.CliID);    
                        }
                        else if(CurQst==2){
                            pData.pParent->SendTo("Question #2. What color is a banana?\n1)Red\n2)Green\n3)Yellow\n\0", pData.CliID);
                        }
                    break;
                // Тест Informatika
                case 6:  pData.pParent->SendTo("\n*** TEST: <Informatika> ***\n\n\0", pData.CliID);
                         if(CurQst==1){
                            pData.pParent->SendTo("Question #1. How many bytes in kilobyte?\n1)1024 byte\n2)8 byte\n3)512 byte\n\0", pData.CliID);    
                         }
                         else if(CurQst==2){
                            pData.pParent->SendTo("Question #2. Turn  4 in the binary system\n1)101\n2)100\n3)1000\n\0", pData.CliID);
                         }
                    break;*/
        }

        if (!ListenRecv(From, MsgStr, pData)) return false;

        switch (State) {
            case 1: if (strcmp((char*) MsgStr.c_str(), "1") == 0)
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
                else State = 5;
                break;

            case 2:
                /*if(register_user((char*)MsgStr.c_str(),pData.pParent,pData.CliID))
                        pData.pParent->SendTo("Registration finished!\0", pData.CliID);
                memmove(reg_name, (char*)MsgStr.c_str(), strlen((char*)MsgStr.c_str())); */
                MessageBuf = MsgStr;
                mesId = AddMessage(MsgStr, currentUserName);
                State = 4;
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
                if (strcmp((char*) MsgStr.c_str(), "Y") == 0) 
                {
                    errMessage = DeleteUser(currentUserName);
                    currentUserName = "";
                    State = 11;
                }
                else
                    State = 1;
                break; 

                /*case 4: if(strcmp((char*)MsgStr.c_str(),"2\0")==0) State=6;
                        else if(strcmp((char*)MsgStr.c_str(),"1\0")==0) State=5;
                        else if(strcmp((char*)MsgStr.c_str(),"0\0")==0) State=1;
                    break;
                        
                case 5: if(CurQst==1){
                            if(strcmp((char*)MsgStr.c_str(),"2\0")==0){ 
                                result++;
                                CurQst=2;
                            }
                            else if(strcmp((char*)MsgStr.c_str(),"1\0")==0 | strcmp((char*)MsgStr.c_str(),"3\0")==0)CurQst=2;      
                         }
                         else if(CurQst==2){
                            if(strcmp((char*)MsgStr.c_str(),"3\0")==0){ 
                                result++;
                                printf("Client with ID:%d result are %d of 2\r\n", pData.CliID,result);
                                    
                            } 
                            else if(strcmp((char*)MsgStr.c_str(),"1\0")==0 | strcmp((char*)MsgStr.c_str(),"2\0")==0){
                                printf("Client with ID:%d result are %d of 2\r\n", pData.CliID,result);
                           }
                                CurQst=1;
                                State=4;
                                char* tmp_path = (char*)calloc(50, sizeof(char));
                                memmove(tmp_path, "./reg/", strlen("./reg/"));
                                strcat(tmp_path,reg_name);
                                FILE* file_user = fopen(tmp_path,"w"); // открытие файла клиента
                                char* tmp_res = (char*)calloc(50, sizeof(char));
                                sprintf(tmp_res,"%d",result);
                                fputs(tmp_res,file_user);
                                fclose(file_user);
                                    
                                result=0;
                         }
                    break;
                        
                case 6: if(CurQst==1){
                            if(strcmp((char*)MsgStr.c_str(),"1\0")>0){ 
                                result++;
                                CurQst=2;
                            }
                            else if(strcmp((char*)MsgStr.c_str(),"2\0")==0 | strcmp((char*)MsgStr.c_str(),"3\0")==0)CurQst=2;      
                         }
                         else if(CurQst==2){
                            if(strcmp((char*)MsgStr.c_str(),"2\0")==0){ 
                                result++;
                                printf("Client with ID:%d result are %d of 2\r\n", pData.CliID,result);
                                    
                            } 
                            else if(strcmp((char*)MsgStr.c_str(),"1\0")==0 | strcmp((char*)MsgStr.c_str(),"3\0")==0){
                                printf("Client with ID:%d result are %d of 2\r\n", pData.CliID,result);
                                    
                            }
                                CurQst=1;
                                State=4;
                                  
                                char* tmp_path = (char*)calloc(50, sizeof(char));
                                memmove(tmp_path, "./reg/", strlen("./reg/"));
                                strcat(tmp_path,reg_name);
                                FILE* file_user = fopen(tmp_path,"w");
                                char* tmp_res = (char*)calloc(50, sizeof(char));
                                sprintf(tmp_res,"%d",result);
                                fputs(tmp_res,file_user);
                                fclose(file_user);
                                    
                                result=0;
                         }
                    break;*/
        }
    }
    return true;
};

// Функция приема сообщений от клиентов

bool ListenRecv(ClientID& From, std::string& MsgStr, ListenThInput& pData) {
    char recvbuf[4096];

    int res = recv(pData.ClientSocket, recvbuf, 1024, 0);

    printf("Received %d symbols!\n", res);
    //if(res==0) {printf("Connection closed\r\n"); return false;}
    //else if(res==-1) {printf("Receive failed\r\n"); return false;}
    //else
    if (res > 0) {
        MsgStr.clear();
        for (int i = 0; i < res; i++)
            if (recvbuf[i] != '\n' && recvbuf[i] != '\r' && recvbuf[i] != '\t' && recvbuf[i] != '\0')
                MsgStr.push_back(recvbuf[i]);
    }
    return true;
};

/*Функция регистрации клиентов*/
bool register_user(char* user_name, CTcpServer* pData, ClientID id) {
    bool have = false;
    for (int i = 0; i < pData->users.size(); i++) {
        if (strcmp(pData->users[i], user_name) == 0) {
            have = true;
        }
    }
    char* tmp2 = (char*) calloc(50, sizeof (char));
    memmove(tmp2, user_name, strlen(user_name));
    char* tmp_path = (char*) calloc(50, sizeof (char));
    memmove(tmp_path, "./reg/", strlen("./reg/"));
    strcat(tmp_path, tmp2);

    if (!have) // если пользователь первый раз
    {
        pData->users.push_back(user_name);
        FILE* file = fopen("./reg/users.txt", "a");
        char* tmp = (char*) calloc(50, sizeof (char));
        memmove(tmp, user_name, strlen(user_name));
        strcat(tmp, (char*) "\r\n");
        fputs(tmp, file);
        fclose(file);
        FILE* file_user = fopen(tmp_path, "w");
        fputs("0", file_user);
        fclose(file_user);
        return true;
    } else //если пользователь уже зарегестрирован
    {
        FILE* file_user2 = fopen(tmp_path, "r");
        char* tmp3 = (char*) calloc(50, sizeof (char));
        char* tmp4 = (char*) calloc(50, sizeof (char));
        strcat(tmp3, "\nYour last result ");
        fgets(tmp4, 2, file_user2);
        strcat(tmp3, tmp4);
        pData->SendTo(tmp3, id);
        fclose(file_user2);
        return false;
    }
}