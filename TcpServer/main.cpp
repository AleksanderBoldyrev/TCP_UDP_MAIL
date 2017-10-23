#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "CTcpServer.h"


int StrIn(char* pBuf, int BufSize, char* Delim="\r\n\0")
{
	int Ind=0;
	char c;
        c=getchar();
        //scanf("%c",&c);
	printf("%c",c);
	while(!strchr(Delim,c))
	{
		if(Ind>=(BufSize-1))
		{
			pBuf[Ind]='\0';
			return Ind;
		};
		pBuf[Ind]=c;

		//scanf("%c",&c);
                c=getchar();
		printf("%c",c);
		Ind++;
	};
	pBuf[Ind]='\0';
	return Ind;
};

int main(int argc, char** argv) {
    setlocale(LC_ALL, "en_US.UTF-8");
    char str[512];
    CTcpServer Server;
    Server.StartAccept(5555);
    while(true){
        scanf("%s",str);
		if(strcmp(str,"exit")==0){ 
                    Server.DisconnectAll();//return 0;
                    break;
                }
    }	//getchar();
   // Server.DisconnectAll();
    return (EXIT_SUCCESS);
}

