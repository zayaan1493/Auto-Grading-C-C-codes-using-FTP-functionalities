
In this project I have implemented file transfer protocol on top of TCP sockets and then have implemented a console based Online Judge. There will be a server and multiple clients 
communicating with the server. Each client process will open a new connection with the server. 
I have used select to handle multiple client requests. 
 
Each client can send a c/c++ file to the server through CODEJUD command and the server will 
reply to the client whether the given code is successful or giving error at any point of time during 
execution of code. If execution is successful then the server will also check and reply to the 
client about the acceptance of the c/c++ file. 

Problem :-  
- Write two separate C programs one for TCP server (handles requests from multiple 
servers) and one for client. 
- The server program will use the SELECT system call to handle multiple clients.
- Multiple clients should be simultaneously able to communicate with the server. 
- Server accepts connections from clients. 
- Server receives control information like FTP commands over this connection. 
- Server checks for valid commands, and executes them if there is no error. 
- In order to transfer files etc. design ftp communication set up.


The FTP setup:-
1. RETR : This command causes the remote host to initiate a data connection and to 
send the requested file over the data connection. 
2. STOR : This command causes to store a file into the current directory of the remote 
host. 
3. LIST : Sends a request to display the list of all files present in the directory. 
4. ABOR : This command tells the server to abort the previous FTP service command 
and any associated transfer of data. 
5. QUIT : This command terminates a USER and if file transfer is not in progress , the 
server closes the control connection. 
6. DELE : This command deletes a file in the current directory of server.
7. CODEJUD : This command will take a c/c++ file from client and server will compile , execute 
and match the output with given test cases and will notify back to client about any error or 
correctness of c/c++ file.


Working of server and client is as follows: 
1. Server accepts connections from clients. 
2. Server receives control information like FTP commands over this connection. 
3. Server checks for valid commands, and executes them if there is no error. 
4. Server creates a new data connection with the client for sending data information. 
Only one file can be sent over one data connection. 
5. The same  control connection remains active throughout the user session. 
6. Client receives the response and displays it to the user.
7. *After successful connection- 
Three features to be added at server side- (​CODEJUD​) 
a. Compilation Phase - ​This phase ​will take a String that contains the source code and 
coding language (c/c++).This is the file client wants to test. This phase will compile 
the input and send compilation error messages to clients if any. Else it will create an 
o​bject file and send a compilation success message to the client. 

b. Execution Phase​ -  This phase will take object file , time Limit(1sec). This phase will 
execute your code by providing some secret inputs if required. This phase will decide 
if the input file has any error (TLE or Runtime error). If it succeeds then it will create a 
new output.txt file and send an execution success message to the client. 
 
c. Matching Phase​ - This phase will compare Correct testcase.txt file with the actual 
output.txt file, if it is matched then the successful message will be passed to client, 
else the wrong output message will be passed to client.




