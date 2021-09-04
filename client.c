/*
Name : client.c
Creator : Md Laadla (mailzayaan1493.ml@kgpian.iitkgp.ac.in)
Roll No : 20CS60R20
Date : 7th February 2021
Description : This is the client program for a task in which we design a File Transfer Protocol model, and allow functionalities like RETR(Retrieve), STOR(Store), LIST(list),
	QUIT(Quit) and DELE(delete). Also Online Judge/ Autograder is implemented using the command CODEJUD in which the client sends a C or CPP file to the server and the server replies
	the client with appropriate messages after executing the sent program file.
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <errno.h>
#include <arpa/inet.h> 
#include <sys/wait.h>
#define MAX 1024 // max size of the buffer when transfering files between the server and client
#define MAX_str 10000 // max length of the string in order to send the list of files


/*............................................................RETR................................................................................*/

void retr(int cli2, char split[], long int size)
{
	/*
	Name : retr
	Input : cli2  -> socket descriptor through which the we need to accept the data
			split -> name of the file which needs to be transfered
			size  -> size of the file that needs to get transfered

	Description : The retr function accepts the file with name 'split' of size 'size' from the server in chunks of MAX size bytes
	*/
	FILE *fp=fopen(split,"wb");

	if (!fp) // Error opening the desired file
	{
		printf("Error file : %s\n",strerror(errno));
		exit(0);
	}

	char temp[MAX]={0};
	int r;

	while (size>0)
	{
		
		if(size>MAX)
		{
			r=recv(cli2,temp,MAX,0);
			fwrite(temp,MAX,1,fp);
			size-=MAX;

		}
		else
		{
			r=recv(cli2,temp,size,0);
			fwrite(temp,size,1,fp);
			size=0;
		}
						
		if (r<0)
		{
			printf("Error reading: %s\n",strerror(errno));
			exit(0);
		}
		else if(r==0)
		{
			printf("ERROR: Server not active\n");
			exit(0);
		}

		memset(temp,0,MAX);//reset the value of temp to NULL
	}

	printf("'''Completed Recieving File'''\n");
	fclose(fp);

}


/*.............................................................STOR..............................................................................*/


void stor(int cli2, FILE *fp, long int size)
{
	/*
	Name :  stor
	Input : cli2 -> socket descriptor through which we need to pass the data
			fp   -> file descriptor of the file to be sent to the client
			size -> size of the file to be sent to the client
	Description : The stor function transfers the file fp of size 'size' to the server in chunks of MAX size bytes
	*/
	fseek(fp,0,SEEK_SET);//set the function pointer at the start of the file
	char temp[MAX]={0};
	int r;

	while(size>0)
	{
		if(size>MAX)
		{
			fread(temp,MAX,1,fp);
			r=send(cli2,temp,MAX,0);
			size-=MAX;
		}
		
		else
		{
			fread(temp,size,1,fp);
			r=send(cli2,temp,size,0);
			size=0;
		}
						
		if (r<0)
		{
			printf("Error reading: %s\n",strerror(errno));
			exit(0);
		}
		else if(r==0)
		{
			printf("ERROR: Server not active\n");
			exit(0);
		}
	
	}
	printf("'''Completed Sending File'''\n");
	fclose(fp);

}


/*..............................................Creation of the data socket and perform specific operation...........................................................*/


