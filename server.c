//Names of Authors: Ashwin Natarajan, Chandrakanth Mamillapalli
//Course Number and Name: CSE 434, Computer Networks
//Semester: Fall 2016
//Project Part: 2
//Time spent: 17 hours

/*Design Specifications:
There are three main specifications that are worth mentioning.
1) class lock_table_class:
	A class object was used because it helps modularize the code and also restrict the access to the class variables.
	Variables:
		This class has 3 variables. filename[25][50], lock[25] and count.
		These variables are used to keep track of filenames, lock and total number of files in the working directory respectively.
		The file list is generated at the start of the program execution.
	Methods:
		void populate(char a[])
			This method will add the specified filename a[] to the existing list and will also increment the count.
		int show_count()
			This method will return the value of count.
		void set_lock(int pos, int val)
			This method will set the lock value for the specified position (pos) as val.
		int check_lock(int pos)
			This method will check the lock status at the specified position (pos).
		int find(char a[])
			This method will search the object for the given filename a, and will return the position of the filename. It returns 999 if the filename is not found.
		void display()
			This method is used to print the list of filenames.
			
2) struct stack:
	This structure is used to build stacks of lines in order to reverse the order.
	It has 2 variables, char line[256] which holds the line itself and struct stack *next which will point to the next structure in the stack.
	
3) int * active_connections = (int*) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0):
	This is a simple integer. But what's interesting about this variable is that the pointer will hold a shared memory location's address.
	This ensures that every child process will be able to make modifications to the variable in the parent process.
*/

//All the required include files.
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <fstream>
#include <dirent.h>

//Constants are defined here
#define client_table_size 25
#define kill "kill"
#define start "<--"
#define finish "-->"
#define reset "reset"

using namespace std;

//This function is used to display a fatal error message and end the program
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

/*This is the class object used to maintain the list of file names and their associated lock values
  A class object was used here because it is much safer to access the member variable via class methods and not directly.  */
class lock_table_class
{
	private:
		//All the variables are stored in private so that they can only be safely accessed by the public object methods.
		int lock[25];		//This array stores the lock values. 0 = unlocked. 1 = read locked. 2 = write locked.
		int count;
		char filename[25][50];
	public:
		lock_table_class()		//This is the constructor function that initializes all the variables.
		{
			bzero(lock,25);
			int i=0;
			for(i=0;i<25;i++)
				bzero(filename[i],50);
			count = 0;
			cout<<"Object initialized";
		}
		
		void populate(char name[])		//This function is used to add a filename to the list.
		{
			strcpy(filename[count],name);
			count++;
		}
		
		int show_count()				//This function is used to display the number of files in the working directory.
		{
			return count;
		}
		
		void set_lock(int pos, int val)	//This function is used to set a lock value at a specified position.
		{
			lock[pos] = val;
		}
		
		int check_lock(int pos)			//This function is used to check the lock value at a specified position
		{
			return lock[pos];
		}
		
		void display()					//This function is used to list out all the filenames.
		{
			int i = 0;
			for(i=0;i<count;i++)
				cout<<filename[i]<<endl;
		}
		
		int find(char name[])				//This function returns the position number of the specified filename. Returns 999 if not found.
		{
			int i = 0, pos = 0, flag = 0;
			for(i=0;i<count;i++)
			{
				if(strcmp(filename[i],name)==0)
				{
					pos = i;
					flag = 1;
					break;
				}
			}
			if(flag == 1)
				return pos;
			else return 999;
		}
	
};

/*This is the data structure defined to reverse the order of lines. Each instance will hold one line and these structures are created dynamically depending on the
number of lines in the received file. They are linked as a stack after being generated.*/
struct stack
{
	char line[256];
	struct stack *next;
};

//This function is used to check whether the passed message is the kill command. 1 is returned if the message is indeed "kill" and 0 otherwise.
int str_kill(char a[])
{
    if (strcmp(kill,a) == 0)
		return 1;
	else
		return 0;
}

