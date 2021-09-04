/*
Name : server.c
Creator : Md Laadla (mailzayaan1493.ml@kgpian.iitkgp.ac.in)
Roll No : 20CS60R20
Date : 7th February 2021
Description : This is the server program for a task in which we design a File Transfer Protocol model, and allow functionalities like RETR(Retrieve), STOR(Store), LIST(list),
	QUIT(Quit) and DELE(delete). Also an online judge is implemeted which is accessed using CODEJUD command whcih transfers program files and the server executes the program and replies
	the client with appropriate messages.
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <errno.h>
#include <dirent.h>
#include <time.h>
#define MAX 1024 // max size of the buffer when transfering files between the server and client
#define MAX_str 10000// max length of the string in order to send the list of files


void del(char filename[])
{
	/*
	Name : del
	Input : filename -> an character array containing the name of the file to be deleted
	*/
	int x=remove(filename);

	if (x==0)
	{
		printf("DELETED THE FILE : %s\n",filename);
	}
	else
	{
		printf("FILE DELETION ERROR : %s\n",strerror(errno));
	}

}



/*...................................................................LIST...........................................................................*/

void list(int client)
{
	/*
	Name : list
	Input : client -> socket descriptor through which we need to pass the data
	*/
	struct dirent *files;//pointer to a directory entry
	DIR *dir = opendir(".");//open the current working directory
	if (!dir)
	{
		send(client,"Error with the directory\n",30,0);
		return;
	}

	char temp[MAX_str]={0};

	while ((files=readdir(dir)))
	{
		strcat(temp,files->d_name);//get the names of the files
		strcat(temp,"\n");
	}
	send(client,temp,strlen(temp),0);
	printf("'''Completed sending list of files''''\n");
	closedir(dir);
}

/*..................................................................RETR..............................................................................*/

void retr(int client,FILE *fp,long int size)
{
	/*
	Name : retr
	Input : client -> socket descriptor through which we need to pass the data
			fp     -> file descriptor of the file to be sent to the client
			size   -> size of the file to be sent to the client
	Description : The retr function transfers the file fp of size 'size' to the client in chunks of MAX size bytes
	*/
	fseek(fp,0,SEEK_SET);//set the function pointer at the start of the file
	char temp[MAX]={0};
	int r;

    while(size>0)
    {
    	if(size>MAX)
    	{
    		fread(temp,MAX,1,fp);
    		r=send(client,temp,MAX,0);
    		size-=MAX;
    	}
    	else
    	{
    		fread(temp,size,1,fp);
    		r=send(client,temp,size,0);
    		size=0;
    	}

		if (r<0)
		{
			printf("Error sending: %s\n",strerror(errno));
			exit(0);
		}
		memset(temp,0,MAX);//reset the value of temp to NULL
    }

    printf("'''Completed Sending File'''\n");

    fclose(fp);//close the file
}

/*................................................................STOR.............................................................................*/

void stor(int client,char filename[],long int size)
{
	/*
	Name : stor
	Input : client -> socket descriptor through which the we need to accept the data
			filename -> name of the file which needs to be transfered
			size     -> size of the file that needs to get transfered

	Description : The stor function accepts the file with name 'filename' of size 'size' from the client in chunks of MAX size bytes
	*/

	FILE *fp=fopen(filename,"wb");//open the file in write mode
	if (!fp)
	{
		printf("Error file : %s\n",strerror(errno));
		exit(0);
	}

	char temp[MAX]={0};
	int r;

	//accept data
	while (size>0)
	{
		
		if (size>MAX)
		{
			r=recv(client,temp,MAX,0);
			fwrite(temp,MAX,1,fp);
			size-=MAX;
		}
		else
		{
			r=recv(client,temp,size,0);
			fwrite(temp,size,1,fp);
			size=0;
		}
		if (r<0)
		{
			printf("Error reading: %s\n",strerror(errno));
			exit(0);
		}

		memset(temp,0,MAX);//reset the value of temp to NULL
	}

	printf("'''Completed Recieving File'''\n");
	fclose(fp);//close the file

}

