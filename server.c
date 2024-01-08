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
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <time.h> 

#define SA struct sockaddr 
#define BACKLOG 10 
#define PORT "3490" 



//it helps us to handle all the dead process which was created with the fork system call.
void sigchld_handler(int s){
	int saved_errno = errno;
	while(waitpid(-1,NULL,WNOHANG) > 0);
	errno = saved_errno;
}


// give IPV4 or IPV6  based on the family set in the sa
void *get_in_addr(struct sockaddr *sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);	
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


// this is the code for server creation. here i have used TCP instead of UDP because i need all the data without any loss. if we use UDP we
// have to implement those in the upper layers.
// this function will return socket descripter to the calling function.
int server_creation(){
	int sockfd;
	struct addrinfo hints,*servinfo,*p;
	int yes = 1;
	int rv;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;// my ip
	
	// set the address of the server with the port info.
	if((rv = getaddrinfo(NULL,PORT,&hints,&servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(rv));	
		return 1;
	}
	
	// loop through all the results and bind to the socket in the first we can
	for(p = servinfo; p!= NULL; p=p->ai_next){
		sockfd=socket(p->ai_family,p->ai_socktype,p->ai_protocol);
		if(sockfd==-1){ 
			perror("server: socket\n"); 
			continue; 
		} 
		
		// SO_REUSEADDR is used to reuse the same port even if it was already created by this.
		// this is needed when the program is closed due to some system errors then socket will be closed automaticlly after few
		// minutes in that case before the socket is closed if we rerun the program then we have use the already used port 	
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
			perror("setsockopt");
			exit(1);	
		}
		
		// it will help us to bind to the port.
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
	

	// server will be listening with maximum simultaneos connections of BACKLOG
	if(listen(sockfd,BACKLOG) == -1){ 
		perror("listen");
		exit(1); 
	} 
	return sockfd;
}

//connection establishment with the client
//return connection descriptor to the calling function
int connection_accepting(int sockfd){
	int connfd;
	struct sockaddr_storage their_addr;
	char s[INET6_ADDRSTRLEN];
	socklen_t sin_size;
	
	sin_size = sizeof(their_addr); 
	connfd=accept(sockfd,(SA*)&their_addr,&sin_size); 
	if(connfd == -1){ 
		perror("\naccept error\n");
		return -1;
	} 
	//printing the client name
	inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof(s));
	printf("\nserver: got connection from %s\n", s);
	
	return connfd;
}

void message_handler(int connfd){
	char buff[100];
	int n;
	//receving data from the client
	if((n = recv(connfd,buff,sizeof(buff),0)) == -1){
		printf("msg not received\n"); 
		exit(0); 
	} 
	buff[n] = '\0';
	//received timestamp of the message
	time_t rawtime=0; 
	rawtime=time(NULL);
	char buff1[100];
			
	// taking only the time without new line character
	strncpy(buff1,ctime(&rawtime),strlen(ctime(&rawtime))-1);
	buff1[strlen(ctime(&rawtime))-1] = '\0';
	int w;
	printf("wait for sometime and enter any number for time difference delay:");
	scanf("%d",&w); 
	sprintf(buff1,"%s %s",buff1,buff);
	
	//sending data to the client		 
	if (send(connfd,buff1,sizeof(buff1),0) == -1){ 
		printf("not send\n"); 
		exit(0); 
	} 
}

// reap all dead processes that are created as child processes
void signal_handler(){
	struct sigaction sa;
	sa.sa_handler = sigchld_handler; 
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
}



int main(){ 
	int sockfd,connfd; 
	
	//server creation .
	sockfd = server_creation();
	
	signal_handler();	

	printf("server: waiting for connections...\n");
	 
	while(1){ 
		
		connfd = connection_accepting(sockfd);
			
		if(connfd == -1){
			continue;
		}

		// fork is used for concurrent server.
		// here fork is used to create child process to handle single client connection because if two clients needs to 
		// connect to the server simultaneously if we do the client acceptence without fork if some client got connected then until 
		// the client releases the server no one can able to connect to the server.
		// to avoid this , used fork, that creates child process to handle the connection.
		int fk=fork(); 
		if (fk==0){ 
			close(sockfd);
			message_handler(connfd);			
			close(connfd); 
			exit(0);
		} 
		close(connfd);  
	} 
	close(sockfd); 
	return 0;
} 

