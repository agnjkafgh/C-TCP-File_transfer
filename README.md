# C-TCP-File_transfer
Server and client code to transfer files over a network using TCP/IP written on C

Names of Authors: Ashwin Natarajan, Chandrakanth Mamillapalli
Course Number and Name: CSE 434, Computer Networks, at Arizona State University
Semester: Fall 2016
Project Part: 2
Time spent: 17 hours

Instructions:
Note: These instructions are meant for usage on a linux based OS.

Automated method:
	1. Use the command "make" (without the quotation marks). This should create two directories client_dir and server_dir.
	2. The executables and the test files will automatically be moved to the respective directories.
	3. Use the command "make clean" (without the quotation marks) to revert the folder to how it was before using make.
 
Manual Method:
	1 Compile the codes using the following commands:
		$ g++ server.c -o server
		$ mkdir server_dir
		$ mv server test1.txt test2.txt test3.txt test4.txt test5.txt citizen5.txt server_dir
		$ g++ client.c -o client
		$ mkdir client_dir
		$ mv client test_input1.txt test_input2.txt test_input3.txt client_dir

	2 The codes for executing the programs are as follows:
		$ ./server <port_number>
		$ ./client <server_hostname> <client_ID> <server_port>
		
	3 Note: These codes should be executed on two different computers or two different terminal windows.
	
Note about write lock:
	For the write mode, the client will first send the command <filename><,><space><'w'>
	Then the server will apply a write lock and for that file.
	Then the client will wait for the user to enter the file name of the file to be sent.
	Upon receiving this input, the client will then send the file line by line.
	The write lock can be tested in this time.
	Or a 6000 line file called citizen5.txt will also be provided.

Credits to our professor: Dr. Kanika Grover
