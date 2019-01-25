#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

int cmpstrr(char *a, char *b)
{
	int i;
	for(i=0;i<5;i++)
	{
		if(a[i]!=b[i])
			return 0;
	}
	return 1;
}

int main(int argc , char *argv[])
{
	//handle input
	int tcpOrUdp=0;
	int sendOrRead=0;
	char ipAddress[20];
	char port[5];
	long lport;
	char fileName[100];
	unsigned long size;
	unsigned long ctrr = 0;
	int ctr5 = 5;
	printf("program started\n");
	printf("%d\n", argc);
	if(strcmp(argv[1],"tcp\0")==0)
	{
		tcpOrUdp = 1;
	}
	else
	{
		tcpOrUdp = 2;
	}
	if(strcmp(argv[2],"send\0")==0)
	{
		sendOrRead = 1;
	}
	else
	{
		sendOrRead = 2;
	}
	strcpy(ipAddress, argv[3]);
	strcpy(port, argv[4]);
	lport = strtol(port,NULL,10);
	if(argc == 6)
		strcpy(fileName, argv[5]);
	printf("Debug log : %d %d %s %ld\n", tcpOrUdp, sendOrRead, ipAddress, lport);

	if(tcpOrUdp==1)//tcp
	{
		//create socket
		int sockfd = 0;
		sockfd = socket(AF_INET , SOCK_STREAM , 0);
    		if (sockfd == -1)
		{
        		printf("Fail to create a socket.");
    		}
		
		//connection
		if(sendOrRead == 1)
		{
			//send
			char inputBuffer[256] = {};
    			char message[10];
			struct sockaddr_in serverInfo,clientInfo;
    			int addrlen = sizeof(clientInfo);
			int forClientSockfd = 0;
			char ch;
			char receiveMessage[100] = {};
			char m1 = -1; 
			printf("send");
			FILE *file1 = fopen(fileName, "rb");
    			bzero(&serverInfo,sizeof(serverInfo));
			if(!file1) { 
        			puts("Error! File open failed."); 
        			return 1; 
    			} 
			fseek(file1, 0, SEEK_END);
			size = ftell(file1);
			printf("TCP send(1/3) : File opened.\n");
    			serverInfo.sin_family = PF_INET;
    			serverInfo.sin_addr.s_addr = INADDR_ANY;
    			serverInfo.sin_port = htons(lport);
    			bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    			listen(sockfd,5);
			printf("TCP send(2/3) : Start sending.\n");
			forClientSockfd = accept(sockfd,(struct sockaddr*) &clientInfo, &addrlen);
			send(forClientSockfd,fileName,sizeof(fileName),0);
			recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
			
			while(!feof(file1))
			{
				fread(&ch, sizeof(char), 1, file1);
				message[0] = ch;
				//printf("try send %s \n", message);	
        			send(forClientSockfd,message,strlen(message),0);
				//printf("%s sent \n", message);					//DBUG
        			//recv(forClientSockfd,inputBuffer,sizeof(inputBuffer),0);
        			//printf("Get:%s\n",inputBuffer);
				ctrr++;
				if(ctrr/(double)size>ctr5/100)
				{
					ctr5+=5;
					time_t t = time(NULL);
					struct tm datetime = *localtime(&t);
					printf("%d%% %d/%d/%d %d:%d:%d\n",ctr5, \
					datetime.tm_year + 1900, datetime.tm_mon + 1, datetime.tm_mday, \
					datetime.tm_hour , datetime.tm_min, datetime.tm_sec);
				}
    			}
			strcpy(message,"EndOfFile");
			//send(forClientSockfd,message,strlen(message),0);
			send(forClientSockfd,&m1,sizeof(char),0);
			//printf("%s sent \n", message);
			printf("TCP send(3/3) : File sent.\n");
		}
		else
		{
			//receive
    			struct sockaddr_in info;
    			bzero(&info,sizeof(info));
    			info.sin_family = PF_INET;

    			//localhost test
    			info.sin_addr.s_addr = inet_addr(ipAddress);
    			info.sin_port = htons(lport);
    			int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
    			if(err==-1){
        			printf("Connection error\n");
				return 1;
    			}

    			//get message from server
    			char receiveMessage[100] = {};
			char ch;
			int intint;
			char *key = "EndOfFile";
			int ctr=0;
			char myBuf[10] = {};
			int i;
			printf("TCP recv(1/3) : Start receiving.\n");
			recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
			FILE *file2 = fopen(receiveMessage, "wb"); 
			if(!file2) { 
        			puts("Error! Failed to create file.\n"); 
        			return 1; 
    			} 
			printf("TCP recv(2/3) : File created.\n");
			send(sockfd,"YEE",sizeof("YEE"),0);
			memset(receiveMessage, 0, sizeof(receiveMessage));
			while(1)
			{
     				//intint = recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
				intint = recv(sockfd,&ch,sizeof(char),0);
				
				//printf("%s\n",receiveMessage);
				/*if(cmpstrr(receiveMessage, "EndOfFile")==1)//||(receiveMessage[0]=='E'&&receiveMessage[1]=='n'))
				{
					printf("EOF recved\n");
					break;
				}*/
				if(ch==-1)
					break;
				
				printf("%s\n",receiveMessage);
				//ch = receiveMessage[0];
				printf("write : %c\n",ch);
				fwrite(&ch, sizeof(char), 1, file2);
				//fwrite(receiveMessage, strlen(receiveMessage), 1, file2);
				//memset(receiveMessage, 0, sizeof(receiveMessage));
    				
			}
			printf("TCP recv(3/3) : Done.\n");
    			printf("close Socket\n");
			fclose(file2);
    			close(sockfd);
		}
	}
	else//udp
	{
		int udpSocket, portNum, nBytes;
  		char buffer[100], recv_buffer[100];
  		struct sockaddr_in serverAddr, clientAddr;
  		struct sockaddr_storage serverStorage;
  		socklen_t addr_size, client_addr_size;
  		int i, ctr=1, loss_packet=0;
		char ch, key;
		//create socket
		udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

		//Configure settings in address struct
  		serverAddr.sin_family = AF_INET;
  		serverAddr.sin_port = htons(lport);
  		serverAddr.sin_addr.s_addr = inet_addr(ipAddress);
  		memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
		if(sendOrRead==1)
		{
			//send
			addr_size = sizeof serverAddr;
			FILE *file1 = fopen(fileName, "rb");
			
			if(!file1) { 
        			puts("Error! File open failed.\n"); 
        			return 1; 
    			} 
			//FILE *f2 = file1;
			//fseek(f2, 0, SEEK_END);
			//size = ftell(f2);
			printf("UDP send(1/3) : File opened.\n");
			printf("UDP send(2/3) : Start sending\n");
			nBytes = strlen(fileName) + 1;
			//send file name
			do{
					//Send message
    					sendto(udpSocket,fileName,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);
    					//Receive message
                			nBytes = recvfrom(udpSocket,recv_buffer,100,0,NULL, NULL);
				}while(strcmp(fileName, recv_buffer)!=0);
			//send data
			while(!feof(file1))
			{
				fread(&ch, sizeof(char), 1, file1);
				buffer[0] = ch;
				recv_buffer[1] = '0';
				if(ctr%3==0)
				{
					key = '0';
				}
        			else if(ctr%3==1)
				{
					key = '1';
				}
				else if(ctr%3==2)
				{
					key = '2';
				}
				buffer[1] = key;
    				nBytes = strlen(buffer) + 1;
				
				do{
					//Send message
    					sendto(udpSocket,buffer,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);
    					//Receive message
                			nBytes = recvfrom(udpSocket,recv_buffer,100,0,NULL, NULL);
				}while(strcmp(buffer, recv_buffer)!=0);
				ctr++;
    				printf("Received from server: %s\n",recv_buffer);

  			}
			printf("UDP send(2.5/3) : sending Eof\n");
			strcpy(buffer,"EOF\0");
			while(strcmp(buffer, recv_buffer)!=0)
			{
				printf("UDP send(while) : sending %s\n", buffer);
    				/*Send message to server*/
    				sendto(udpSocket,buffer,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);
    				/*Receive message from server*/
                		nBytes = recvfrom(udpSocket,recv_buffer,100,0,NULL, NULL);
			}
			printf("UDP send(3/3) : File sent.\n");
		}
		else
		{
			//receive
 			
			/*Bind socket with address struct*/
  			bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  			/*Initialize size variable to be used later on*/
  			addr_size = sizeof serverStorage;
			nBytes = recvfrom(udpSocket,recv_buffer,100,0,(struct sockaddr *)&serverStorage, &addr_size);
			sendto(udpSocket,recv_buffer,nBytes,0,(struct sockaddr *)&serverStorage,addr_size);
			FILE *file2 = fopen(recv_buffer, "wb");
			if(!file2) { 
        			puts("Error! Failed to create file."); 
        			return 1; 
    			} 
			printf("UDP recv(1/3) : File created.\n");
			
			printf("UDP recv(2/3) : Start receiving.\n");
  			while(1)
			{
				//update key
				if(ctr%3==0)
				{
					key = '0';
				}
        			else if(ctr%3==1)
				{
					key = '1';
				}
				else if(ctr%3==2)
				{
					key = '2';
				}
    				/* Try to receive any incoming UDP datagram. Address and port of 
      				requesting client will be stored on serverStorage variable */
    				nBytes = recvfrom(udpSocket,recv_buffer,100,0,(struct sockaddr *)&serverStorage, &addr_size);
				printf("Received %s \n", recv_buffer);
				printf("Received key %c \n", recv_buffer[1]);
				//check for eof
				if(strcmp(recv_buffer, "EOF\0")==0||(recv_buffer[0]=='E'&&recv_buffer[1]=='O'))
				{
					printf("EOF recved \n");
					sendto(udpSocket,recv_buffer,nBytes,0,(struct sockaddr *)&serverStorage,addr_size);
					break;
				}

				//check data
				if(recv_buffer[1]==key)
				{
					//save data
					ch = recv_buffer[0];
					fwrite(&ch, sizeof(char), 1, file2);
					printf("Write %c \n", ch);
					/*ctrr++;
					if(ctrr/(double)size>ctr5/100)
					{
						ctr5+=5;
						time_t t = time(NULL);
						struct tm datetime = *localtime(&t);
						printf("%d%% %d/%d/%d %d:%d:%d\n",ctr5, \
						datetime.tm_year + 1900, datetime.tm_mon + 1, datetime.tm_mday, \
						datetime.tm_hour , datetime.tm_min, datetime.tm_sec);
					}*/
					//send ack
					sendto(udpSocket,recv_buffer,nBytes,0,(struct sockaddr *)&serverStorage,addr_size);
					ctr++;
				}
				else
				{
					//data loss
					loss_packet++;
					sendto(udpSocket,recv_buffer,nBytes,0,(struct sockaddr *)&serverStorage,addr_size);
				}
  			}
			printf("UDP recv(3/3) : Done.\n");
			printf("Total loss packet : %d\n", loss_packet);
			fclose(file2);
		}
	}
	
	return 0;
}
