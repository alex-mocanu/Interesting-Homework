#include "utils.h"

queue<int> freeUsers; //Queue of users ready for a download
queue<string> tasks; //Queue of downloads to be completed
map<int, pair<string, int> > clients; //Address and port according to socket
map<string, FILE*> files; //File pointers according to filename
map<string, bool> sites; //Hash for checking if a site was already visited

//Download function
void download(fd_set &read_fds, int fdmax, int sockfd, string everything, 
	FILE* out, FILE* err);

//Function for creating directory and opening file for writing
void createDir(string task);

int main(int argc, char* argv[]){
	int sockfd, newsockfd, clilen;
	struct sockaddr_in serv_addr, cli_addr;
	char buffer[BUFLEN], msg[BUFLEN];
	bool needToDownload = 0; //Variable telling if a download command was given
	FILE *out = NULL, *err = NULL;
	string logFile, arguments = "", recursive = "1", everything = "0";
	int port;

	fd_set read_fds, tmp_fds;
	int fdmax;

	//Make configurations according to command-line arguments
	for(int i = 1; i < argc; ++i)
		arguments += string(argv[i]) + " ";

	if(arguments.find("-o") != -1){
		stringstream ss(arguments.substr(arguments.find("-o") + 2));
		ss >> logFile;
		out = fopen((logFile + ".stdout").c_str(), "w");
		err = fopen((logFile + ".stderr").c_str(), "w");
	}
	
	if(arguments.find("-p") == -1)
		error("No port specified\n", err);
	stringstream ss(arguments.substr(arguments.find("-p") + 2));
	ss >> port;
	if(port == 0)
		error("Wrong port number\n", err);

	if(arguments.find("-r") != -1)
		recursive = "5";
	if(arguments.find("-e") != -1)
		everything = "1";

	//Open socket
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
		error("Opening socket error\n", err);

	//Initialize server socket structure
	serv_addr.sin_family = PF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	//Binding socket
	if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(struct sockaddr)) == -1)
		error("Binding error\n", err);

	//Listening
	if(listen(sockfd, MAX_CLIENTS) == -1)
		error("Listening error\n", err);

	//Add sockfd to read descriptors
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;

	//Add standard input to read descriptors
	FD_SET(0, &read_fds);

	while(1){
		tmp_fds = read_fds;
		//Select active sockets
		if(select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1){
			print("Select error\n", err);
			continue;
		}

		int i;
		for(i = 0; i <= fdmax; ++i)
			if(FD_ISSET(i, &tmp_fds)){
				//A connection request has arrived
				if(i == sockfd){
					socklen_t clilen = sizeof(cli_addr);
					if((newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr,
					 &clilen)) == -1){
						print("Accept error\n", err);
						continue;
					}
					else{
						FD_SET(newsockfd, &read_fds);
						if(newsockfd > fdmax)
							fdmax = newsockfd;
					}
					sprintf(msg, "New connection from %s, port %d, socket_client %d\n",
						inet_ntoa(cli_addr.sin_addr), 
					 	ntohs(cli_addr.sin_port), newsockfd);
					
					print(msg, out);
					clients[newsockfd] = make_pair(string(
						inet_ntoa(cli_addr.sin_addr)), ntohs(cli_addr.sin_port));
					freeUsers.push(newsockfd);
					if(freeUsers.size() >= 5 && needToDownload){
						download(read_fds, fdmax, sockfd, everything, out, err);
						cout << "Server exits\n";
						print("Server exits\n\n", out);
						break;
					}
				}

				//Command from command-line
				else if(i == 0){
					string command;
					getline(cin, command);
					//Status command
					if(command == "status"){
						if(clients.size() == 0)
							cout << "No clients connected\n";

						//Print list of clients
						for(map<int, pair<string, int> >::iterator it = 
							clients.begin(); it != clients.end(); ++it){
							sprintf(msg, "%s %d", it->second.first.c_str(), 
								it->second.second);
							cout << msg << "\n";
						}
					}
					//Exit command
					else if(command == "exit"){
						cout << "Server exits\n";
						print("Server exits\n\n", out);
						//Finish program if there are no clients
						if(clients.size() == 0){
							close(sockfd);
							if(out){
								fclose(out);
								fclose(err);
							}
							return 0;
						}
						//Notify the clients that the server is quitting
						for(map<int, pair<string, int> >::iterator it = 
							clients.begin(); it != clients.end(); ++it){
							string toSend = "exit" + string(BUFLEN - 4, ' ');
							if(send(it->first, toSend.c_str(), toSend.length(), 
								0) == -1)
								error("Send error\n", err);
						}
					}
					//Download command or invalid command
					else{
						int pos = command.find(" ");
						if(pos == -1 || command.substr(0, pos) != "download"){
							cout << "Invalid command\n";
							print("Invalid command\n", err);
							continue;
						}
						//Download command
						needToDownload = 1;
						tasks.push(recursive + " " + everything + " " + command);
						//Start download if there are at least five clients
						if(freeUsers.size() >= 5){
							download(read_fds, fdmax, sockfd, everything, out, err);
							//Exit after finishing download
							cout << "Server exits\n";
							print("Server exits\n\n", out);
							break;
						}
					}
				}

				else{
					memset(buffer, 0, BUFLEN);
					int n;
					//Connection closed
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0){
						close(i); 
						FD_CLR(i, &read_fds); // remove socket from read descriptors
						clients.erase(i);
						if(clients.size() == 0){
							close(sockfd);
							if(out){
								fclose(out);
								fclose(err);
							}
							return 0;
						}

						if (n == 0){
							sprintf(msg, "selectserver: socket %d hung up\n", i);
							print(msg, out);
						}
						else{
							print("Receive error\n", err);
							continue;
						}
					}
				}
			}

			//Finish program
			if(i <= fdmax){
				for(map<string, FILE*>::iterator it = files.begin(); 
					it != files.end(); ++it)
					fclose(it->second);
				close(sockfd);
				break;
			}
	}
	return 0;
}

