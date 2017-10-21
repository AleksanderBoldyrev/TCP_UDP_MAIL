#include "stdinc.h"
#include "TcpClient.h"

int StrIn(char* pBuf, int BufSize, char* Delim="\r\n\0")
{
	int Ind=0;
	char c;
    c=getchar();
	while(!strchr(Delim,c))
	{
		if(Ind>=(BufSize-1))
		{
			pBuf[Ind]='\0';
			return Ind;
		};
		pBuf[Ind]=c;

        c=getchar();
		Ind++;
	};
	pBuf[Ind]='\0';
	return Ind;
};


int main(int argc, char* argv[])
{
	CTcpClient Client;
    char Cmd[512];
	char IP[16];
	UINT Port=0;

START:
	printf("Enter connect IP: ");
	StrIn(IP,16);
	printf("\r\nEnter connect port: ");
	scanf("%d",&Port);

	if(!Client.Connect(IP,Port)) 
	{
		printf("Can't connect. Return to start.\r\n");
		goto START;
	};
	bool tmp=false;
    while(true)
    {
        StrIn(Cmd,512);
		if(tmp)
			Client.Send(Cmd);
		tmp=true;
    };

	return 0;
}