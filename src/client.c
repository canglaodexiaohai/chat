#include"Common.h"
#include <stdio.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h> //for gethostbyname
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<fcntl.h>
#include<string>
#include<iostream>
#define MAX_BUF 4096
#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"

int main(int argc,char *argv[])
{
    int sockfd;//socket
	char buf[1024];
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
    
    while(1)
    {
        printf("Input:");
       //bin fgets(buf,1024,stdin);
		//buf[1024]="asdsda";
        std::cin>>str;
        send(sockfd,str.c_str(),str.length()+1,0);
        recv(sockfd,buf,1024,0);
        printf("Server:%s\n",buf);
        memset(buf,0,sizeof(buf));
    }

    return 0;
}
