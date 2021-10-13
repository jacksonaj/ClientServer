#include<stdio.h>  //standard input-output
#include<sys/types.h>  //pthread
#include<sys/socket.h>  //socket(), bind(), listen(), accept()
#include<netinet/in.h> //make available aockaddr_in struct
#include<netdb.h>  //define hostent structure
#include<string.h>  //memset(), memcpy()
#include<stdlib.h>  //exit()
#include<pthread.h>
#include<fcntl.h> //read only>

#define SERVER_PORT 5432
#define MAX_LINE 256
#define MAX_PENDING 5

// STRUCTURE OF PACKET FOR ALL PACKETS
struct packet{
        short type;
	short seqNumber; 
        char uName[MAX_LINE];
        char mName[MAX_LINE];
        char data[MAX_LINE];
	int multiGroup;
};

// STRUCT OF REGISTRATION TABLE THAT HOLDS EACH CLIENT CONNECTED TO SERVER
struct registrationTable{
	int port;
	int sockid;
	char mName[MAX_LINE];
	char uName[MAX_LINE];
	int multiGroup;
};

// function declarations
int pthread_join(pthread_t thread, void **retval);
void *multicaster();
void *join_handler(struct registrationTable *clientData);

// global variables
pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER; // LOCK AND UNLOCK  

struct registrationTable table[10]; // REGISTRATION TABLE 
struct registrationTable client_info; // data from client stored here in server before sent to JH 
struct packet packet_data; // CHAT DATA PACKET	
int myIndex = 0; // index of registration table 
struct packet dataBuffer[MAX_LINE]; // buffer to hold chat data sent
int bufferIndex = 0;
int bIndex = 0;

