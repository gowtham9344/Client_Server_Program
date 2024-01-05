#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <time.h> 
#define MAX 100 
#define PORT 8050 
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
	serad.sin_addr.s_addr=htonl(INADDR_ANY); 
	serad.sin_port=htons(PORT); 
	if(bind(sockfd,(SA*)&serad,sizeof(serad))!=0){ 
		printf("socket not binded\n"); 
		exit(0); 
	}
	printf("socket binded\n"); 
	if(listen(sockfd,3)!=0){ 
		printf("Listen failed\n"); 
		exit(0); 
	} 
	printf("Server listening\n"); 
	int r; 
	r=100; 
	while(r--){ 
		int len=sizeof(cliad); 
		connfd=accept(sockfd,(SA*)&cliad,&len); 
		if(connfd<0){ 
			printf("server accepted failed\n"); 
			exit(0); 
		} 
		printf("server accepted client\n");  
		int fk=fork(); 
		if (fk==0){ 
			char buff[100];
			if(recv(connfd,buff,sizeof(buff),0)<0){
				printf("msg not received\n"); 
				exit(0); 
			} 
			int w;
			printf("wait for sometime and enter any number:");
			scanf("%d",&w);
			time_t rawtime=0; 
			rawtime=time(NULL); 
			char buff1[100];
			strncpy(buff1,ctime(&rawtime),strlen(ctime(&rawtime))-1);
			sprintf(buff,"%s %s",buff,buff1);		 
			if (send(connfd,buff,sizeof(buff),0)<0){ 
				printf("not send\n"); 
				exit(0); 
			} 
			printf("msg received\n"); 
			exit(0); 
		} 
		else{ 
		 	close(connfd); 
		} 
	} 
	close(sockfd); 
} 

