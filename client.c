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
 
#define PORT "8000" 
#define MAX 100
#define SA struct sockaddr

// give IPV4 or IPV6  based on the family set in the sa
void *get_in_addr(struct sockaddr *sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);	
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// this is the code for client creation. here i have used TCP instead of UDP because i need all the data without any loss. if we use UDP we
// have to implement those in the upper layers.
// this function will return socket descripter to the calling function.
int client_creation(int argc, char* argv[]){
	int sockfd;
	struct addrinfo hints,*servinfo,*p;
	char s[INET6_ADDRSTRLEN];
	int rv;
	

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
		
		
		
		// connect will help us to connect to the server with the addr given in arguments.
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
	
	//printing ip address of the server.
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof(s));
	printf("client: connecting to %s\n", s);
	freeaddrinfo(servinfo);
	
	return sockfd;
}

void message_handler(int sockfd,char buff[]){
	// sending message to the server.
	if (send(sockfd,buff,100,0) == -1){ 
		perror("send");
		exit(1); 
	} 
	
	//receving message from the server.
	if(recv(sockfd,buff,100,0) == -1){ 
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
}
 
int main(int argc,char* argv[]){ 
	int sockfd; 
	char buff[100];
	printf("Enter the message needs to be send to server:");
	scanf("%s",buff);

	// should provide the server ip address when running in the arguments.
	if(argc != 2){
		fprintf(stderr,"usage: client hostname\n");
		exit(1);	
	}
	
	sockfd = client_creation(argc,argv);
	
	
		
	message_handler(sockfd,buff);
	
	close(sockfd);
	return 0;
}