//This function is used to check whether the passed message is the reset command. 1 is returned if the message is indeed "reset" and 0 otherwise.
int str_reset(char a[])
{
	if (strcmp(reset,a)==0)
		return 1;
	else
		return 0;
}



int main(int argc, char *argv[])
{
	//This block is used to check whether only one argument is passed.
	 if(argc!=2)
	{
		   printf("\t Usage: ./server <Port_number>\n");
		   return 0;
	}
	//In this section, all the variables are defined and initialized
	 int id_table[client_table_size];
	 bzero(id_table,client_table_size);
	 int count = 0, i=0,j=0;
     int sockfd, newsockfd, portno;
     int err_flag;   //
     int pid, pos, mode, found_pos;
     socklen_t clilen;
	 int lines, prog;
	 char unique[] = "Unique client";
     char duplicate[] = "Duplicate client";
	 char full[] ="Server full";
	 char buffer[256];
	 char line_array[256];
	 string line;
	 char filename[50];
	 struct sockaddr_in serv_addr, cli_addr;
     int n, id, locked, len;
	 std::ifstream file_r;
	 std::ofstream file_w;
	 char y_or_n;
	 struct stack *top = NULL;
	 struct stack *temp = NULL;
	 struct stack *temp1 = NULL;
	 
	 /*The number of active connections variable is mapped to a shared memory address using mmap() because when a child process is created, 
	   a second copy of all variables is also created for each child process. Hence the number of active connections cannot be tracked if a shared memory location is not allocated.*/
	 int * active_connections = (int*) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	 *active_connections = 0;
	 
	 /*The lock_table_class object is mapped to a shared memory address using mmap() because when a child process is created, 
	   a second copy of all variables is also created for each child process. Hence the lock values cannot be tracked if a shared memory location is not allocated.*/
	 lock_table_class * table = (lock_table_class*)mmap(NULL, sizeof(lock_table_class), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	 
     //The socket is opened here
	 sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");
	bzero(filename,50);
	file_listing:
	 DIR *p;
	 struct dirent *pp;
	 p = opendir("./");
	 if(p!=NULL)
	 {
		while((pp = readdir (p)) != NULL)
		{
			len = strlen(pp->d_name);
			if (strncmp(pp->d_name + len - 4, ".txt", 4) == 0)
			{
				//strcpy(file_table[file_count],pp->d_name);
				//file_count++;
				table->populate(pp->d_name);
			}
		}
	 }
	 
	 cout<<"File Count = "<<table->show_count()<<endl;
	 closedir(p);
	 cout<<"List of files:"<<endl;
	 table->display();
	 cout<<endl;

     bzero((char *) &serv_addr, sizeof(serv_addr));

     portno = atoi(argv[1]);

     serv_addr.sin_family = AF_INET;

	//contains the IP address of the host
     serv_addr.sin_addr.s_addr = INADDR_ANY;

	//contain the port number
     serv_addr.sin_port = htons(portno);

	//bind() system call binds a socket to an address, in this case the address of the current host and port number on which the server will run.
	//three arguments, the socket file descriptor, the address to which is bound, and the size of the address to which it is bound.
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");

	//This infinite while loop contains the main server code.			  
	while(1)
	{
		err_flag = 0;		//err_flag is used to keep track of whether the supplied client ID already exists in the list or not.
		cout<<"Active connections = "<<*active_connections<<endl;
		printf("The client table is: ");		//This block displays the client table as of that moment.
		if(count==0)
		{
			printf("empty\n");
		}
		else
		{
			for(i=0;i<count;i++)
			{
				printf("%d\t",id_table[i]);
			}
			printf("\n");
		}
		cout<<endl;
		//This statement commands the server to listen for incoming connections. The second argument represents the number of connections that can wait on the server at any point of time.
		listen(sockfd,5);
		printf("Listening for clients...\n");
		clilen = sizeof(cli_addr);

		//accept() system call causes the process to block until a client connects to the server.
		newsockfd = accept(sockfd,
                 (struct sockaddr *) &cli_addr,
                 &clilen);

		if (newsockfd < 0)
          error("ERROR on accept");
		printf("Socket accepted\n");

		bzero(buffer,256);
		printf("Buffer initialized. Reading\n");

		//reads from the socket into a buffer for a maximum of 255 characters
		//read call uses new file descriptor, the one returned by accept()
		n = read(newsockfd,buffer,255);
		if (n < 0) error("ERROR reading from socket");
	 

		//both the server can read and write after a connection has been established.
		//everything written by the client will be read by the server, and everything written by the server will be read by the client.
		id = atoi(buffer);
		printf("The client address is: %d\n",id);

		//This section checks the authenticity of the client's ID number.
		if(count==0)
		{
			printf("Unique client\n");
			id_table[count]=id;
			count++;
		}
		else
		{
			for(i=0,j=0;i<count;i++)
			{
				if(id_table[i]==id)
				{
					j++;
					break;
				}
			}
			if(j==0)
			{
				printf("Unique client\n");
				id_table[count]=id;
				count++;
			}
			else
			{
				printf("Duplicate client. Terminating Connection\n");
				printf("----------------------------------------\n");
				err_flag = 1;
			}
		}

		//err_flag is set to 1 if a duplicate connection is detected.
		if(err_flag)
		{
			n = write(newsockfd,duplicate,16);
			if (n < 0)
				error("ERROR writing to socket");
				close(newsockfd);
				continue;
		}
		

		//Once the client is verified as unique, the control is passed onto the next block.
		else
		{
			//The connection is terminated if the number of active connections at that time is greater than 5.
			if(*active_connections < 5)
				*active_connections = *active_connections + 1;
			else
			{
				n = write(newsockfd,full,11);
				if (n < 0)
					error("ERROR writing to socket");
				close(newsockfd);
				printf("Server full. Terminating Connection\n");
				printf("----------------------------------------\n");
				id_table[count] = 0;
				count--;
				continue;
			}
			
			n = write(newsockfd,unique,13);
			if (n < 0)
			error("ERROR writing to socket");


			//After verifying the client, a child process is created to handle that request.
			pid = fork();	
			if(pid < 0)		
				error("Forking failed\n");



			//This entire block contains the instructions that the child process should perform
			else if(pid == 0)
			{

				//After a connection a client has successfully connected to the server
				//initialize the buffer using the bzero() function
			
				//This loop is used to process all of the client's requests and it can also detect the kill command.
				while(1)
				{
					fflush(stdin);
					bzero(filename,50);  //The first request is read here.
					
					bzero(buffer,256);
					do
					{
						n = read(newsockfd,buffer,255);
						if (n < 0) error("ERROR reading from socket");
					}while(strlen(buffer)==0);
					printf("Client %d: %s",id,buffer);
					
					//This block checks if the kill command has been passed by the client.
	 				if(str_kill(buffer)==1)
					{
						
						*active_connections = *active_connections - 1;	//The shared variable is updated if the kill command is received
						printf("Kill command received\n");
						printf("----------------------------------------\n");
						cout<<"New act_conn = "<<*active_connections<<endl;
						
						break;
					}
					
					//This block checks if the reset command has been passed by the client.
					if(str_reset(buffer) == 1)
					{
						/*n = write(newsockfd,"Server: Connection Reset.",24);
						if (n < 0) 
							error("ERROR writing to socket");*/
						cout<<"Connection reset"<<endl<<endl;
						continue;
					}
					
					else
					{
						pos = strlen(buffer) - 3;
						mode = buffer[pos+2];
						for(i=0;i<pos;i++)
							filename[i]=buffer[i];
						printf("\nFilename: %s  Mode: %c\n",filename,mode);
						found_pos = 0;
						cout<<"searching for file: "<<endl;
						
						//The find() object method is used to find the position of the file and is stored in found_pos.
						found_pos = table->find(filename);
						if(found_pos == 999)
						{
							cout<<"File not found."<<endl;
							n = write(newsockfd,"File not found.",15);
							if (n < 0) 
								error("ERROR writing to socket");
							continue;
						}
						cout<<"found at: "<<found_pos<<endl;
						cout<<"Finished checking filename"<<endl;
						
						//The following is the write mode block
						if(mode == 'w')
						{
							top = NULL;
							temp = NULL;
							temp1 = NULL;
							
							locked = table->check_lock(found_pos);		//The check_lock() method is used to check the lock value at the specified position.
							
							cout<<"Lock value: "<<locked<<endl;
							
							//If the lock value is non-zero, then the write operation is not allowed.
							if(locked != 0)
							{
								n = write(newsockfd,"File is locked.",15);
								if (n < 0) 
									error("ERROR writing to socket");
								continue;
							}
							
							n = write(newsockfd,"Ready for write.",16);
							if (n < 0) 
								error("ERROR writing to socket");
							cout<<"ready msg sent(w)"<<endl;
							
							//The file is opened with the specified attributes so that the incoming text can be appended to the existing file (after reversing).
							file_w.open(filename,ofstream::out|ofstream::app);
							table->set_lock(found_pos, 2);		//The file is locked to level2 (write lock)
							
							cout<<"New lock status: "<<table->check_lock(found_pos)<<endl;
							cout<<"File open status: "<<file_w.is_open()<<endl;
							file_w<<endl;
							bzero(buffer,256);
							n = read(newsockfd,buffer,255);
							if (n < 0)
								error("ERROR reading from socket");
							cout<<buffer<<endl;
							
							//the next incoming message can either be a kill command, a reset command or a start command.
							if(str_kill(buffer)==1)
							{
								*active_connections = *active_connections - 1;
								printf("Kill command received\n");
								printf("----------------------------------------\n");
								cout<<"New act_conn = "<<*active_connections<<endl;
								table->set_lock(found_pos,0);
								cout<<"Lock released"<<endl;
								break;
							}
							
							if(str_reset(buffer) == 1)
							{
								/*n = write(newsockfd,"Server: Connection Reset.",24);
								if (n < 0) 
									error("ERROR writing to socket");*/
								cout<<"Connection reset"<<endl<<endl;
								table->set_lock(found_pos,0);
								cout<<"Lock released"<<endl;
								continue;
							}
							
							else if(strcmp(buffer,start)==0)
							{
								while(1)
								{
									bzero(buffer,256);
									n = read(newsockfd,buffer,255);		//This while loop will be exited when the finish message ("-->") is received.
									if (n < 0)
										error("ERROR reading from socket");
									
									/*If the stack is empty, the top variable will hold the address of the newly created structure.*/
									
									if(strcmp(buffer,finish)==0)
										break;
									if(top == NULL)
									{
										top = new struct stack;
										top ->next = NULL;
									}
									
									/*If the stack is not empty, temp will hold the address of the newly created structure until it's corresponding pointer value
									  is set to top and top's value is changed to this newly created struct.*/
									else
									{
										temp =  new struct stack;
										temp -> next = top;
										top = temp;
										temp = NULL;
									}
									bzero(top->line,256);
									strcpy(top->line,buffer);
								}
							}
							
							/*In this block, all the lines are popped from the stack and copied into the file. This process will reverse the sequence of the lines.*/
							temp = top;
							while(temp != NULL)
							{
								file_w<<temp->line<<endl;
								temp1 = temp;
								temp = temp ->next;
								delete temp1;
							}
							file_w.close();   //The file is closed after this process.
							
							table->set_lock(found_pos, 0);  //The lock is then released (set to 0).
							cout<<"Lock released:  "<<table->check_lock(found_pos)<<endl;
							
						}
						
						//This is the read mode block.
						else
						{
							//the lock value is checked using the check_lock() method.
							locked = table->check_lock(found_pos);
							cout<<"Lock value: "<<locked<<endl;
							int lock_change = 0;
							
							//If the file is write locked, then it cannot be accessed.
							if(locked == 2)
							{
								n = write(newsockfd,"File is locked.",15);
								if (n < 0) 
									error("ERROR writing to socket");
								continue;
							}
							
							//If the file is unlocked, then a read lock is set. 
							//The lock_change variable is used to keep track of whether this fork() set the read lock.
							if(locked == 0)
							{
								lock_change = 1;
								table->set_lock(found_pos, 1);
							}
							
							cout<<"New lock status: "<<table->check_lock(found_pos)<<endl;
							file_r.open(filename);
							
							lines = 0;		//This variable keeps track of the number of lines in the file.
							
							if(file_r.is_open())
							{
								//The transmission is begun with a start ("<--") symbol.
								n = write(newsockfd,start,3);
								if (n < 0) 
									error("ERROR writing to socket");
								system("sleep 0.01");
								cout<<start<<endl;
								
								while(file_r.good())
								{
									getline(file_r,line);
									lines++;
								}
								
								file_r.clear();
								file_r.seekg(0, ios::beg);
								cout<<"Lines = "<<lines<<endl;
								j = 0;		//This variable is used to calculate the progress percentage.
								
								while(file_r.good())
								{
									len = 0;
									prog = 0;
									bzero(line_array,256);
									getline(file_r,line);
									len = line.size();
									for(i=0;i<len;i++)
										line_array[i]=line[i];
									n = write(newsockfd,line_array,len);		//The file is sent line by line.
									if (n < 0) 
										error("ERROR writing to socket");
									
									cout<<"\rIn progress "<<(j*100)/lines<<"%";	//The progress is calculated and updated here.
									j++;
									system("sleep 0.01");			//Without a 10ms sleep, the two programs seem to lose sync.
								}
								cout<<"\rIn progress "<<(j*100)/lines<<"%"<<endl;
								
								cout<<finish<<endl;
								if (n < 0) 
									error("ERROR writing to socket");
								file_r.close();
								
								//The transmission is ended with a finish("-->") symbol.
								n = write(newsockfd,finish,3);
								if (n < 0) 
									error("ERROR writing to socket");
								
								//If this client was the one who set the read lock, then the read lock is released (set to 0).
								if(lock_change == 1)
									table->set_lock(found_pos,0);
								cout<<"Lock released"<<endl;
							}
							else
							{
								printf("File not found..\n");
								n = write(newsockfd,"File not found.",15);
								if (n < 0) 
									error("ERROR writing to socket");
							}
						}
							
					
					}
					
					//Now the server asks the client if it has any more queries.
					bzero(buffer,256);
					n = write(newsockfd,"Server: Continue? :[Y/N]",24);
					if (n < 0) 
						error("ERROR writing to socket");
					cout<<"Server: Continue? :[Y/N]"<<endl;
					
					//The client response is read.
					n = read(newsockfd,buffer,255);
					if (n < 0)
						error("ERROR reading from socket");
					y_or_n = buffer[0];
					cout<<"Client: "<<y_or_n<<endl;
					
					//If the response is yes, then another iteration of this infinite while loop is performed.
					if(y_or_n == 'y' || y_or_n == 'Y')
					{
						cout<<"----------------------------------------"<<endl;
					}
					
					//If the response is no, then the connection is killed and the the active_connections shared variable is decremented by 1.
					else
					{
						*active_connections = *active_connections -1;
						printf("Kill command received\n");
						printf("----------------------------------------\n");
						cout<<"New act_conn = "<<*active_connections<<endl;
						break;
					}
				}
				close(newsockfd);
			}
			else
			{
				
			}
			
		}

		//The parent will close the socket.
		

	}

//close connections using file descriptors
     
	 
     close(sockfd);

     return 0;
}
