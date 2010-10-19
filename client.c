#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define OK 0
#define FAILED -1

#define SEPARATOR ' '

void error(char* msg) {
	perror(msg);
	exit(FAILED);
}

int main(int argc, char* argv[]) {
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent*server;

	char buffer[256];
	if (argc < 3) {
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		return FAILED;
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("ERROR opening socket");
	}
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*) server -> h_addr, (char*)&serv_addr.sin_addr.s_addr, server -> h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR connecting");
	}

	while(1) {
		printf("kivi> ");
		bzero(buffer,256);
		fgets(buffer,255,stdin);
		char tmp[100];
		int len = strlen(buffer) - 1;
		sprintf(tmp, "%d%c", len, SEPARATOR);
		n = write(sockfd,tmp,strlen(tmp));
		n = write(sockfd,buffer,len);
		if (n < 0) {
			error("ERROR writing to socket");
		}
		bzero(buffer,256);
		n = read(sockfd,buffer,255);
		if (n < 0) {
			error("ERROR reading from socket");
		}
		printf("%s\n",buffer);
	}
	
	return 0;
}