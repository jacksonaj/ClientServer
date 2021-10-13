HOW TO RUN CLIENT AND SERVER 

** COMPILE
	gcc -o server newtcpserver.c // compile server source code and name it server
	gcc -o client newtcpclient.c // compile client aource code and name it client

** RUN
	./server // run server first with no additional arguments
	./client localhost <username> // run client with (2) arguemnts
			1) server name
			2) user name

** CLIENT INPUT
	- Enter desired group chat number, an int between 0 and 99
	- Once the client is registered (indicated by a confirmation packet sent from the 
		server side and a confirmation packet received in the client), Enter
		the message to send to other users in your chosen group. 
		Message cannot exceed 255 char
	- Wait a second or two for another prompt to enter your message


** ADDITIONAL CLIENTS
	- To run more than one client, open new terminal window and use the run
		client command with a new username
	- To communicate with an already running client, enter the same group number
	- If the same group number is entered, the clients can communicate
		// client enter message to send to users may not print on time
		with multiple clients running due to the threading, but the clients 
		will be waiting for input to send to their groups. 
	- If different group numbers are entered, the clients cannot communicate 
		with each other
 

** client and server will run indefinitely
	- client waits for input to send to server/other clients in group
		and waits to receive other data packets
	- server waits to register new clients and to receive data from 
		registered clients