void dsocket_com(int port, FILE *fp, long int size, char split[], int operation)
{
	/*
	Name : dsocket_com
	Input : port   -> The port number of the control socket
			fp     -> The file descriptor used in case of stor function calling
			size   -> The size of the file that needs to be transferred or recieved
			split  -> The name of the file that need to be accepted used in retr function
			operation -> signifies which operation needs to be performed  1 --> retr,  2 --> stor, 3 --> list
	*/
	int data_port=port+1;// The port at which the server and client connects

	int cli2;// socket descriptor at the client side

					
					
	cli2 = socket(AF_INET, SOCK_STREAM, 0);

	if (cli2<0)
	{
		printf("Socket Error in client : %s\n",strerror(errno));
		exit(0);
	}

	printf("Data Socket Created!\n");

	struct sockaddr_in serve_addr2;// the socket address

	struct hostent *server2;

	bzero((char *)&serve_addr2,sizeof(serve_addr2));//Initializes buffer

	serve_addr2.sin_family = AF_INET;

	server2 = gethostbyname("localhost");//host address to IP address

	bcopy((char *)server2->h_addr, (char *)&serve_addr2.sin_addr.s_addr,server2->h_length);//copy server IP address
	serve_addr2.sin_port = htons(data_port);// defining the port address
					

	int con2 = connect(cli2, (struct sockaddr *)&serve_addr2, sizeof(serve_addr2));// connect request to the server
	if(con2<0)
	{
		printf("Data connect error : %s\n",strerror(errno));
		exit(0);
	}

	if (operation==1)
	{
		retr(cli2,split,size);
	}

	else if(operation==2)
	{
		stor(cli2,fp,size);
	}

	else if (operation==3)
	{
		char temp[MAX]={0};

		int r=recv(cli2,temp,MAX_str,0);

		if (r<0)
		{
			printf("Error reading: %s\n",strerror(errno));
		}
		else if(r==0)
		{
			printf("ERROR: Server not active\n");
			exit(0);
		}

		printf("MESSAGE FROM SERVER : \n%s",temp);

	}

	printf("Data Socket disconnected\n");
	close(cli2);//close the data socket

}



/*.....................................................................THE MAIN FUNCTION..............................................................................*/


