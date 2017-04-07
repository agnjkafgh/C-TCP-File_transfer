//Names of Authors: Ashwin Natarajan, Chandrakanth Mamillapalli
//Course Number and Name: CSE 434, Computer Networks
//Semester: Fall 2016
//Project Part: 2
//Time spent: 17 hours

//All the required include files.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <dirent.h>
#include <fstream>

//Constants are defined here
#define kill "kill"
#define reset "reset"
#define comma ','
#define space ' '
#define start "<--"
#define finish "-->"
using namespace std;

//This function is used to display a fatal error message and end the program
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

//This function is used to check whether the passed message is the kill command. 1 is returned if the message is indeed "kill" and 0 otherwise.
int str_kill(char a[])
{
    if(strcmp(kill,a)==0)
		return 1;
	else
		return 0;
}

//This function is used to check whether the passed message is the reset command. 1 is returned if the message is indeed "reset" and 0 otherwise.
int str_reset(char a[])
{
	if(strcmp(reset,a)==0)
		return 1;
	else
		return 0;
}

/*This is the data structure defined to reverse the order of lines. Each instance will hold one line and these structures are created dynamically depending on the
number of lines in the received file. They are linked as a stack after being generated.*/
struct stack
{
	char line[256];
	struct stack *next;
};


int main(int argc, char *argv[])
{

    //All the variables used are declared here
    int sockfd, port, n, i, j, lines;
    int kill_flag=0, len = 0, reset_flag = 0;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	char client_id[5];
    char buf[256];
    char unique[] = "Unique client";
    char duplicate[] = "Duplicate client";
	char full[] = "Server full";
    char filename[50];
	char filename1[50];
	char line_array[256];
    char mode, y_or_n;
	ofstream file_w;
	ifstream file_r;
    string line;
	struct stack *top = NULL;
	struct stack *temp = NULL;
	struct stack *temp1 = NULL;
	DIR *p;
	struct dirent *pp;
	p = opendir("./");
    //This if section check whether the number of arguments provided is correct. argc should be four (for 3 arguments)
	if(argc != 4)
	{
		printf("\n\t Usage: ./client <hostname> <client_number> <server_port>\n");
		return(0);
	}

    //Here, we read all the cmd line arguments and save them in variables. The value of argv[0] is always the name of the program.
	bzero(client_id,5);
	port = atoi(argv[3]);
	strcpy(client_id,argv[2]);
    server = gethostbyname(argv[1]);	


    //The next line of code will open a socket and the socket number is returned to the sockfd variable.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    //This block is used to display all the .txt files in the working directory
	cout<<"File list"<<endl;
	if(p!=NULL)
	{
		while((pp = readdir (p)) != NULL)
		{
			len = strlen(pp->d_name);
			if (strncmp(pp->d_name + len - 4, ".txt", 4) == 0)
			{
				cout<<pp->d_name<<endl;
			}
		}
	}
	closedir(p);
	
	printf("Client ID is %s\n",client_id);
	
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    //The above block means that the server could not be reached or the server has not been setup yet (for the given hostname).

    bzero((char *) &serv_addr, sizeof(serv_addr));


    serv_addr.sin_family = AF_INET;         //This means that the addresses will be referred to as internet addresses


    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);


    serv_addr.sin_port = htons(port);


    //In this block, a connection with the server is established and the data is transmitted.
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
	printf("Connection successful.\n");

    //Here, the client ID is transmitted and the client is authenticated.
	printf("Sending client ID number.\n");
	n = write(sockfd,client_id,strlen(client_id));
	if(n<0)
		error("ERROR writing to socket");
	else
		printf("ID successfully sent\n");
    bzero(buf,256);
     n = read(sockfd,buf,255);
    if (n < 0)
         error("ERROR reading from socket");
    printf("The server response is: %s\n\n",buf);
    if(strcmp(buf,duplicate)==0)
    {
        printf("The server has informed that the client ID specified is not unique. Please use another ID\n\n");
        return 0;
    }
	
	if(strcmp(buf,full)==0)
    {
        printf("Server is full. Try again later.\n\n");
        return 0;
    }
    //This infinite loop is used to send of the client's requests in one connection. Entering the message "kill" will terminate the connection.
    while(1)
    {
		fflush(stdin);
		fflush(stdout);
	    bzero(filename,50);
		bzero(buf,256);
        printf("Enter filename with extension or enter kill to terminate or reset to reset: ");
        cin>>filename;    //the filename is read and stored in a char array of size 25.
        if(strlen(filename)==0)
        {
            printf("Please enter valid string name\n\n");
            break;
        }
        else
        {
			reset_flag = 0;
			
			printf("The file name is: %s\n",filename);
			kill_flag  = 0;      //kill_flag variable is used to keep track of whether the "kill" command has been passed.
			reset_flag = 0;      //reset_flag variable is used to keep track of whether the "reset" command has been passed.
			kill_flag = str_kill(filename);
			reset_flag = str_reset(filename);
			//printf("kill_flag = %d\n",kill_flag);
			//cout<<"reset_flag = "<<reset_flag<<endl;
			if (kill_flag==1)
			{
				printf("Killing connection\n\n");
				n = write(sockfd,filename,strlen(filename));
				if (n < 0)
				error("ERROR writing to socket");
				printf("\n");
				break;
			}
			if (reset_flag==1)
			{
				n = write(sockfd,filename,strlen(filename));
				if (n < 0)
					error("ERROR writing to socket");
				cout<<endl;
				fflush(stdin);
				fflush(stdout);
				continue;
			}
			//This block is used to read the mode of file operation (r or w).	
			mode_read:
            printf("Enter mode: ");
            cin>>mode;
			if(mode != 'r' && mode != 'w')
			{
				cout<<"Enter proper mode"<<endl;
				fflush(stdin);
				fflush(stdout);
				goto mode_read;
			}
            bzero(buf,256);
            len = strlen(filename);
            strcpy(buf,filename);
            buf[len] = comma;
			buf[len+1] = space;
            buf[len+2] = mode;
        }
        printf("The command is: %s\n",buf);  //The command is generated with the above info as <filename>,<space><mode>
		
        n = write(sockfd,buf,strlen(buf));

        if (n < 0)
             error("ERROR writing to socket");
		 
		 //Write mode block
		if(mode == 'w')
		{
			
			//If 'w' is the mode specified in the command, the server will put a lock on that file for all other clients. 
			//This lock will not be released until the file write is complete.
			bzero(buf,256);
			n = read(sockfd,buf,255);
			if (n < 0)
				error("ERROR reading from socket");
			cout<<buf<<endl;
			if(strcmp(buf,"File not found.")==0)
				continue;
			if(strcmp(buf,"File is locked.")==0)
				continue;
			
			write_file_name:
			bzero(filename1,50);
			cout<<"Enter the filename of the file to be transmitted or kill to terminate or reset to reset connection : ";
			cin>>filename1;
			kill_flag = 0;      //kill_flag variable is used to keep track of whether the "kill" command has been passed.
			kill_flag = str_kill(filename1);
			reset_flag = 0;     //reset_flag variable is used to keep track of whether the "reset" command has been passed.
			reset_flag = str_reset(filename1);
			//printf("kill_flag = %d\n",kill_flag);
			//cout<<"reset_flag = "<<reset_flag<<endl;
			if (kill_flag==1)
			{
				printf("Killing connection\n\n");
				n = write(sockfd,filename,strlen(filename));
				if (n < 0)
				error("ERROR writing to socket");
				printf("\n");
				break;
			}
			if (reset_flag==1)
			{
				n = write(sockfd,filename,strlen(filename));
				if (n < 0)
					error("ERROR writing to socket");
				cout<<endl;
				continue;
			}
			lines = 0;  //This variable will store the number of lines in the file. It is used to calculate the progress percentage
			j = 0;
			cout<<"Filename1 = "<<filename1<<endl;
			file_r.open(filename1);
			if(file_r.is_open()==0)
			{
				cout<<"Enter a valid filename"<<endl;
				goto write_file_name;
			}
			cout<<"File opened"<<endl;
			n = write(sockfd,start,3);
			if (n < 0)
				error("ERROR writing to socket");
			system("sleep 0.01");
			while(file_r.good())
			{
				getline(file_r,line);
				lines++;
			}		//This block counts the number of lines in the file.
			cout<<"Lines = "<<lines<<endl;
			cout<<"File send started."<<endl;
			file_r.clear();
			file_r.seekg(0, ios::beg);
			fflush(stdin);
			fflush(stdout);
			
			//This block will read and send the file line by line.
			while(file_r.good())
			{
				len = 0;
				bzero(line_array,256);
				getline(file_r,line);
				//cout<<line<<endl;
				len = line.size();
				for(i=0;i<len;i++)
				line_array[i]=line[i];
				n = write(sockfd,line_array,len);
				if (n < 0) 
					error("ERROR writing to socket");
				cout<<"\rIn progress "<<(j*100)/lines<<"%";		//This line of code will calculate and display the progress percentage.
				j++;
				fflush(stdin);
				fflush(stdout);
				system("sleep 0.01");		//Without a 10ms sleep, the two programs seem to lose sync.
			}
			n = write(sockfd,finish,3);
			if (n < 0)
				error("ERROR writing to socket");
			cout<<"\rIn progress "<<(j*100)/lines<<"%"<<endl;
			cout<<"File send complete."<<endl;
			file_r.close();
		}
		
		//The following block is for the read mode.
        else
		{
			top = NULL;		//This variable is used to keep track of the top of the stack.
			temp = NULL;     //This variable is used to keep track of newly created structures until they are inserted into the stack.
			temp1 = NULL;    //This variable is a temporary address holder used to delete structures from the heap after their purpose has been served.
			bzero(buf,256);
			n = read(sockfd,buf,255);
			if (n < 0)
				error("ERROR reading from socket");
			cout<<buf<<endl;
			if(strcmp(buf,"File not found.")==0)
				continue;
			if(strcmp(buf,"File is locked.")==0)
				continue;
			//system("sleep 0.1");
			
			//If the start message ("<--") is receieved, the following block will begin
			if(strcmp(buf,start)==0)
			{
				cout<<"File is being received"<<endl;
				fflush(stdin);
				fflush(stdout);
				file_w.open(filename);			//This line will open (overwrite)/create the file with the same name as the specified input file name, but in the client directory.
				
				while(1)
				{
					bzero(buf,256);
					n = read(sockfd,buf,255);
					if (n < 0)
						error("ERROR reading from socket");
					if(strcmp(buf,finish)==0)		//This while loop will be exited when the finish message ("-->") is received.
						break;
					//cout<<buf<<endl;
					
					/*If the stack is empty, the top variable will hold the address of the newly created structure.*/
					if(top == NULL)			
					{
						top = new struct stack;
						top -> next = NULL;
					}
					
					/*If the stack is not empty, temp will hold the address of the newly created structure until it's corresponding pointer value
					is set to top and top's value is changed to this newly created struct.*/
					else
					{
						temp = new struct stack;
						temp -> next = top;
						top = temp;
						temp = NULL;
					}
					bzero(top -> line, 256);
					strcpy(top -> line, buf);
					//file_w<<buf<<endl;
				}
				
				/*In this block, all the lines are popped from the stack and copied into the file. This process will reverse the sequence of the lines.*/
				temp =  top;
				while(temp != NULL)
				{
					file_w<<temp -> line<<endl;
					temp1 = temp;
					temp = temp -> next;
					delete temp1;
				}
				
				file_w.close();		//The file is closed after this process.
			}
			cout<<buf<<endl;
			cout<<"File read complete"<<endl;
		}
		bzero(buf,256);
		n = read(sockfd,buf,255);		//The server will now ask whether the client wants to continue or not.
		if (n < 0)
			error("ERROR reading from socket");
		cout<<buf<<endl;
		cin>>y_or_n;
		cout<<"y_or_n = "<<y_or_n<<endl;
		fflush(stdin);
		fflush(stdout);
		while(1)
		{
			if(y_or_n != 'y' && y_or_n !='Y' && y_or_n !='n' && y_or_n !='N')
			{
				cout<<"\n response: ";
				cin>>y_or_n;
			}
			else
			{
				break;
			}
		}
		bzero(buf,256);
		buf[0]=y_or_n;
		n = write(sockfd,buf,256);			//The client will send its response.
		if (n < 0) 
			error("ERROR writing to socket");
		if(y_or_n == 'n' || y_or_n == 'N')
		{
			printf("Killing connection\n\n");
			break;
		}
		else
		{
			
			continue;
        }
    }

    close(sockfd);      //The socket is closed in order to terminate the connection.
    return 0;
}
