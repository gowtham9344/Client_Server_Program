#include <stdio.h> 
#include <netdb.h>
#include <stdlib.h> 
#include <string.h> 
#include <time.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#define MAX 100 
#define PORT 8050 
#define PORT1 8090 
#define SA struct sockaddr 
int main(){ 
	int sockfd,connfd; 
	struct sockaddr_in serad,cliad; 
	sockfd=socket(AF_INET,SOCK_STREAM,0); 
	if(sockfd==-1) 
	{ 
		printf("socket not created\n"); 
		exit(0); 
	} 
	printf("socket created\n"); 
	bzero(&serad,sizeof(serad)); 
	serad.sin_family=AF_INET; 
	serad.sin_addr.s_addr=inet_addr("127.0.0.1"); 
	serad.sin_port=htons(PORT);
	if(connect(sockfd,(SA*)&serad,sizeof(serad))!=0){ 
		printf("connection with the server failed\n"); 
		exit(0); 
	} 
	time_t rawtime=0; 
	rawtime=time (NULL); 
	char buff[100];
	char buff1[100];
	strncpy(buff1,ctime(&rawtime),strlen(ctime(&rawtime))-1);
	sprintf(buff,"%s hi",buff1);
	if (send(sockfd,buff,sizeof(buff),0)<0){ 
		printf("not send\n"); 
		exit(0); 
	} 
	printf("msg sent\n"); 
	if(recv(sockfd,buff,sizeof(buff),0)<0){ 
		printf("msg not received\n"); 
		exit(0); 
	}  
	printf("%s\n",buff);
	printf("socket is closed\n"); 
	close(sockfd); 
}