int main(int argc, char* argv[])
{	
	struct sockaddr_in sin;
	struct sockaddr_in clientAddr;
	struct packet packet_reg; // REGISTRATION PACKET
	char buf[MAX_LINE];
	int s, new_s;
	int len;
	int exit_value[10]; // space for exit value ????? 

	// DECLASE ARRAY OF THREAD OBJECTS
	pthread_t threads[2]; // two threads needed - JH and MC	

	// CREATE MULTICASTER THREAD
	// param1 - thread identifier, param3 - function thread peroforms
	pthread_create(&threads[1], NULL, multicaster, NULL);
	
	/* setup passive open */
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("tcpserver: socket");
		exit(1);
	}


	/* build address data structure */
	bzero((char*)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(SERVER_PORT);

		
	// BINDS SOCKET TO SPECIFIED ADDRESS(IP AND PORT)
	if(bind(s,(struct sockaddr *)&sin, sizeof(sin)) < 0){
		perror("tcpclient: bind");
		exit(1);
	}
	// LISTEN DEFINES HOW MANY CONNECTIONS CAN BE PENDING ON THE SOCKET
	listen(s, MAX_PENDING);


	/* wait for connection, then receive and print text */
	while(1){
		// ACCEPT, PASSIVE OPEN
		// RETURNS WHEN REMOTE PARTICIPANT ESTABLISHED CONNECTION
		// RETURNS NEW SOCKET CORRESPONDING TO CONNECTION
		if((new_s = accept(s, (struct sockaddr *)&clientAddr, &len)) < 0){
			perror("tcpserver: accept");
			exit(1);
		}

		// PRINT CONNECTED CLIENTS PORT 
		printf("\n\n****************************\nNew Client's port is %d \n****************************\n", ntohs(clientAddr.sin_port)); 	
		

		// SERVER RECEIVE FIRST REGISTRATION PACKET, CREATE JOIN HANDLER		
		if((recv(new_s, &packet_reg, sizeof(packet_reg), 0) < 0) || ntohs(packet_reg.type)!=121){
			// if packet is wrong type print error and exit
			printf("\nCould Not Receive FIRST Registration Packet \n");		
			exit(1);
		}
		else{	// if packet can be received, print message, create join handler
			printf("\nServer Received FIRST Registration Request Packet!\n  Join Handler Created \n");

			//STORE CLIENT INFORMATION IN CLIENT_INFO TABLE TO PASS TO JOIN HANDLER
			client_info.port = clientAddr.sin_port;
			client_info.sockid = new_s;
			strcpy(client_info.mName, packet_reg.mName);
			strcpy(client_info.uName, packet_reg.uName);
			client_info.multiGroup = packet_reg.multiGroup;
			printf("  Client_Info:\n  Port: %d\n  SockID: %d\n  Machine Name: %s\n  UserName: %s\n  Group Chat Number: %d\n",
				htons(client_info.port), htons(client_info.sockid),
				client_info.mName, client_info.uName, client_info.multiGroup);

			
			// CREATE JOIN HANDLER THREAD 
			// join_handler uses client_info to communicate with client and update table 
		
			// AFTER INSERT CLIENT INTO REG TABLE IN JH, INCEREMENT ONE FOR NEXT CLIENT
//			myIndex  = myIndex + 1; // INCREMENTS REGISTRATION TABLE INDEX 
			
			pthread_create(&threads[0], NULL, join_handler, &client_info);
		}
		
		// main program waits for join handler thread to complete
//		pthread_join(threads[0],&exit_value);

		// AFTER INSERT CLIENT INTO REG TABLE IN JH, INCEREMENT ONE FOR NEXT CLIENT
		// myIndex  = myIndex + 1; // INCREMENTS REGISTRATION TABLE INDEX 


/*		int bufIndex = 0;
		while(1){

			// RECEIVE CHAT DATA PACKET			
			if((recv(new_s, &packet_data, sizeof(packet_data), 0) < 0) || ntohs(packet_data.type)!=131){
				// if packet is wrong type, print error and exit
				printf("\nCould Not Receive Chat Data Packet From Client\n");
			}
			else{	// if packet can be received, print received and the chat data 
				// find other clients within same group and send the data 
				//printf("\nChat Data Packet Recevied!");
				//printf("\n  Packet Type: %d, Client Machine Name: %s,\n  UserName: %s, Group Chat Num: %d, Chat Data: %s\n", 
				//	htons(packet_data.type), packet_data.mName, packet_data.uName, packet_data.multiGroup,  packet_data.data);
				
				
				// CHAT DATA INTO BUFFER
				//strcpy dataBuffer[bufferIndex].uName = packet_data.uName;
				dataBuffer[bufIndex] = packet_data;
				//strcpy dataBuffer[bufferIndex].data = packet_data.data;
				
				bufIndex++;
			}	
		}
*/
/*		// CONSTRUCT RESPONSE CHAT PACKET
		packet_dataR.type = htons(231);
		strcpy(packet_dataR.mName, packet_data.mName);
		strcpy(packet_dataR.uName, packet_data.uName);
		strcpy(packet_dataR.data, packet_data.data);

		//SEND RESPONSE CHAT PACKET
		printf("\nChat Response Sent to Client\n");
		printf("  Packet Type: %d, Client Machine Name: %s,\n  UserName: %s, Chat Data: %s\n", 
		htons(packet_dataR.type), packet_dataR.mName, packet_dataR.uName, packet_dataR.data);
		if(send(new_s, &packet_dataR, sizeof(packet_dataR), 0) < 0){
			printf("\nChat Data Response Failed to Send to Client\n");
		}
	
		while(len = recv(new_s, buf, sizeof(buf), 0))
			fputs(buf, stdout);
		close(new_s);
*/	
	}

}


