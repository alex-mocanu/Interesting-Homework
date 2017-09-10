#include "utils.h"

//Download function
void download(string site, int recur, int every, int sockfd, FILE* err);

int main(int argc, char* argv[]){
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostent* page;
	char buffer[BUFLEN];
	FILE *out = NULL, *err = NULL;
	string arguments = "", server, logFile;
	int port;

	fd_set read_fds, tmp_fds;

	//Make configurations according to command-line arguments
	for(int i = 1; i < argc; ++i)
		arguments += string(argv[i]) + " ";

	if(arguments.find("-o") != -1){
		stringstream sso(arguments.substr(arguments.find("-o") + 2));
		sso >> logFile;

		int PID = getpid(); //Get process pid to personalize log files names
		char pid[10];
		sprintf(pid, "%d", PID);
		out = fopen((logFile + string(pid) + ".stdout").c_str(), "w");
		err = fopen((logFile + string(pid) + ".stderr").c_str(), "w");
	}

	if(arguments.find("-a") == -1)
		error("No address specified\n", err);
	stringstream ssa(arguments.substr(arguments.find("-a") + 2));
	ssa >> server;

	if(arguments.find("-p") == -1)
		error("No port specified\n", err);
	stringstream ssp(arguments.substr(arguments.find("-p") + 2));
	ssp >> port;
	if(port == 0)
		error("Wrong port number\n", err);

	//Open socket
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
		error("Opening socket error\n", err);

	//Initialize server socket structure
	serv_addr.sin_family = PF_INET;
	serv_addr.sin_port = htons(port);
	if(inet_aton(server.c_str(), &serv_addr.sin_addr) == 0)
		error("Address conversion error\n", err);

	FD_SET(sockfd, &read_fds);

	//Connect
	if(connect(sockfd, 	(struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
		error("Connection error\n", err);

	while(1){
		tmp_fds = read_fds;
		//Select active sockets
		if(select(sockfd + 1, &tmp_fds, NULL, NULL, NULL) == -1){
			print("Select error\n", err);
			return 0;
		}

		//Command received from server
		if(FD_ISSET(sockfd, &tmp_fds)){
			memset(buffer, 0, BUFLEN);
			int n;
			//Receive error
			if((n = recv(sockfd, buffer, BUFLEN, 0)) <= 0){
				close(sockfd);
				print("Receive error\n", err);
				continue;
			}
			//Valid command
			else{
				string command = string(buffer, n);
				int pos1 = command.find(" ");
				//Exit command
				if(command.substr(0, pos1) == "exit"){
					print("Client exiting\n", out);
					close(sockfd);
					break;
				}
				//Download command
				else{
					int recur = atoi(command.substr(0, pos1).c_str());
					int pos2 = command.find(" ", pos1 + 1);
					int every = atoi(command.substr(pos1 + 1, pos2 - pos1).c_str());
					int pos3 = command.find("/");
					int pos4 = command.find(" ", pos3);
					string site = command.substr(pos3 + 2, pos4 - pos3 - 1);

					string toPrint = "Downloading " + site + "\n";
					print(toPrint, out);
					download(site, recur, every, sockfd, err);
				}
			}
		}
	}

	if(out){
		fclose(out);
		fclose(err);
	}
	return 0;
}

void download(string site, int recur, int every, int sockfd, FILE* err){
	--recur; //Decrease recursion level
	char buffer[BUFLEN];
	struct hostent *serv_addr;
	struct in_addr* addr;
	int pos = site.find("/");
	string path = site.substr(pos); //Path of the file to be downloaded
	site = site.substr(0, pos); //Name of the website
	string toSend;
	//Determine website address using its name
	serv_addr = gethostbyname(site.c_str());
	addr = (in_addr*)serv_addr->h_addr;
	char *res = inet_ntoa(*addr);
	//Open socket for communication with the site's server
	int sitesockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sitesockfd == -1){
		print("Socket opening error\n", err);
		return;
	}

	struct sockaddr_in server;
	server.sin_family = PF_INET;
	server.sin_port = htons(PORT);
	if(inet_aton(res, &server.sin_addr) == 0){
		print("Address conversion error\n", err);
		return;
	}

	if(connect(sitesockfd, (struct sockaddr*) &server, sizeof(server)) < 0){
		print("Connect error\n", err);
        return;
	}

	//Send get request to the server to obtain the html page content
    string snd = "GET " + path + " HTTP/1.0\r\n\r\n";
    if(send(sitesockfd, snd.c_str(), snd.length(), 0) == -1){
    	print("Send error\n", err);
    	return;
    }

    //Construct page from pieces
    string page;
    int n;
    while((n = recv(sitesockfd, buffer, sizeof(buffer), 0)) > 0)
    	page += string(buffer, n);
    //Remove GET header
    page = page.substr(page.find("\r\n\r\n") + 4);

    //Form message header
    string header = "data " + site + path;
    int SIZE = BUFLEN - header.length() - 5;
    //Send file chunks with the header above and chunk's size
    for(int i = 0; i < page.length(); i += SIZE){
    	int dim = min(SIZE, (int)page.length() - i);
    	char dimension[4];
    	sprintf(dimension, "%d", dim);
    	toSend = header + string(dimension) + " " + page.substr(i, dim);
    	toSend += string(BUFLEN - toSend.length(), ' '); //pad with spaces
    	if(send(sockfd, toSend.c_str(), toSend.length(), 0) == -1){
    		print("Send error\n", err);
    		return;
    	}
    }
    //Send message marking end of file transfer
	toSend = header + "OVER" + string(SIZE + 1, ' ');
	if(send(sockfd, toSend.c_str(), toSend.length(), 0) == -1){
		print("Send error\n", err);
		return;
	}

	//If we are not at the final step of recursivity
	if(recur){
		//Find links
		int pos1 = page.find("<a") + 1;
		while(pos1 != 0){
			//Find label end
			int pos2 = page.find(">", pos1);
			//Check if there is any link
			int pos3 = page.find("href=\"", pos1);

			//If there is a href
			if(pos3 < pos2 && pos3 != -1){
				pos3 += 6;
				//Find end of link
				int pos = page.find("\"", pos3);
				string link = page.substr(pos3, pos - pos3);

				//Check that link refers to same server
				if(link.find("http://") != -1 || link.find("https://") != -1){
					pos1 = page.find("<a", pos1) + 1;
					continue;
				}

				//Find directory the link is in
				string dir = site + path;
				dir = dir.substr(0, dir.find_last_of("/"));
				pos = link.find("../") + 3;
				while(pos != 2){
					//Bring current directory to its parent
					dir = dir.substr(0, dir.find_last_of("/"));
					link = link.substr(pos);
					pos = link.find("../") + 3;
				}
				dir += "/" + link;

				//Check if link is html or htm
				pos = dir.find_last_of(".") + 1;
				string extension = dir.substr(pos);
				if(extension == "html" || extension == "htm"){
					char recurs[1];
					sprintf(recurs, "%d", recur);
					toSend = "link " + string(recurs) + " " + dir;
					toSend += string(BUFLEN - toSend.length(), ' ');
					if(send(sockfd, toSend.c_str(), toSend.length(), 0) == -1){
						print("Send error\n", err);
						continue;
					}
				}

				//If everything option is on
				else if(every && (extension.length() == 3 || extension.length() == 4)){
					char recurs[1];
					sprintf(recurs, "%d", recur);
					toSend = "link " + string(recurs) + " " + dir;
					toSend += string(BUFLEN - toSend.length(), ' ');
					if(send(sockfd, toSend.c_str(), toSend.length(), 0) == -1){
						print("Send error\n", err);
						continue;
					}
				}
			}
			pos1 = page.find("<a", pos1) + 1;
		}
	}

	//If everything is active but not recursive
	else if(every){
		//Find links
		int pos1 = page.find("<a") + 1;
		while(pos1 != 0){
			//Find label end
			int pos2 = page.find(">", pos1);
			//Check if there is any link
			int pos3 = page.find("href=\"", pos1);

			//If there is a href
			if(pos3 < pos2 && pos3 != -1){
				pos3 += 6;
				//Find end of link
				int pos = page.find("\"", pos3);
				string link = page.substr(pos3, pos - pos3);

				//Check that link refers to same server
				if(link.find("http://") != -1 || link.find("https://") != -1){
					pos1 = page.find("<a", pos1) + 1;
					continue;
				}

				//Find directory the link is in
				string dir = site + path;
				dir = dir.substr(0, dir.find_last_of("/"));
				pos = link.find("../") + 3;
				while(pos != 2){
					//Bring current directory to its parent
					dir = dir.substr(0, dir.find_last_of("/"));
					link = link.substr(pos);
					pos = link.find("../") + 3;
				}
				dir += "/" + link;

				//Check if link is html or htm
				pos = dir.find_last_of(".") + 1;
				string extension = dir.substr(pos);

				//If extension is valid
				if((extension.length() == 3 || extension.length() == 4) && 
					extension != "html" && extension != "htm"){
					char recurs[1];
					sprintf(recurs, "%d", recur);
					toSend = "link " + string(recurs) + " " + dir;
					toSend += string(BUFLEN - toSend.length(), ' ');
					if(send(sockfd, toSend.c_str(), toSend.length(), 0) == -1){
						print("Send error\n", err);
						continue;
					}
				}
			}
			pos1 = page.find("<a", pos1) + 1;
		}
	}

	//Signal the end of the download task
	toSend = "finished" + string(BUFLEN - 8, ' ');
	if(send(sockfd, toSend.c_str(), toSend.length(), 0) == -1){
		print("Send error\n", err);
		return;
	}
	//Close communication with the website
	close(sitesockfd);
}