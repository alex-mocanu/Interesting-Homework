#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <dirent.h>
#include <map>
#include <vector>
#include <sstream>
using namespace std;

const int BUFLEN = 4096;
const int TRANS = 4000;
const int QUEUE_LENGTH = 10;
const int MAX_USERS = 105;

//User data structure for password and shared files
typedef struct {
	string pass;
	vector<string> files;
} User;

void error(string err){
    perror(err.c_str());
    exit(1);
}
//Function for giving a response to login command
bool login(int sockfd, string command, map<int, int> userTry, map<string, User>& users, map<int, string>& userBySocket);

int main(int argc, char* argv[]){
	fd_set read_fds;	//read file descriptors used for select()
    fd_set tmp_fds;	//file descriptors copy
    int fdmax;		//maximum value of read file desciptor
	struct sockaddr_in serv_addr, cli_addr; //server and client socket address structures
	int sockfd, newsockfd;	//Server passive socket and new socket descriptors
	map<int, int> userTry;	//Number of tries of a user
	string userList = "";	//List of users on server
	char buffer[BUFLEN];	//Buffer
	map<string, User> users;	//Mapping of usernames and User structures
	map<int, string> userBySocket;	//Mapping of socket descriptors and usernames
	map<string, FILE*> uploadFiles;	//Mapping of filenames and file descriptors
	map<string, FILE*> downloadFiles;	//Mapping of filenames and file descriptors
	bool quitting = 0;	//Quitting switch
	vector<int> sockets;	//List of sockets
	//Check number of command line arguments
	if(argc < 4)
		error("Not enough arguments");

	//Clear read file descriptors and copy descriptors
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

	#define users_config argv[2]
	#define shared_files argv[3]
	//Open configuring files
	ifstream fuser(users_config);
	ifstream fshared(shared_files);

	//Read user-password mapping
	string user, pass;
	int no_users;
	fuser >> no_users;
	for(int i = 0; i < no_users; i++){
		fuser >> user >> pass;
		users[user].pass = pass;
		if(i == no_users - 1)
			userList += user;
		else
		userList += user + " ";
	}

	//Read shared_files file
	string assoc;
	int no_files;
	fshared >> no_files;
	fshared.ignore(32767, '\n');
	for(int i = 0; i < no_files; i++){
		getline(fshared, assoc);
		int pos = assoc.find(":");
		string user = assoc.substr(0, pos);
		string file = assoc.substr(pos + 1, assoc.length() - pos);

		//Check existence of user
		if(users.find(user) == users.end()){
			printf("Utilizatorul %s inexistent!\n", user.c_str());
			continue;
		}
		//Check existence of file
		char path[100];
		sprintf(path, "%s/%s", user.c_str(), file.c_str());
		if(access(path, F_OK) == -1){
			printf("Fisierul %s/%s inexistent!\n", user.c_str(), file.c_str());
			continue;
		}
		users[user].files.push_back(file);
	}

	//Server socket
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
		error("Opening socket error");

	//Socket address
	serv_addr.sin_family = PF_INET;
	serv_addr.sin_port = htons(atoi(argv[1]));
	serv_addr.sin_addr.s_addr = INADDR_ANY;	//use machine IP address

	//Socket binding
	if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
		error("Binding error");

	//Listen
	if(listen(sockfd, QUEUE_LENGTH) == -1)
		error("Listen error");

	//Add socket file descriptor to read file descriptors
    FD_SET(sockfd, &read_fds);
    fdmax = sockfd;

    //Add standard input to read file descriptors
    FD_SET(0, &read_fds);

    while (1) {
		tmp_fds = read_fds;
		//Mark active sockets
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
			error("ERROR in select");
		//If standard input is active check if there is a quit command
		if(FD_ISSET(0, &tmp_fds)){
			string input;
			getline(cin, input);
			if(input == "quit"){
				quitting = 1;
				//Block input server commands
				FD_CLR(0, &read_fds);
				//If there is no client we close server
				if(sockets.size() == 0){
					FD_CLR(sockfd, &read_fds);
					close(sockfd);
					return 0;
				}
				//Announce clients that server is quitting
				for(int i = 0; i < sockets.size(); i++)
					send(sockets[i], "quit:Server quitting", 20, 0);
			}
		}
		//Server each client request
		for(int i = 1; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
			
				if (i == sockfd) {
					//A new connection is being made
					socklen_t clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
						error("ERROR in accept");
					} 
					else {
						//Add new socket to read file descriptors
						FD_SET(newsockfd, &read_fds);
						sockets.push_back(newsockfd);
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}
					}
					printf("Noua conexiune de la %s, port %d, socket_client %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
				}
					
				else {
					//Data is received on the socket
					memset(buffer, 0, BUFLEN);
					int n;
					//In case of an error
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
						if (n == 0) {
							//conexiunea s-a inchis
							printf("selectserver: socket %d hung up\n", i);
						} else {
							error("ERROR in recv");
						}
						//Close connection and remove socket from read file descriptors
						close(i);
						FD_CLR(i, &read_fds);
						for(int j = 0; j < sockets.size(); j++)
							if(sockets[j] == i){
								sockets.erase(sockets.begin() + j);
								break;
							}
						if(quitting && sockets.empty()){
							close(sockfd);
							FD_CLR(sockfd, &read_fds);
							return 0;
						}
					} 
					//In case of a valid packet
					else {
						string command(buffer, n);

						//Identify command
			            int pos = command.find(":");
			            string instr;
			            if(pos > 0)
			            	instr = command.substr(0, pos);
			            else
			            	instr = command;

			            //Login procedure
			            if(instr == "login" && !quitting){
			            	//If brute-force is detected close connection
			            	if(login(i, command, userTry, users, userBySocket) == 0){
			            		close(i);
			            		FD_CLR(i, &read_fds);
			            	}
			            }

			            //Logout procedure
			            else if(instr == "logout" && !quitting){
			            	//Remove username from mapping
			            	userBySocket.erase(i);
			            }

			            //Getuserlist procedure
			            else if(instr == "getuserlist" && !quitting){
			            	string msg("getuserlist:");
			            	msg += userList;
			            	for(int j = 0; j < userList.size(); j += 4096){
			            		string snd;
			            		if(j + 4096 < userList.size())
			            			snd = msg.substr(j, userList.size() - j + 1);
			            		else
			            			snd = msg.substr(j, 4096);
			            		send(i, snd.c_str(), snd.length(), 0);
			            	}
			            	if(userList.size() % 4096 == 0)
			            		send(i, "", 0, 0);
			            }

			            //Getfilelist procedure
			            else if(instr == "getfilelist" && !quitting){
			            	//Identify directory questioned
			            	string dir_name = command.substr(pos + 1, command.length() - pos - 1);
			            	string res = "";
			            	//If there is no such user return error
			            	if(users.find(dir_name) == users.end())
			            		res = "getfilelist:-11 Utilizator inexistent:";
			            	//Else prepare the filelist
			            	else{
				            	DIR* dir;
				            	//Open directory to find files
								dir = opendir(dir_name.c_str());
								//Create map to identify shared files
								map<string, int> files;
								for(int j = 0; j < users[dir_name].files.size(); j++)
									files[users[dir_name].files[j]] = 1;

								struct dirent* file;
								//Examine each file from directory
								while((file = readdir(dir)) != NULL){
									string filename(file->d_name);
									/*If the file is the current or parent 
									directory indicator ignore it*/
									if(filename == "." || filename == "..")
										continue;
									bool shared = files.find(filename) == files.end() ? 0 : 1;
									filename = dir_name + "/" + filename;
									FILE* f = fopen(filename.c_str(), "r");
									//Find file dimension
									fseek(f, 0, SEEK_END);
									long long size = ftell(f);
									fclose(f);
									char filesize[10];
									sprintf(filesize, "%lld", size);
									res += filename + "\t" + string(filesize) + " bytes\t" + (shared == 1 ? "SHARED:" : "PRIVATE:");
								}
								res = "getfilelist:" + res;
							}
							for(int j = 0; j < res.length(); j += 4096){
								string snd;
								if(j + 4096 < res.length())
									snd = res.substr(j, res.length() - j + 1);
								else
									snd = res.substr(j, 4096);
								send(i, snd.c_str(), snd.length() + 1, 0);
							}
			            }

			            //Upload procedure
			            else if(instr == "upload" && !quitting){
			            	//Identify name of the file to be uploaded
			            	string file = command.substr(pos + 1, command.length() - pos - 1);
			            	string completeFile = userBySocket[i] + "/" + file;
			            	string msg = command;
			            	//Check if file is already there
			            	if(access(completeFile.c_str(), F_OK) != -1)
			            		msg += ":-9 Fisier deja prezent";
			            	else{
			            		msg += ":OK";
			            		FILE* f = fopen(completeFile.c_str(), "wb");
					            uploadFiles[completeFile] = f;
			            	}
			            	send(i, msg.c_str(), msg.length(), 0);
			            }

			            //Up procedure
			            else if(instr == "up"){
			            	//Identify file to be uploaded and data to be written
			            	int pos2 = command.find(":", pos + 1);
			            	string file = command.substr(pos + 1, pos2 - pos - 1);
			        		string completeFile = userBySocket[i] + "/" + file;
			        		const char* toWrite = command.substr(BUFLEN - TRANS, command.length() - (BUFLEN - TRANS)).c_str();
			        		int size = command.length() - (BUFLEN - TRANS);
			        		//Write data into uploaded file
			        		fwrite(toWrite, sizeof(char), size, uploadFiles[completeFile]);
			        		//If the transfer is over remove file from uploadedFiles mapping
			        		if(size < TRANS){
			        			fclose(uploadFiles[completeFile]);
			        			uploadFiles.erase(completeFile);
			        		}
			        		//Else send confirmation message to client
			        		else{
			        			string toSend = "up:" + file;
			        			send(i, toSend.c_str(), toSend.length(), 0);
			        		}
			            }

			            //Download procedure
			            else if(instr == "download" && !quitting){
			            	int pos2 = command.find(":", pos + 1);
			            	string user = command.substr(pos + 1, pos2 - pos - 1);
			            	if(user == "@")
			            		user = userBySocket[i];
			            	string file = command.substr(pos2 + 1, command.length() - pos2 - 1);
			            	string completeFile = user + "/" + file;
			            	string msg("download:");
			            	msg += user + ":" + file + ":";
			            	//If user is not found return error
			            	if(users.find(user) == users.end())
			            		msg += "-11 Utilizator inexistent";
			            	//If there the requested file is not present return error
			            	else if(access(completeFile.c_str(), F_OK) == -1)
			            		msg += "-4 Fisier inexistent";
			            	else{
			            		//If the file is being uploaded return error
			            		if(uploadFiles.find(userBySocket[i] + "/" + file) != uploadFiles.end())
			            			msg += "-10 Fisier in transfer";
			            		else{
			            			//If file belongs to other user
				            		if(user != userBySocket[i] && user != "@"){
				            			int j;
				            			for(j = 0; j < users[user].files.size(); j++)
				            				if(users[user].files[j] == file){
				            					msg += "OK";
				            					break;
				            				}
				            			//If file is not shared return error
				            			if(j == users[user].files.size())
				            				msg += "-5 Descarcare interzisa";
				            		}
				            		//If file belongs to current user
				            		else
				            			msg += "OK";
			            		}
			            	}
			            	/*If download request is successful open file to be 
			            	downloaded and add it to downloadFiles map*/
			            	if(msg.find("OK") > 0){
			            		FILE* f = fopen(completeFile.c_str(), "rb");
			            		completeFile = userBySocket[i] + "/" + completeFile;
			            		downloadFiles[completeFile] = f;
			            	}
			            	send(i, msg.c_str(), msg.length(), 0);
			            }

			            //Down procedure
			            else if(instr == "down"){
			            	int pos2 = command.find(":", pos + 1);
			            	string user = command.substr(pos + 1, pos2 - pos - 1);
			            	string file = command.substr(pos2 + 1, command.length() - pos2 - 1);
			            	string completeFile = userBySocket[i] + "/" + user + "/" + file;
			            	//Open file that is downloaded
			            	FILE* f = downloadFiles[completeFile];
			            	//Prepare packet header
			            	string header("down:");
			            	header += user + ":" + file;
			            	header += string(BUFLEN - TRANS - header.length(), ':');
			            	//Read data from file
			            	char snd[BUFLEN] = {0};
			            	int read = fread(snd, sizeof(char), TRANS, f);
			            	char toSend[BUFLEN] = {0};

			            	for(int j = 0; j < header.length(); j++)
			            		toSend[j] = header[j];
			            	for(int j = header.length(); j < header.length() + read; j++)
			            		toSend[j] = snd[j - header.length()];
			            	//Send packet to client
			            	if(send(i, toSend, read + BUFLEN - TRANS, 0) == -1)
			            		error("Send error");
			            	//If transfer is finished remove file from mapping
			            	if(read < TRANS){
			            		fclose(downloadFiles[completeFile]);
			            		downloadFiles.erase(completeFile);
			            	}
			            }

			            //Share procedure
			            else if(instr == "share" && !quitting){
			            	//Identify file to be shared
			            	string file = command.substr(pos + 1, command.length() - pos - 1);
			            	string completeFile = userBySocket[i] + "/" + file;
			            	string msg("share:");
			            	//If file is not present return error
			            	if(access(completeFile.c_str(), F_OK) == -1){
			            		msg += "-4 Fisier inexistent";
			            		send(i, msg.c_str(), msg.length(), 0);
			            		continue;
			            	}
			            	//If file is already shared return error
			            	int j;
			            	int size = users[userBySocket[i]].files.size();
			            	for(j = 0; j < size; j++)
			            		if(users[userBySocket[i]].files[j] == file){
			            			msg += "-6 Fisier deja partajat";
			            			break;
			            		}
			            	/*If request is successful add file to list of 
			            	shared files*/
			            	if(j == size){
			            		users[userBySocket[i]].files.push_back(file);
			            		msg += "200 Fisierul " + file + " a fost partajat.";
			            	}

			            	send(i, msg.c_str(), msg.length(), 0);
			            }

			            //Unshare procedure
			            else if(instr == "unshare" && !quitting){
			            	//Identify file to be unshared
			            	string file = command.substr(pos + 1, command.length() - pos - 1);
			            	string completeFile = userBySocket[i] + "/" + file;
			            	string msg("unshare:");
			            	//If file is not present return error
			            	if(access(completeFile.c_str(), F_OK) == -1){
			            		msg += "-4 Fisier inexistent";
			            		send(i, msg.c_str(), msg.length(), 0);
			            		continue;
			            	}
			            	//Remove file from list of shared files
			            	int j;
			            	int size = users[userBySocket[i]].files.size();
			            	for(j = 0; j < size; j++)
			            		if(users[userBySocket[i]].files[j] == file){
			            			users[userBySocket[i]].files.erase(users[userBySocket[i]].files.begin() + j);
			            			msg += "200 Fisierul " + file + " a fost setat ca PRIVATE.";
			            			break;
			            		}
			            	//If file is not shared return error
			            	if(j == size)
			            		msg += "-7 Fisier deja privat";

			            	send(i, msg.c_str(), msg.length(), 0);
			            }

			            //Delete procedure
			            else if(instr == "delete" && !quitting){
			            	//Identify file to be deleted
			            	string file = command.substr(pos + 1, command.length() - pos - 1);
			            	string completeFile = userBySocket[i] + "/" + file;
			            	string msg("delete:");
			            	//If file is not present return error
			            	if(access(completeFile.c_str(), F_OK) == -1)
			            		msg += "-4 Fisier inexistent";
			            	//If file is being transfered return error
			            	else if(uploadFiles.find(completeFile) != uploadFiles.end())
			            		msg += "-10 Fisier in transfer";
			            	else if(downloadFiles.find(completeFile) != downloadFiles.end())
			            		msg += "-10 Fisier in transfer";
			            	//Else remove file from user's directory
			            	else{
			            		unlink(completeFile.c_str());
			            		for(int j = 0; j < users[userBySocket[i]].files.size(); j++)
			            			if(users[userBySocket[i]].files[j] == file){
			            				users[userBySocket[i]].files.erase(users[userBySocket[i]].files.begin() + j);
			            				break;
			            			}
			            		msg += "200 Fisierul " + file + " a fost sters.";
			            	}

			            	send(i, msg.c_str(), msg.length(), 0);
			            }

			            //Client quitting
			            else if(instr == "quit" && !quitting){
			            }
					}
				} 
			}
		}
    }
	return 0;
}

bool login(int sockfd, string command, map<int, int> userTry, map<string, User>& users, map<int, string>& userBySocket){
	int pos1 = command.find(":");
	int pos2 = command.find(":", pos1 + 1);
	string user = command.substr(pos1 + 1, pos2 - pos1 - 1);
	string pass = command.substr(pos2 + 1, command.length() - pos2 - 1);
	string msg = "login:" + user + ":" + pass + ":";
	//If user or password is incorrect return error
	if(users.find(user) == users.end() || users[user].pass != pass){
		if(userTry[sockfd] < 2){
			msg += "-3 User/parola inexistent";
			send(sockfd, msg.c_str(), msg.length(), 0);
			userTry[sockfd]++;
		}
		/*If client reached 3 consecutive unsuccessful tries brute-force is 
		identified*/
		else{
			userTry[sockfd] = 0;
			msg += "-8 Brute-force detectat";
			send(sockfd, msg.c_str(), msg.length(), 0);
			return 0;
		}
	}
	//Else clear user tries and associate username to socket
	else{
		userTry[sockfd] = 0;
		userBySocket[sockfd] = user;
		msg += "OK";
		send(sockfd, msg.c_str(), msg.length(), 0);
	}

	return 1;
}