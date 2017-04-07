all: server files client

server: server.c
	mkdir server_dir
	mkdir client_dir
	g++ server.c -o server
	mv server server_dir

client: client.c
	g++ client.c -o client
	mv client client_dir

files:
	cd test_files; mv test_input1.txt test_input2.txt test_input3.txt ..;
	mv test_input1.txt test_input2.txt test_input3.txt client_dir
	cd test_files; mv test1.txt test2.txt test3.txt test4.txt test5.txt citizen5.txt ..;
	mv test1.txt test2.txt test3.txt test4.txt test5.txt citizen5.txt server_dir
	clear

clean:
	cd client_dir; mv test_input1.txt test_input2.txt test_input3.txt ..;
	mv test_input1.txt test_input2.txt test_input3.txt test_files
	cd server_dir; mv test1.txt test2.txt test3.txt test4.txt test5.txt citizen5.txt ..
	mv test1.txt test2.txt test3.txt test4.txt test5.txt citizen5.txt test_files
	rm -r server_dir
	rm -r client_dir
	clear