void download(fd_set &read_fds, int fdmax, int sockfd, string everything, 
	FILE* out, FILE* err){
	fd_set tmp_fds;
	char buffer[BUFLEN], msg[BUFLEN];
	int working = 1; //Number of working clients
	int newsockfd;
	struct sockaddr_in cli_addr;

	//Assign first link to a client
	while(1){
		string task = tasks.front();
		tasks.pop();
		int user = freeUsers.front();
		freeUsers.pop();

		string toSend = task + string(BUFLEN - task.length(), ' ');
		if(send(user, toSend.c_str(), toSend.length(), 0) == -1){
			tasks.push(task);
			freeUsers.push(user);
			print("Send error\n", err);
			continue;
		}
		//Create directory and open file for writing
		createDir(task);

		char usr[2];
		sprintf(usr, "%d", user);
		print("Socket " + string(usr) + ": " + task + "\n", out);
		break;
	}

	while(1){
		//If there are no clients working and no more tasks, finish program
		if(working == 0 && tasks.empty()){
			for(map<int, pair<string, int> >::iterator it = 
				clients.begin(); it != clients.end(); ++it){
				string toSend = "exit" + string(BUFLEN - 4, ' ');
				if(send(it->first, toSend.c_str(), toSend.length(), 0) == -1)
					error("Send error\n", err);
			}
			break;
		}
		//Assign links to clients
		while(!freeUsers.empty() && !tasks.empty()){
			string task = tasks.front();
			tasks.pop();
			int user = freeUsers.front();
			freeUsers.pop();

			string toSend = task + string(BUFLEN - task.length(), ' ');
			if(send(user, toSend.c_str(), toSend.length(), 0) == -1){
				tasks.push(task);
				freeUsers.push(user);
				print("Send error\n", err);
				continue;
			}
			createDir(task);
			++working;

			char usr[2];
			sprintf(usr, "%d", user);
			print("Socket " + string(usr) + ": " + task + "\n", out);
		}

		tmp_fds = read_fds;
		if(select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1){
			print("Select error\n", err);
			continue;
		}

		for(int i = 1; i <= fdmax; ++i)
			if(FD_ISSET(i, &tmp_fds)){
				//A connection request has arrived
				if(i == sockfd){
					socklen_t clilen = sizeof(cli_addr);
					if((newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr,
					 &clilen)) == -1){
						print("Accept error\n", err);
						continue;
					}
					else{
						FD_SET(newsockfd, &read_fds);
						if(newsockfd > fdmax)
							fdmax = newsockfd;
					}
					sprintf(msg, "New connection from %s, port %d, socket_client %d\n",
						inet_ntoa(cli_addr.sin_addr), 
					 	ntohs(cli_addr.sin_port), newsockfd);
					
					print(msg, out);
					clients[newsockfd] = make_pair(string(
						inet_ntoa(cli_addr.sin_addr)), ntohs(cli_addr.sin_port));
					freeUsers.push(newsockfd);
				}

				memset(buffer, 0, BUFLEN);
				int n;
				//Connection closed
				if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0){
					if (n == 0){
						sprintf(msg, "selectserver: socket %d hung up\n", i);
						print(msg, out);
					}
					else{
						print("Receive error\n", err);
						continue;
					}
					close(i); 
					FD_CLR(i, &read_fds); // remove socket from read descriptors
					clients.erase(i);
				}

				//Data from client
				else{
					string rec(buffer, n);
					int pos1 = rec.find(" ");
					string type;
					type = rec.substr(0, pos1); //message type

					//Client sent files
					if(type == "data"){
						int pos2 = rec.find(" ", pos1 + 1);
						string fileName = rec.substr(pos1 + 1, pos2 - pos1 - 1);
						string content = rec.substr(pos2 + 1); //file content

						//If we finished sending the file
						if(content.find("OVER") == 0){
							fclose(files[fileName]);
							files.erase(fileName);
							continue;
						}

						//Determine the size of the effective information
						int pos3 = rec.find(" ", pos2 + 1);
						int dimension = atoi(rec.substr(pos2 + 1, pos3 - pos2 - 
							1).c_str());
						content = rec.substr(pos3 + 1, dimension);
						//Write content to file
						fwrite(content.c_str(), sizeof(char), content.length(), 
							files[fileName]);
					}

					//Client sent a list of links
					if(type == "link"){
						int pos2 = rec.find(" ", pos1 + 1);
						string recur = rec.substr(pos1 + 1, pos2 - pos1 - 1); //recursion level
						int pos3 = rec.find(" ", pos2 + 1);
						string site = rec.substr(7, pos3 - 7);
						//If the site hasn't been visited, add it to the tasks
						if(!sites[site]){
							sites[site] = 1;
							tasks.push(recur + " " + everything + 
								" download http://" + site);
						}
					}

					//If the task is done the user is free for another task
					if(type == "finished"){
						freeUsers.push(i);
						--working;
					}
				}
			}
	}
}

void createDir(string task){
	task = task.substr(task.find("/") + 2);
	sites[task] = 1; //Add site to visited sites map
	int pos = task.find_last_of("/");
	string dir = task.substr(0, pos);
	//Create hierarchy of directories if there isn't one
	struct stat st;
	if(stat(dir.c_str(), &st)){
		string comm = "mkdir -p " + dir;
		system(comm.c_str());
	}

	FILE* f = fopen(task.c_str(), "wb");
	files[task] = f;
}