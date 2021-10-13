#include<stdio.h>  //standard input-output
#include<sys/types.h>  //pthread
#include<sys/socket.h>  //socket(), bind(), listen(), accept()
#include<netinet/in.h>  //make available sockaddr_in struct
#include<netdb.h>  //define hostent structure
#include<string.h>  //memset(), memcpy()
#include<stdlib.h> //exit()
#include <pthread.h>
#define SERVER_PORT 5432
#define MAX_LINE 256

// STRUCTURE OF PACKET //
struct packet{
	short type;
	short seqNumber; 
	char uName[MAX_LINE];
	char mName[MAX_LINE];
	char data[MAX_LINE];
	int multiGroup;
};

// Function Definition
void *receiver(int *sock);

// Global Variable
struct packet packet_data;

int main(int argc, char* argv[])
{
	
	struct hostent *hp;
	struct sockaddr_in sin;
	struct packet packet_reg; // REGISTRATION PACKET
	struct packet packet_conf; // CONFIRMATION PACKET 
	//struct packet packet_data; // CHAT DATA PACKET
	char *host;
	char *user_name;
	char buf[MAX_LINE];
	char userInput[MAX_LINE];
	int s;
	int len;

	// Declare Array of threads
	pthread_t threads[2];	

	// PUT USER INPUT ARGUMENTS INTO VARIABLES
	if(argc == 3){
		host = argv[1];
		user_name = argv[2];
	}
	else{ // IF LACKING ARGUMENT, ERROR
		fprintf(stderr, "usage:newclient server\n");
		exit(1);
	}

	/* translate host name into peer's IP address */
	hp = gethostbyname(host);
	if(!hp){
		fprintf(stderr, "unkown host: %s\n", host);
		exit(1);
	}

	/* active open */
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("tcpclient: socket");
		exit(1);
	}

	/* build address data structure */
	bzero((char*)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	sin.sin_port = htons(SERVER_PORT);

	
	if(connect(s,(struct sockaddr *)&sin, sizeof(sin)) < 0){
		perror("tcpclient: connect");
		close(s);
		exit(1);
	}	


	// CONSTRUCTING REGISTRATION PACKET
	packet_reg.type = htons(121);
	gethostname(buf, 255);
	strcpy(packet_reg.mName, buf);
	strcpy(packet_reg.uName, user_name);	

	// prompt to ask user for chat room
	char userChatGroup[2];
	char *trash;
	
	printf("\n\nEnter Chat Group Number Between 0 and 99: ");
	fgets(userChatGroup, 4, stdin);
	int userGroup = strtol(userChatGroup,trash, 10); 
	
	if( userGroup < 0 || userGroup > 99){
		printf("\nINCOMPATIBLE INPUT\n\n");
		exit(1);
	}
	

	//printf("Group Num: %d", userGroup);
	packet_reg.multiGroup = userGroup;

	// SEND FIRST REGISTRATION PACKET TO SERVER
	printf("\nFIRST Registration Packet Sent To Server");
	printf("\n  Packet Type: %d, Client Machine Name: %s,\n  UserName: %s, Group Chat Number: %d\n", 
		htons(packet_reg.type), packet_reg.mName, packet_reg.uName, packet_reg.multiGroup);
	if(send(s,&packet_reg, sizeof(packet_reg),0) < 0){		
		printf("\n Send Failed\n");
		exit(1);
	}

	
	// SEND SECOND  REGISTRATION PACKET TO SERVER
	printf("\nSECOND Registration Packet Sent To Server(JH)");
	printf("\n  Packet Type: %d, Client Machine Name: %s,\n  UserName: %s, Group Chat Number: %d\n", 
		htons(packet_reg.type), packet_reg.mName, packet_reg.uName, packet_reg.multiGroup);
	if(send(s,&packet_reg, sizeof(packet_reg),0) < 0){		
		printf("\n Send Failed\n");
		exit(1);
	}

	
	// SEND THIRD REGISTRATION PACKET TO SERVER
	printf("\nTHIRD Registration Packet Sent To Server(JH)");
	printf("\n  Packet Type: %d, Client Machine Name: %s,\n  UserName: %s, Group Chat Number: %d\n", 
		htons(packet_reg.type), packet_reg.mName, packet_reg.uName, packet_reg.multiGroup);
	if(send(s,&packet_reg, sizeof(packet_reg),0) < 0){		
		printf("\n Send Failed\n");
		exit(1);
	}
	
	// RECV CONFIRMATION PACKET FROM SERVER
	if((recv(s, &packet_conf, sizeof(packet_conf), 0) < 0) || ntohs(packet_conf.type) != 221){
		// if packet is wrong type, print error and exit
		printf("\nCould Not Receive Confirmation Packet \n");
		exit(1);
	}	
	else{
		printf("\nConfirmation Packet Received!");
		printf("\n  Packet Type: %d, Client Machine Name: %s,\n  UserName: %s, Group Chat Number: %d\n", 
			htons(packet_conf.type), packet_conf.mName, packet_conf.uName, packet_conf.multiGroup);
	}
	

	// create thread for receiver
	pthread_create(&threads[0], NULL, receiver, &s);	

	//main loop: get and send lines of text 
	while(1){
		
		// GET USER INPUT FOR DATA PACKET 
		sleep(3); // pause to allow messages to be received before prompting for message
		printf("\n\nENTER MESSAGE TO SEND USERS IN GROUP %d: ", userGroup);
		fgets(userInput, sizeof(userInput), stdin);
		userInput[MAX_LINE-1] = '\0';
		len = strlen(userInput) + 1;
	
		//CONSTRUCT CHAT DATA PACKET
		packet_data.type = htons(131);
		gethostname(buf, 255);
		strcpy(packet_data.mName, buf);
		strcpy(packet_data.uName, user_name);
		packet_data.multiGroup = userGroup;
		strcpy(packet_data.data, userInput);

		//SEND CHAT DATA PACKET
		printf("\n\nChat Data Packet SENT\n");
		printf("  Packet Type: %d, Client Machine Name: %s\n", 
			htons(packet_data.type), packet_data.mName);

		printf("  [BEGIN MESSAGE]\n");
		printf("     TO: GROUP %d\n     %s: %s", packet_data.multiGroup, packet_data.uName, packet_data.data);
		printf("  [END MESSAGE]\n\n");

		if(send(s, &packet_data, sizeof(packet_data), 0) < 0){
			printf("\nChat Data Packet Failed To Send To Server\n");
			exit(1);	

		}	
	}
}


void *receiver(int *sock){
	int newSock = *sock;

	while(1){
		
		// RECEIVE CHAT DATA PACKET
		if((recv(newSock, &packet_data, sizeof(packet_data), 0) < 0) || ntohs(packet_data.type) != 131){
			// if packet is wrong type, print error and exit
			printf("\nFailed to Receive Chat Data Packet\n");
			exit(1);
		}
		else{	// if packet received, print username and chat data 
			printf("\n\nChat Data Packet RECEIVED!");
			printf("\n  Packet Type: %d, Client Machine Name: %s\n", 
			 htons(packet_data.type), packet_data.mName);	
			
			printf("  [BEGIN MESSAGE]\n");
			printf("     %s: %s", packet_data.uName, packet_data.data);
			printf("  [END MESSAGE]\n\n");
		}

	}
}	
