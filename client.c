#include <stdio.h> 
#include <netdb.h>
#include <stdlib.h> 
#include <unistd.h>
#include <errno.h>
#include <string.h> 
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h> 
#include <sys/types.h> 
 
#define PORT "8050" 
#define MAX 100
#define SA struct sockaddr

void *get_in_addr(struct sockaddr *sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);	
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
 
int main(int argc,char* argv[]){ 
	int sockfd,numbytes; 	
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	char buff[100];
	printf("Enter the message needs to be send to server:");
	scanf("%s",buff);

	if(argc != 2){
		fprintf(stderr,"usage: client hostname\n");
		exit(1);	
	}
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(argv[1],PORT,&hints,&servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(rv));	
		return 1;
	}

	for(p = servinfo; p!= NULL; p=p->ai_next){
		sockfd=socket(p->ai_family,p->ai_socktype,p->ai_protocol);
		if(sockfd==-1){ 
			perror("server: socket\n"); 
			continue; 
		}
		
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		} 
		break;
	}

	if(p == NULL){
		fprintf(stderr, "client: failed to connect\n");
		return 2;	
	}
	
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof(s));
	printf("client: connecting to %s\n", s);
	
	freeaddrinfo(servinfo);
		
	
	if (send(sockfd,buff,sizeof(buff),0) == -1){ 
		perror("send");
		exit(1); 
	} 

	if(recv(sockfd,buff,sizeof(buff),0) == -1){ 
		perror("recv");
		exit(1); 
	}  
	
	// time of received the message from the server
	time_t rawtime=0; 
	rawtime=time (NULL); 
	char buff1[MAX];
	strncpy(buff1,ctime(&rawtime),strlen(ctime(&rawtime))-1);
	sprintf(buff,"%s %s",buff,buff1);
	printf("The final received message with the time stamp is \n%s\n",buff); 
	close(sockfd); 
	return 0;
}