/*..............................................Creation of the data socket and perform specific operation...........................................................*/


void data_com(int socket, FILE *fp, long int size, char split[], int operation)
{
	/*
	Name : data_com
	Input : socket -> The socket descriptor of the master socket
			fp     -> The file descriptor used in case of retr function calling
			size   -> The size of the file that needs to be transferred or recieved
			split  -> The name of the file that need to be accepted used in stor function
			operation -> signifies which operation needs to be performed  1 --> retr,  2 --> stor, 3 --> list
	*/
	struct sockaddr_in new_addr;

	int len2 = sizeof(new_addr);

	// printf("Creating new data socket\n");

	bzero((char *)&new_addr,sizeof(new_addr));// initializes with zero
	int new_sock2 = accept(socket, (struct sockaddr *)& new_addr,  (socklen_t *)&len2);// accepts and returns the socket descriptor to which the client communicates with the server


	if (new_sock2<0)
	{
		printf("Error in data socket accepting : %s\n",strerror(errno));
		exit(0);
	}

	printf("Data Connection accepted from %s : %d\n",inet_ntoa(new_addr.sin_addr),ntohs(new_addr.sin_port));

	if (operation==1)//for retr operation
	{
		retr(new_sock2,fp,size);
	}
	else if (operation==2)//for stor operation
	{
		stor(new_sock2,split,size);
	}

	else if (operation==3)//for list operation
	{
		list(new_sock2);
	}

	printf("Disconnected from data socket %s : %d \n",inet_ntoa(new_addr.sin_addr),ntohs(new_addr.sin_port));

	close(new_sock2);//close the socket
}





int compare(FILE *output, FILE *testcase)
{
	/*
	Name : compare
	Input: output -> The file descriptor of the output file
		   testcase -> The file descriptor of the testcase file
	Return : 1-> if the files matched
			 0-> if the files does'nt match
	Description : It takes in two file and compares the file.
	*/


	char *line1=NULL; size_t len1=0;
	char *line2=NULL; size_t len2=0;
	int flag=0;

	while (getline(&line1,&len2, output)!=-1 && getline(&line2,&len2,testcase)!=-1)
	{
		/*..............Remove carriage returns if any.........................*/

		if (line1[strlen(line1)-2]=='\r' || line1[strlen(line1)-2]=='\n') 
			line1[strlen(line1)-2]='\0';
		else if (line1[strlen(line1)-1]=='\n' || line1[strlen(line1)-1]=='\r')
			line1[strlen(line1)-1]='\0';

		if (line2[strlen(line2)-2]=='\r' || line2[strlen(line2)-2]=='\n') 
			line2[strlen(line2)-2]='\0';
		else if (line2[strlen(line2)-1]=='\n' || line2[strlen(line2)-1]=='\r')
			line2[strlen(line2)-1]='\0';

		/*......................................................................*/

		if (strcmp(line1,line2))
		{
			flag=1;break;
		}

	}

	if(flag || (getline(&line1,&len2, output)!=-1 && getline(&line2,&len2,testcase)==-1) || (getline(&line1,&len2, output)==-1 && getline(&line2,&len2,testcase)!=-1))
		return 0;
	else 
	{
		return 1;
	}
}


/*..............................................................Main Function.......................................................................................*/