int main(int argc, char** argv)
{


	int port;// The port at which the server and client connects
	sscanf(argv[2], "%d", &port);

	int cli;// socket descriptor at the client side

	char in[MAX]={0};//in--> to read the results sent by the server
	char out[MAX]={0};//out --> the string containing the message to be sent to the server

	
	cli = socket(AF_INET, SOCK_STREAM, 0);

	if (cli<0)
	{
		printf("Socket Creation Problem by Client\n");
		exit(0);
	}

	printf("Socket Created!\n");

	struct sockaddr_in serve_addr;// the socket address

	struct hostent *server;

	bzero((char *)&serve_addr,sizeof(serve_addr));//Initializes buffer

	serve_addr.sin_family = AF_INET;

	server = gethostbyname(argv[1]);//host address to IP address

	bcopy((char *)server->h_addr, (char *)&serve_addr.sin_addr.s_addr,server->h_length);//copy server IP address
	serve_addr.sin_port = htons(port);// defining the port address

	int con = connect(cli, (struct sockaddr *)&serve_addr, sizeof(serve_addr));// connect request to the server

	if (con<0)
	{
		printf("Error while connecting from the Client\n");
		exit(0);
	}
	printf("Connected ...\n");


	while (1)
	{
		memset(in,0,MAX);
		memset(out,0,MAX);
		printf("Command the server : ");
		fgets(out, MAX, stdin);
		out[strlen(out)-1]='\0';

		char *split=strtok(out," ");

		/*................................RETR........................................*/

		if (!strcmp(split,"RETR"))
		{
			
			split=strtok(NULL,"\0");

			out[strlen(out)]=' ';

			int a=access(split,F_OK);
			if (a!=-1)
			{
				printf("FIle already present \n");
				continue;
			}

			send(cli,out,strlen(out),0);

			long int size=0;
			char temp[MAX]={0};

			int r=recv(cli,temp,MAX,0);
			if (r<0)
			{
				printf("Error reading: %s\n",strerror(errno));
				continue;
			}
			else if(r==0)
			{
				printf("ERROR: Server not active\n");
				exit(0);
			}

			size=strtol(temp,NULL,0);

			//printf("size %ld\n",size );

			if (size==-1)
			{
				printf("FIle not present at server's directory\n");
			}
			else if (size==-2)
			{
				printf("File error at server side\n");
			}
			else
			{

				int child=fork();//fork for creating a process for data transfer

				if (child < 0)
				{
					printf("Error forking : %s\n",strerror(errno));
					exit(0);
				}

				else if (child==0)
				{
					close(cli);//close the control socket

					dsocket_com(port, NULL, size, split, 1);
					
					break;
				}

			}	

		}

		/*..........................STOR.............................................*/

		else if (!strcmp(split,"STOR"))
		{
			
			split=strtok(NULL,"\0");

			


			int a=access(split,F_OK);
			if (a==-1)
			{
				printf("FIle not present \n");
				continue;
			}

			out[strlen(out)]=' ';

			send(cli,out,strlen(out),0);

			char temp[MAX]={0};

			int dr=recv(cli,temp,5,0);

			if (dr<0)
			{
				printf("Error reading: %s\n",strerror(errno));
				continue;
			}
			else if(dr==0)
			{
				printf("ERROR: Server not active\n");
				exit(0);
			}
			

			FILE *fp=fopen(split,"rb");
			if (fp==NULL)
			{
				send(cli,"-1",MAX,0);
				printf("File Error: %s\n",strerror(errno));
			}
			else
			{

				fseek(fp, 0, SEEK_END); 

				memset(temp,0,strlen(temp));
  
    			// calculating the size of the file 
    			long int size= ftell(fp);
    			
    			sprintf(temp,"%ld",size);

    			send(cli,temp,strlen(temp),0);

    			fseek(fp,0,SEEK_SET);

    			int child = fork();
				if (child < 0)
				{
					printf("Error forking : %s\n",strerror(errno));
					exit(0);
				}
				else if (child==0)
				{
					close(cli);

					dsocket_com(port, fp, size, NULL, 2);

					break;

				}

				else if (child>0)
					fclose(fp);
			}

		}

		/*..............................LIST.......................................*/

		else if(!strcmp(split,"LIST"))
		{
			split=strtok(NULL,"\0");

			if (split==NULL)
			{
				send(cli,"LIST",4,0);

				char temp[MAX]={0};
				int r=recv(cli,temp,MAX_str,0);

				if (r<0)
				{
					printf("Error reading: %s\n",strerror(errno));
					continue;
				}
				else if(r==0)
				{
					printf("ERROR: Server not active\n");
					exit(0);
				}

				int child = fork();
				if (child < 0)
				{
					printf("Error forking : %s\n",strerror(errno));
					exit(0);
				}
				else if (child==0)
				{
					
					close(cli);

					dsocket_com(port, NULL, 0, NULL, 3);

					break;
				}
			}
			else
			{
				out[strlen(out)]=' ';

				send(cli,out,strlen(out),0);

				char temp[MAX_str]={0};

				int r=recv(cli,temp,MAX_str,0);

				if (r<0)
				{
					printf("Error reading: %s\n",strerror(errno));
					continue;
				}
				else if(r==0)
				{
					printf("ERROR: Server not active\n");
					exit(0);
				}

				printf("MESSAGE FROM SERVER : %s\n",temp);
			}

			
		}
		/*.......................................QUIT..............................................*/

		else if (!strcmp(split,"QUIT"))
		{
			split=strtok(NULL,"\0");

			if (split==NULL)
			{
				send(cli,"QUIT",4,0);
				exit(0);
			}
			else
			{
				out[strlen(out)]=' ';

				send(cli,out,strlen(out),0);

				char temp[MAX]={0};

				int r=recv(cli,temp,MAX_str,0);

				if (r<0)
				{
					printf("Error reading: %s\n",strerror(errno));
					continue;
				}
				else if(r==0)
				{
					printf("ERROR: Server not active\n");
					exit(0);
				}

				printf("MESSAGE FROM SERVER : %s\n",temp);
			}
		}

		/*...................................DELETE............................................*/

		else if(!strcmp(split,"DELE"))
		{
			split=strtok(NULL,"\0");

			out[strlen(out)]=' ';

			send(cli,out,strlen(out),0);

			char temp[MAX]={0};

			int r=recv(cli,temp,MAX,0);

			if (r<0)
			{
				printf("Error reading: %s\n",strerror(errno));
				continue;
			}
			else if(r==0)
			{
				printf("ERROR: Server not active\n");
				exit(0);
			}

			printf("MESSAGE FROM SERVER : %s\n",temp);
		}

		/*.....................................CODE JUDGE.....................................*/

		else if (!strcmp(split,"CODEJUD"))
		{
			split=strtok(NULL," ");

			char filename[MAX]={0};

			sprintf(filename,"%s",split);

			int a=access(filename,F_OK);
			if (a==-1)
			{
				printf("FIle not present \n");
				continue;
			}

			split[strlen(split)]=' ';

			out[strlen(out)]=' ';
			
			send(cli,out,strlen(out),0);//send the command

			char temp[MAX]={0};

			int dr=recv(cli,temp,5,0);

			if(dr==0)
			{
				printf("ERROR: Server not active\n");
				exit(0);
			}
			else if (dr<0)
			{
				printf("Recieving Error : %s\n",strerror(errno));
				continue;
			}
			

			FILE *fp=fopen(filename,"rb");
			if (fp==NULL)
			{
				send(cli,"-1",MAX,0);
				printf("File Error: %s\n",strerror(errno));
			}
			else
			{

				fseek(fp, 0, SEEK_END); 

				memset(temp,0,strlen(temp));
  
    			// calculating the size of the file 
    			long int size= ftell(fp);
    			
    			sprintf(temp,"%ld",size);

    			send(cli,temp,strlen(temp),0);

    			memset(temp,0,MAX);

    			int r=recv(cli,temp,25,0);

				if (r<0)
				{
					printf("Error reading: %s\n",strerror(errno));
					continue;
				}
				else if(r==0)
				{
					printf("ERROR: Server not active\n");
					exit(0);
				}

				if(!strcmp(temp,"INVALID COMMAND"))
				{
					printf("COMPILE MESSAGE FROM SERVER : %s\n",temp);
					continue;
				}

    			fseek(fp,0,SEEK_SET);

    			stor(cli,fp,size);// Calling the Store function to transfer the file

    			memset(temp,0,strlen(temp));

    			r=recv(cli,temp,25,0);

				if (r<0)
				{
					printf("Error reading: %s\n",strerror(errno));
					continue;
				}
				else if(r==0)
				{
					printf("ERROR: Server not active\n");
					exit(0);
				}

				printf("COMPILE MESSAGE FROM SERVER : %s\n",temp);

				if (!strcmp(temp,"COMPILE_ERROR") || !strcmp(temp,"INVALID COMMAND"))
				{
					continue;
				}
				else
				{
					/* When compilation is successful*/
					send(cli,"ACK",5,0);


					memset(temp,0,strlen(temp));

	    			int r=recv(cli,temp,25,0);

					if (r<0)
					{
						printf("Error reading: %s\n",strerror(errno));
						continue;
					}
					else if(r==0)
					{
						printf("ERROR: Server not active\n");
						exit(0);
					}

					printf("RUN_TIME MESSAGE FROM SERVER : %s\n",temp);

					if(!strcmp(temp,"TIME LIMIT EXCEEDED") || !strcmp(temp,"RUN_ERROR"))
					{
						continue;
					}
					else
					{
						send(cli,"ACK",5,0);
						memset(temp,0,strlen(temp));

		    			int r=recv(cli,temp,25,0);

						if (r<0)
						{
							printf("Error reading: %s\n",strerror(errno));
							continue;
						}
						else if(r==0)
						{
							printf("ERROR: Server not active\n");
							exit(0);
						}

						printf("VERDICT FROM SERVER : %s\n",temp);
					}


				}

    		}
		}

		/*...............................INVALID COMMAND CASES....................................*/
		else
		{
			split=strtok(NULL,"\0");

			out[strlen(out)]=' ';

			send(cli,out,strlen(out),0);

			char temp[MAX]={0};

			int r= recv(cli,temp,MAX,0);

			if (r<0)
			{
				printf("Error reading: %s\n",strerror(errno));
				continue;
			}
			else if(r==0)
			{
				printf("ERROR: Server not active\n");
				exit(0);
			}

			printf("%s\n",temp);
		}

		sleep(2);
	}
	return 0;
}