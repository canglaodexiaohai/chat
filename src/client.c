#include"DealMessage.h"
#include<errno.h>
#include<string>
#include<stdlib.h>
#define MAX_BUF 4096

#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"
void loop(int i)
{
    int sockfd;//socket
    //创建socket
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
    	perror("socket");
    	exit(1);
    }
    printf("Success to establish a socket...\n");
	//	int flags = fcntl(sockfd, F_GETFL, 0);
//	if(flags == -1)
//	{
//			perror("fcntl");
//	}
//	flags |= O_NONBLOCK;
//	fcntl(sockfd, F_SETFL, flags);
	/*init sockaddr_in*/
	struct sockaddr_in servAddr;
    servAddr.sin_family=AF_INET;
    servAddr.sin_port=htons(SERVER_PORT);
    servAddr.sin_addr.s_addr=inet_addr(SERVER_IP);
 //   bzero(&(servAddr.sin_zero),8);
    /*connect the socket*/
    printf("start connect\n");
    if(connect(sockfd,(struct sockaddr *)&servAddr,sizeof(struct sockaddr_in))==-1)
    {
    	perror("fail to connect the socket");
    	exit(1);
    }
    printf("Success to connect the socket...\n");
    //send-recv
    std::string str;
    Packet readbuf,writebuf;
    char buf[1024];
    while(1)
    {
        cout<<"Input:";
        cin>>str;
       // str="name+"+std::to_string(i);
        writebuf.n_msgLen = str.length()+1;
        strcpy(writebuf.msg,str.c_str());
        send(sockfd,&writebuf,writebuf.n_msgLen+4,0);
        recv(sockfd,&readbuf.n_msgLen,4,0);
        recv(sockfd,buf,readbuf.n_msgLen,0);
        printf("Server:%s\n",buf);
        memset(buf,0,sizeof(buf));

    }

}
int main()
{
  for(int i=0;i<1;++i)
  {
    loop(i);
  }
  return 0;
}