int main(int argc, char** argv)
{


	/*........................Control Socket Initialisation..................................*/

	int port;
	sscanf(argv[1], "%d", &port);// the port taken from command line
	int serv;//socket descriptor for the server

	int max_cli=30;//set maximum number number of clients

	int max_sds,sd;

	int client_socket[max_cli];

	fd_set readfds;

	memset(client_socket,0,max_cli*sizeof(int));

	serv=socket(AF_INET, SOCK_STREAM, 0);//Creation of the master socket


	// AF_NET --> Ipv4 protocol, SOCK_STREAM --> TCP, 0-->IP
	if(serv<=0){
		printf("Error while creating socket in Server");
		exit(0);
	}

	printf("Control Socket Created!\n");
	struct sockaddr_in serv_addr;

	bzero((char *)&serv_addr,sizeof(serv_addr));// initializes with zero

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;//Kernel chooses the addresses
	serv_addr.sin_port = htons(port);//converts the host byte order to network byte order

	int b = bind(serv, (struct sockaddr *)& serv_addr, sizeof(serv_addr));// binding IP address and port number to create a socket

	if (b<0)
	{
		printf("Binding Error from Server\n");
		exit(0);
	}

	printf("Control Socket Binding done\n");

	int l = listen(serv,30);// Listening for clients to connect, total 30 pending clients it can accomodate in the queue

	if (l<0)
	{
		printf("Error while listening by the Server\n");
		exit(0);
	}

	printf("Listening from the Control socket\n");






	/*.....................................Data Socket Initialisation............................... */

	int data_port=port+1;
	
	int serv2,new_sock2;//socket descriptor for the server


	serv2=socket(AF_INET, SOCK_STREAM, 0);
	// AF_NET --> Ipv4 protocol, SOCK_STREAM --> TCP, 0-->IP
	if(serv2<=0){
		printf("Error in data_socket: %s\n",strerror(errno));
		exit(0);
	}



	// printf("Data socket Created!\n");

	struct sockaddr_in server_addr;

	bzero((char *)&server_addr,sizeof(server_addr));
	// initializes with zero

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;//Kernel chooses the addresses
	server_addr.sin_port = htons(data_port);//converts the host byte order to network byte order

	int b2 = bind(serv2, (struct sockaddr *)& server_addr, sizeof(server_addr));// binding IP address and port number to create a socket

	if (b2<0)
	{
		printf("Error in data_bind: %s\n",strerror(errno));	
		exit(0);
	}

	// printf("Data Socket Binding done\n");

	
	int l2 = listen(serv2,5);// Listening for clients to connect, total 3 pending clients it can accomodate in the queue

	if (l2<0)
	{
		printf("Error in data_socket listen : %s\n",strerror(errno));
		exit(0);
	}

	// printf("Starting to accept data\n");

	/*....................................................................................*/


	while (1)
	{


		FD_ZERO(&readfds);//clearing the set of sockets

		FD_SET(serv,&readfds);//adding master socket to the set

		max_sds=serv;

		int child_flag=0;


		//addition of child sockets to the set if there are any

		for (int i = 0; i < max_cli; ++i)
		{
			if (client_socket[i]>0)
				FD_SET(client_socket[i],&readfds);


			if (client_socket[i]> max_sds)//store the highest file descriptor no.
				max_sds=client_socket[i];
		}


		//waiting for some activity

		int act = select(max_sds+1, &readfds, NULL, NULL, NULL);

		if (act <0)
		{
			printf("Select error : %s\n",strerror(errno));
			exit(0);
		}

		//handle connections of the master socket

		if (FD_ISSET(serv, &readfds))
		{
			
		
			int len = sizeof(serv_addr);
			int new_sock = accept(serv, (struct sockaddr *)& serv_addr,  (socklen_t *)&len);// accepts and returns the socket descriptor to which the client communicates with the server
			if (new_sock<0)
			{
				printf("Error accepting : %s\n",strerror(errno));
				exit(0);
			}

			printf("Control Connection accepted from %s : %d\n",inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port));



			for (int i = 0; i < max_cli; ++i)
			{
				if (!client_socket[i])
				{
					
					client_socket[i]=new_sock;

					break;
				}
			}
		}

		// checks for any input from other clients

		for (int i = 0; i < max_cli; ++i)
		{


			if (FD_ISSET(client_socket[i],&readfds))
			{

				char buffer[MAX]={0};
				int rr = recv(client_socket[i], buffer, MAX,0); // reads from the client 

				int len = sizeof(serv_addr);
				getpeername(client_socket[i],(struct sockaddr*) &serv_addr,(socklen_t *)&len);

				printf("COMMAND RECIEVED FROM CLIENT %s  %d : %s \n",inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port),buffer);

				
				if(rr<=0)
				{
					if (rr<0)
					{
						printf("Error Reading : %s\n",strerror(errno));
					}
					printf("Disconnected from %s : %d \n",inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port));
					close (client_socket[i]);
					client_socket[i]=0;
					continue;
				}
				
				

				char *split=strtok(buffer," ");


				/*......................................RETR.............................................*/


				if (!strcmp(buffer,"RETR"))
				{

					split=strtok(NULL,"\0");

					int a=access(split,F_OK);
					
					if (a==-1)
					{
						send(client_socket[i],"-1",2,0);
						printf("File not present\n");
						continue;
					}

					else 
					{
						FILE *fp=fopen(split,"rb");
						if (fp==NULL)
						{
							send(client_socket[i],"-2",MAX,0);
							printf("File Error: %s\n",strerror(errno));
						}
						else
						{

							fseek(fp, 0, SEEK_END); 
	  
	    					// calculating the size of the file 
	    					long int size= ftell(fp);
	    					
	    					memset(buffer,0,strlen(buffer));
	    					sprintf(buffer,"%ld",size);

	    					send(client_socket[i],buffer,strlen(buffer),0);

	    					fseek(fp,0,SEEK_SET);


							int child = fork();
							if (child < 0)
							{
								printf("Error forking : %s\n",strerror(errno));
								exit(0);
							}
							else if (child==0)
							{
								close(serv);

								data_com(serv2,fp,size,NULL,1);

								child_flag=1;
								break;

							}
							else if(child>0)
							{
								fclose(fp);
							}
						}

					}
					

				}

				/*.......................................STOR............................................................*/

				if (!strcmp(buffer,"STOR"))
				{

					send(client_socket[i],"dummy",5,0);//send dummy message inorder to check for the scenario when two successive sends from the client is faultly recieved by one recv function by the server

					split=strtok(NULL,"\0");

					long int size;
					char temp[MAX]={0};

					int r=recv(client_socket[i],temp,MAX,0);

					if(r<=0)
					{
						if (r<0)
						{
							printf("Error Reading : %s\n",strerror(errno));
						}
						printf("Disconnected from %s : %d \n",inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port));
						close (client_socket[i]);
						client_socket[i]=0;
						continue;
					}

					size=strtol(temp,NULL,0);

					if (size==-1)
					{
						printf("File Error from client's side\n");
						continue;
					}
					else
					{
						int child=fork();

						if (child < 0)
						{
							printf("Error forking : %s\n",strerror(errno));
							exit(0);
						}

						else if (child==0)
						{
							close(serv);

							data_com(serv2,NULL,size,split,2);

							child_flag=1;
							break;
						}

					}

				}

				/*.......................................LIST........................................................*/

				else if(!strcmp(split,"LIST"))
				{
					split=strtok(NULL,"\0");
					if (split==NULL)
					{
						send(client_socket[i],"ACK",4,0);
						
						int child=fork();

						if (child < 0)
						{
							printf("Error forking : %s\n",strerror(errno));
							exit(0);
						}

						else if (child==0)
						{
							close(serv);

							data_com(serv2,NULL,0,NULL,3);

							child_flag=1;
							break;
						}

					}
					else
					{
						send(client_socket[i],"INVALID COMMAND",20,0);
					}

				}

				/*........................................QUIT.......................................................*/

				else if((!strcmp(split,"QUIT")))
				{
					split=strtok(NULL,"\0");
					if (split==NULL)
					{
						printf("Disconnected from %s : %d \n",inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port));
						close (client_socket[i]);
						client_socket[i]=0;
					}
					else
					{
						send(client_socket[i],"INVALID COMMAND",20,0);
					}
					
				}

				/*......................................DELETE......................................................*/

				else if (!strcmp(split,"DELE"))
				{

					split=strtok(NULL,"\0");

					int x=remove(split);

					if (x==0)
					{
						send(client_socket[i],"Deleted Successfully",30,0);
					}
					else
					{
						send(client_socket[i],strerror(errno),MAX,0);
					}
				}

				/*.......................................................CODE JUDGE......................................................................................*/


				else if (!strcmp(split,"CODEJUD"))
				{
					send(client_socket[i],"dummy",5,0);//send dummy message inorder to check for the scenario when two successive sends from the client is faultly recieved by one recv function by the server

					split=strtok(NULL," ");

					char filename[MAX]={0};

					sprintf(filename,"%s",split);

					split=strtok(NULL,"\0");



					long int size;
					char temp[MAX]={0};

					int r=recv(client_socket[i],temp,MAX,0);// recieve the size of the file

					if(r<=0)
					{
						if (r<0)
						{
							printf("Error Reading : %s\n",strerror(errno));
						}
						printf("Disconnected from %s : %d \n",inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port));
						close (client_socket[i]);
						client_socket[i]=0;
						continue;
					}

					size=strtol(temp,NULL,0);

					if (size==-1)
					{
						printf("File Error from client's side\n");
						continue;
					}

					else
					{
						FILE *err=fopen("error.txt","w+");

						if (split==NULL)//case when the command syntax is wrong
						{
							send(client_socket[i],"INVALID COMMAND",25,0);
							
							fclose(err);
							del("error.txt");
							continue;
						}

						send(client_socket[i],"ACK",25,0);

						stor(client_socket[i],filename,size);

						char command[MAX]={0};

						char *my_temp=strtok(filename,".");



						if (!strcmp(split,"c"))
							sprintf(command,"gcc %s.%s -o %s.o 2>error.txt",my_temp,split,my_temp);
						else if(!strcmp(split,"cpp"))
							sprintf(command,"g++ %s.%s -o %s.o 2>error.txt",my_temp,split,my_temp);
						else
						{
							send(client_socket[i],"INVALID COMMAND",25,0);
							filename[strlen(filename)]='.';

							del(filename);
							fclose(err);
							del("error.txt");
							continue;
						}


						int s= system(command);

						if (s)
						{
							send(client_socket[i],"COMPILE_ERROR",25,0);
							filename[strlen(filename)]='.';

							del(filename);
							fclose(err);
							del("error.txt");
							continue;//remove the recieved file
						}
						else
						{
							send(client_socket[i],"COMPILE_SUCCESS",25,0);

							//ACK recieve
							r = recv(client_socket[i],temp,5,0);

							if(r<=0)
							{
								if (r<0)
								{
									printf("Error Reading : %s\n",strerror(errno));
								}
								printf("Disconnected from %s : %d \n",inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port));
								close (client_socket[i]);
								client_socket[i]=0;
								continue;
							}



							char filename2[MAX]={0};


							sprintf(filename2,"input_%s.txt",my_temp);

							long int value=0;

							int a=access(filename2,F_OK);
							if (a==-1)
							{
								/* If the input file is not present*/

								memset(command,0,MAX);
								sprintf(command,"timeout 1s ./%s.o >>output_%s.txt 2>error.txt ; echo $? >error.txt",my_temp,my_temp);

								system(command);
								fseek(err,0,SEEK_SET);

								char *line=NULL; size_t len=0;

								getline(&line,&len, err);

								value = strtol(line,NULL,0);


								if (value!=0)
								{
									

									if (value==124)
										send(client_socket[i],"TIME LIMIT EXCEEDED",25,0);
									else
										send(client_socket[i],"RUN_ERROR",25,0);


									/*.....................Delete the extra created files...............*/
									fclose(err);
									del("error.txt");

									

									char output_file[MAX]={0};
									sprintf(output_file,"output_%s.txt",my_temp);
									del(output_file);

									sprintf(output_file,"%s.o",my_temp);
									del(output_file);

									filename[strlen(filename)]='.';
									del(filename);

									/*........................................................................*/

									continue;

								}
								else
								{
									send(client_socket[i],"RUN_SUCCESS",25,0);
								}
							}

							else
							{
								/*If the input file is present*/

								FILE *fp=fopen(filename2,"r");

								if (fp==NULL)
								{
									printf("File Error : %s\n",strerror(errno));
									continue;
								}

								char *line=NULL; size_t len=0;
								

								while (getline(&line,&len, fp)!=-1)// for each line in the input file
								{
									memset(command,0,MAX);

									if (line[strlen(line)-2]=='\r' || line[strlen(line)-2]=='\n')  
										line[strlen(line)-2]='\0';
									else if (line[strlen(line)-1]=='\n' || line[strlen(line)-1]=='\r')
										line[strlen(line)-1]='\0';

									sprintf(command,"echo '%s' | timeout 1s ./%s.o >>output_%s.txt 2>error.txt ; echo $? >error.txt",line,my_temp,my_temp);
									
									/* In case of timeout the linux comamnd returns 124 and in case of succesful execution 0 is returned*/

									system(command);

									fseek(err,0,SEEK_SET);

									char *line=NULL; size_t len=0;

									getline(&line,&len, err);

									value = strtol(line,NULL,0);
									

									if (value!=0)
									{

										break;
									}
									
								}
								if (value!=0)
								{
									if ( value == 124)
										send(client_socket[i],"TIME LIMIT EXCEEDED",25,0);
									else
										send(client_socket[i],"RUN_ERROR",25,0);

									fclose(err);
									del("error.txt");

									

									char output_file[MAX]={0};
									sprintf(output_file,"output_%s.txt",my_temp);
									del(output_file);

									sprintf(output_file,"%s.o",my_temp);
									del(output_file);

									filename[strlen(filename)]='.';
									del(filename);

									continue;
								}

								else
								{
									send(client_socket[i],"RUN_SUCCESS",25,0);
									fclose(fp);
								}

							}

							r = recv(client_socket[i],temp,5,0);

							if(r<=0)
							{
								if (r<0)
								{
									printf("Error Reading : %s\n",strerror(errno));
								}
								printf("Disconnected from %s : %d \n",inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port));
								close (client_socket[i]);
								client_socket[i]=0;
								continue;
							}

							/*......Open and compare the output file and the corresponding testcase file.......*/

							sprintf(filename2,"output_%s.txt",my_temp);


							FILE *fp1=fopen(filename2,"r");
							if (fp1==NULL)
							{
								printf("File Error : %s\n",strerror(errno));
								exit(0);
							}

							sprintf(filename2,"testcase_%s.txt",my_temp);

							a=access(filename2,F_OK);
							if (a==-1)
							{
								// if the testcase file is not present

								send(client_socket[i],"NO TEST_CASE FILE",25,0);
								fclose(fp1);
							}

							else
							{
								FILE *fp2=fopen(filename2,"r");
								if (fp2==NULL)
								{
									printf("File Error : %s\n",strerror(errno));
									continue;
								}

								int result = compare(fp1,fp2);
								if (result)
									send(client_socket[i],"ACCEPTED",25,0);
								else
									send(client_socket[i],"WRONG_ANSWER",25,0);

								fclose(fp1);
								fclose(fp2);
							}
							/*......................................................................................*/	
							
						}
						
						
						/*Close the files opened to do the task eg. error.txt, output_filename.txt, filename.o etc files*/
						char output_file[MAX]={0};
						
						sprintf(output_file,"output_%s.txt",my_temp);
						del(output_file);

						sprintf(output_file,"%s.o",my_temp);
						del(output_file);

						filename[strlen(filename)]='.';
						del(filename);

						fclose(err);
						del("error.txt");

					}
				}

				/*....................................INVALID COMMAND CASES................................................*/

				else
				{
					send(client_socket[i],"INVALID COMMAND",20,0);
				}
			}

		}

		if (child_flag)
		{
			break;
		}
	}

	close(serv);//close the master socket
	
	return 0;
}