void *join_handler(struct registrationTable *clientData) {
	printf("\n\n****************************\nENTER JOIN HANDLER\n****************************\n");

	int newsock;
	newsock = clientData->sockid;
	struct packet packet_reg; 
	struct packet packet_conf;
	struct packet packet_data;
	int bufIndex =0;

	// JOIN HANDLER RECEIVES SECOND REGISTRATION PACKET		
	if((recv(newsock, &packet_reg, sizeof(packet_reg), 0) < 0) || ntohs(packet_reg.type)!=121){
		// if packet is wrong type or does not receive, print error and exit
		printf("\nCould Not Receive SECOND Registration Packet in JH\n");		
		exit(1);
	}
	else{	// if packet can be received, print message and wait for another reg packet
		printf("\nReceived SECOND Registration Request Packet!\n  Waiting For Third Registation Packet... \n");
	}


	// JOIN HANDLER RECEIVES THIRD REGISTRATION PACKET
	if((recv(newsock, &packet_reg, sizeof(packet_reg), 0) < 0) || ntohs(packet_reg.type)!=121){
		// if packet is wrong type or does not receive, print error and exit
		printf("\nCould Not Receive THIRD Registration Packet in JH\n");		
		exit(1);
	}
	else{	// if packet can be received, print and update Registration Table with client info
		printf("\nReceived THIRD Registration Request Packet!\n  Updating Registration Table with Client Info... \n");
		
		// LOCK VARIABLE TO ACCESS TABLE
		pthread_mutex_lock(&my_mutex);

		// STORE CLIENT INFO IN REGISTRATION TABLE
		table[myIndex].port = clientData->port;
		table[myIndex].sockid = clientData->sockid;
		strcpy(table[myIndex].mName, clientData->mName);
		strcpy(table[myIndex].uName, clientData->uName);
		table[myIndex].multiGroup = clientData->multiGroup;
		printf("  Registration Table Details:\n  Port: %d\n  SockID: %d\n  Machine Name: %s\n  UserName: %s\n  Group Chat Number: %d\n",
			htons(table[myIndex].port), htons(table[myIndex].sockid),
			table[myIndex].mName, table[myIndex].uName), table[myIndex].multiGroup;
	
	//	sleep(2);
		
		myIndex++; // INCREMENTS REGISTRATION TABLE INDEX 
		// UNLOCK TABLE AFTER UPDATED
		pthread_mutex_unlock(&my_mutex);
	}
	
	
	// ONCE JOIN HANDLER RECEIVES THREE REGISTRATION PACKETS
	// CONSTRUCT CONFIRMATION PACKET AND SEND ACK TO CLIENT
	packet_conf.type = htons(221);
	strcpy(packet_conf.mName, table[myIndex-1].mName);
	strcpy(packet_conf.uName, table[myIndex-1].uName);
	packet_conf.multiGroup = table[myIndex-1].multiGroup;
		
	// SEND CONFIRMATION PACKET
	printf("\nConfirmation Packet Sent To Client \n");
	printf("  Packet Type: %d, Client Machine Name: %s,\n  UserName: %s, Group Chat Number: %d\n", 
	htons(packet_conf.type), packet_conf.mName, packet_conf.uName, packet_conf.multiGroup);
	if(send(newsock,&packet_conf, sizeof(packet_conf),0) < 0){
		printf("\nConfirmation Packet Send Failed\n");
		exit(1);
	}



	while(1){

		// RECEIVE CHAT DATA PACKET			
		if((recv(newsock, &packet_data, sizeof(packet_data), 0) < 0) || ntohs(packet_data.type)!=131){
			// if packet is wrong type, print error and exit
			printf("\nCould Not Receive Chat Data Packet From Client\n");
			exit(1);
		}
		else{	// if packet can be received, print received and the chat data 
			// find other clients within same group and send the data 
			printf("\nChat Data Packet Recevied!");
			printf("\n  Packet Type: %d, Client Machine Name: %s,\n  UserName: %s, Group Chat Num: %d, Chat Data: %s\n", 
				htons(packet_data.type), packet_data.mName, packet_data.uName, packet_data.multiGroup,  packet_data.data);
			
			
			// CHAT DATA INTO BUFFER
			dataBuffer[bufferIndex].type = packet_data.type;
			dataBuffer[bufferIndex].multiGroup = packet_data.multiGroup;
			strcpy(dataBuffer[bufferIndex].uName, packet_data.uName);
			strcpy(dataBuffer[bufferIndex].mName, packet_data.mName);
			strcpy(dataBuffer[bufferIndex].data, packet_data.data);
			
			bufferIndex++;
		}
	}

		
	printf("\n\n****************************\nEXIT JOIN HANDLER\n****************************\n");
      	// Leave the thread
	pthread_exit(NULL);
}
void *multicaster()
{
	struct packet packet_data; // CHAT DATA PACKET	
	struct packet temp;
	int csock;

	while(1) {
		if(myIndex > 0){
			//LOCK TABLE TO SEND PACKETS	
			
			if(bIndex < bufferIndex){
				temp.type = dataBuffer[bIndex].type;
				temp.multiGroup = dataBuffer[bIndex].multiGroup;
				strcpy(temp.data, dataBuffer[bIndex].data);
				strcpy(temp.uName, dataBuffer[bIndex].uName);
				strcpy(temp.mName, dataBuffer[bIndex].mName);
	
				pthread_mutex_lock(&my_mutex);
				printf("\n[BEGIN MESSAGE]\n");
				printf("\n  TO: ");
				for(int i=0; i<myIndex; i++){
			
					csock = table[i].sockid;
					if(table[i].multiGroup == temp.multiGroup){
						if(send(csock, &temp, sizeof(temp), 0) < 0){
							printf("\n  Chat Data Packet Send Failed!\n");
							//exit(1);
						}
						else{
							printf("%s, ", table[i].uName);
						}
						
					}
				
				} 
				printf("\n  %s: %s",temp.uName, temp.data);
				printf("\n[END MESSAGE]\n");
				//UNLOCK AFTER PACKETS SENT
				pthread_mutex_unlock(&my_mutex);
				bIndex++;

			}
			sleep(1);

		}

	}
	
	
}
