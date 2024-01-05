#include <string.h>  
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <netdb.h>
#include <time.h> 

#define SA struct sockaddr 
#define BACKLOG 10 
#define PORT "8050" 

void sigchld_handler(int s){
	int saved_errno = errno;
	while(waitpid(-1,NULL,WNOHANG) > 0);
	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);	
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(){ 
	int sockfd,connfd; 
	struct addrinfo hints,*servinfo,*p;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;// my ip
	
	if((rv = getaddrinfo(NULL,PORT,&hints,&servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(rv));	
		return 1;
	}
	
	// loop through all the results and bind to the first we can
	for(p = servinfo; p!= NULL; p=p->ai_next){
		sockfd=socket(p->ai_family,p->ai_socktype,p->ai_protocol);
		if(sockfd==-1){ 
			perror("server: socket\n"); 
			continue; 
		} 
		
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
			perror("setsockopt");
			exit(1);	
		}
		
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}

	if(p == NULL){
		fprintf(stderr, "server: failed to bind\n");
		exit(1);	
	}
	

	if(listen(sockfd,BACKLOG) == -1){ 
		perror("listen");
		exit(1); 
	} 
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	printf("server: waiting for connections...\n");
	 
	while(1){ 
		sin_size = sizeof(their_addr); 
		connfd=accept(sockfd,(SA*)&their_addr,&sin_size); 
		if(connfd == -1){ 
			perror("\naccept error\n");
			continue;
		} 
		inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof(s));
		printf("\nserver: got connection from %s\n", s);
  
		int fk=fork(); 
		if (fk==0){ 
			close(sockfd);
			char buff[100];
			if(recv(connfd,buff,sizeof(buff),0) == -1){
				printf("msg not received\n"); 
				exit(0); 
			} 

			//received timestamp of the message
			time_t rawtime=0; 
			rawtime=time(NULL);
			char buff1[100];

			// taking only the time without new line character
			strncpy(buff1,ctime(&rawtime),strlen(ctime(&rawtime))-1);
			int w;
			printf("wait for sometime and enter any number for time difference delay:");
			scanf("%d",&w); 
			sprintf(buff1,"%s %s",buff1,buff);		 
			if (send(connfd,buff1,sizeof(buff1),0) == -1){ 
				printf("not send\n"); 
				exit(0); 
			} 
			close(connfd); 
			exit(0);
		} 
		close(connfd);  
	} 
	close(sockfd); 
	return 0;
} 

