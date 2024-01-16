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
#include <netdb.h>
#include <time.h> 
#include <poll.h>

#define SA struct sockaddr 
#define BACKLOG 0 
#define PORT "8000" 
#define NUM_FDS 5

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
	}
	
	// server will be listening with maximum simultaneos connections of BACKLOG
	if(listen(sockfd,BACKLOG) == -1){ 
		perror("listen");
		exit(1); 
	} 
	return sockfd;
}

// connection establishment with the client
void connection_accepting(int sockfd, struct pollfd **pollfds, int *maxfds, int *numfds) {
    int connfd;
    struct sockaddr_storage their_addr;
    char s[INET6_ADDRSTRLEN];
    socklen_t sin_size;

    sin_size = sizeof(their_addr);
    connfd = accept(sockfd, (SA*)&their_addr, &sin_size);
    if (connfd == -1) {
        perror("accept");
        exit(1);
    }

    if (*numfds == *maxfds) {
        *pollfds = realloc(*pollfds, (*maxfds + NUM_FDS) * sizeof(struct pollfd));

        if (*pollfds == NULL) {
            perror("realloc");
            exit(1);
        }
        *maxfds += NUM_FDS;
    }
    (*numfds)++;

    ((*pollfds) + *numfds - 1)->fd = connfd;
    ((*pollfds) + *numfds - 1)->events = POLLIN;
    ((*pollfds) + *numfds - 1)->revents = 0;

    // Printing the client name
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
    printf("\nserver: got connection from %s\n", s);
}


void message_handler(struct pollfd *pollfds){
	char buff[100];
	int n;
	//receving data from the client
	if((n = recv(pollfds->fd,buff,sizeof(buff),0)) == -1){
		printf("msg not received\n"); 
		exit(1); 
	} 
	buff[n] = '\0';
	//received timestamp of the message
	time_t rawtime=0; 
	rawtime=time(NULL);
	char buff1[100];
	char buff2[200];
			
	// taking only the time without new line character
	strncpy(buff1,ctime(&rawtime),strlen(ctime(&rawtime))-1);
	buff1[strlen(ctime(&rawtime))-1] = '\0';
	int w;
	printf("wait for sometime and enter any number for time difference delay:");
	scanf("%d",&w); 
	sprintf(buff2,"%s %s",buff1,buff);
	
	//sending data to the client		 
	if (send(pollfds->fd,buff2,sizeof(buff2),0) == -1){ 
		printf("not send\n"); 
		exit(1); 
	} 
	
	close(pollfds->fd); // connection is closed for that client
	pollfds->fd *= -1; // make it negative to ignore it in the future
}




int main(){ 
	int sockfd,connfd; 
	nfds_t nfds = 0;
	struct pollfd *pollfds;
	int maxfds = 0;
	int numfds = 0;
	
	//server creation .
	sockfd = server_creation();
	
	if((pollfds = malloc(NUM_FDS*sizeof(struct pollfd))) == NULL){
		perror("malloc");
		exit(1);
	}
	maxfds = NUM_FDS;
	
	pollfds -> fd = sockfd;
	pollfds -> events = POLLIN;
	pollfds -> revents = 0;
	numfds = 1;

	printf("server: waiting for connections...\n");
	 
	while(1){ 
	
		nfds = numfds;
		if(poll(pollfds,nfds,-1) == -1){
			perror("poll");
			exit(1);
		}
		
		for(int fd = 0; fd < nfds;fd++){
			if((pollfds + fd)->fd <= 0)
				continue;
			
			if(((pollfds + fd)->revents & POLLIN) == POLLIN){
				if((pollfds + fd)->fd == sockfd){
					connection_accepting(sockfd,&pollfds,&maxfds,&numfds);
				}
				else{
					message_handler(pollfds+fd);
				}
			}
		}
	} 
	close(sockfd); 
	return 0;
} 

