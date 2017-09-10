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
#include <map>
#include <vector>
#include <sstream>
using namespace std;

const int BUFLEN = 4096;
const int TRANS = 4000;

void error(string err){
    perror(err.c_str());
    exit(1);
}

//Function for upload process
void up(int sockfd, string& command, map<string, FILE*>& uploadFiles, string& prompt, ofstream& flog);

int main(int argc, char* argv[]){
	fd_set read_fds;    //read file descriptors used for select()
    fd_set tmp_fds;    //file descriptors copy
    struct sockaddr_in serv_addr;   //server socket address structure
    bool loggedIn = 0, quitting = 0;    //indicators for login and quit
    string prompt("$ ");    //user's prompt
    string loggedUser("");  //name of logged user
    char rec[BUFLEN], snd[BUFLEN];  //receive and send buffers
    map<string, FILE*> uploadFiles; //map for upload files and their FILE*
    map<string, FILE*> downloadFiles;   //map for download files and their FILE*

    //Check command line arguments
    if(argc < 3)
        error("Not enough arguments");

    //Show prompt at opening of client
    cout << prompt;
    cout.flush();

	//Log file
	char logname[20];
    int PID = getpid(); //Process PID
	sprintf(logname, "client-%d.log", PID);
	ofstream flog(logname);

	//Client socket
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
		error("Opening socket error");

	//Server address
	serv_addr.sin_family = PF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	if(inet_aton(argv[1], &serv_addr.sin_addr) == 0)
		error("Inet_aton error");

	//Connect
	if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
		error("Connect error");

	//Add socket descriptor and input stream to list of descriptors
	FD_SET(sockfd, &read_fds);
    FD_SET(0, &read_fds);

	while(1){
        tmp_fds = read_fds;
        //Mark active sockets
        if (select(sockfd + 1, &tmp_fds, NULL, NULL, NULL) == -1)
            error("ERROR in select");

        //Send command to server
        if(FD_ISSET(0, &tmp_fds)){
            //Read from stdin
            string command;
            getline(cin, command);

            //Identify command
            int pos = command.find(" ");
            string instr;
            if(pos > 0)
            	instr = command.substr(0, pos);
            else
            	instr = command;

            //Login procedure
            if(instr == "login"){
                //If user is logged in print error message
                if(loggedIn){
                    cout << "-2 Sesiune deja deschisa\n";
                    flog << prompt << command << "\n" << "-2 Sesiune deja deschisa\n\n";

                    cout << prompt;
                    cout.flush();
                    continue;
                }
                //Identify command arguments
                int pos1 = command.find(" ");
                int pos2 = command.find(" ", pos1 + 1);
                string user = command.substr(pos1 + 1, pos2 - pos1 - 1);
                string pass = command.substr(pos2 + 1, command.length() - pos2 - 1);
                string msg = "login:" + user + ":" + pass;
                //Send login message to server
                send(sockfd, msg.c_str(), msg.length(), 0);
            }

            //Logout procedure
            else if(instr == "logout"){
                //If user is not logged in print error message
                if(loggedIn == 0){
                    cout << "-1 Clientul nu e autentificat\n";
                    flog << prompt << command << "\n" << "-1 Clientul nu e autentificat\n\n";
                }
                //Else change user prompt and announce the server of the logout
                else{
                    flog << prompt << command << "\n\n";
                    loggedIn = 0;
                    loggedUser = "";
                    prompt = "$ ";
                    send(sockfd, "logout:", 7, 0);
                }

                cout << prompt;
                cout.flush();
            }

            //Getuserlist procedure
            else if(instr == "getuserlist"){
                //If user is not logged in print error message
                if(loggedIn == 0){
                    cout << "-1 Clientul nu e autentificat\n";
                    flog << prompt << command << "\n" << "-1 Clientul nu e autentificat\n\n";

                    cout << prompt;
                    cout.flush();
                }
                //Else request the server the userlist
                else{
                    flog << prompt << command << "\n";
                    send(sockfd, "getuserlist:", 12, 0);
                }
            }

            //Getfilelist procedure
            else if(instr == "getfilelist"){
                //If user is not logged in print error message
                if(loggedIn == 0){
                    cout << "-1 Clientul nu e autentificat\n";
                    flog << prompt << command << "\n" << "-1 Clientul nu e autentificat\n\n";

                    cout << prompt;
                    cout.flush();
                }
                //Else request the server the filelist
                else{
                    flog << prompt << command << "\n";
                    string user = command.substr(pos + 1, command.length() - pos - 1);
                    string msg = "getfilelist:" + user;
                    send(sockfd, msg.c_str(), msg.length(), 0);
                }
            }

            //Upload procedure
            else if(instr == "upload"){
                //If user is not logged in print error message
                if(loggedIn == 0){
                    cout << "-1 Clientul nu e autentificat\n";
                    flog << prompt << command << "\n" << "-1 Clientul nu e autentificat\n\n";

                    cout << prompt;
                    cout.flush();
                }
                //Else verify if you can send file and start sending it if you can
                else{
                    int pos = command.find(" ");
                    string file = command.substr(pos + 1, command.length() - pos - 1);
                    string completeFile = "Users/" + loggedUser + "/" + file;
                    //If there is no such file print error message
                    if(access(completeFile.c_str(), F_OK) == -1){
                        cout << "-4 Fisier inexistent\n";
                        flog << prompt << command << "\n" << "-4 Fisier inexistent\n\n";

                        cout << prompt;
                        cout.flush();
                        continue;
                    }
                    //If the file is downloading print error message
                    else if(downloadFiles.find(file) != downloadFiles.end()){
                        cout << "-10 Fisier in transfer\n";
                        flog << prompt << command << "\n" << "-10 Fisier in transfer\n\n";

                        cout << prompt;
                        cout.flush();
                        continue;
                    }
                    //Else ask server if you can send file
                    flog << prompt << command << "\n";
                    string msg = "upload:" + file;
                    send(sockfd, msg.c_str(), msg.length(), 0);
                }
            }

            //Download procedure
            else if(instr == "download"){
                //If user is not logged in print error message
                if(loggedIn == 0){
                    cout << "-1 Clientul nu e autentificat\n";
                    flog << prompt << command << "\n" << "-1 Clientul nu e autentificat\n\n";

                    cout << prompt;
                    cout.flush();
                }
                //Else check if you can download file and download it if you can
                else{
                    int pos2 = command.find(" ", pos + 1);
                    string user = command.substr(pos + 1, pos2 - pos - 1);
                    string file = command.substr(pos2 + 1, command.length() - pos2 - 1);
                    string msg("download:");
                    msg += user + ":" + file;

                    flog << prompt << command << "\n";

                    char pid[5];
                    sprintf(pid, "%d", PID);
                    string downFile = string(pid) + "_" + file;
                    //If file is already downloading print error message
                    if(downloadFiles.find(downFile) != downloadFiles.end()){
                        cout << "-10 Fisier in transfer\n";
                        flog << "-10 Fisier in transfer\n\n";

                        cout << prompt;
                        cout.flush();
                    }
                    //Else ask server if it can send file
                    else
                        send(sockfd, msg.c_str(), msg.length(), 0);
                }
            }

            //Share procedure
            else if(instr == "share"){
                //If user is not logged in print error message
                if(loggedIn == 0){
                    cout << "-1 Clientul nu e autentificat\n";
                    flog << prompt << command << "\n" << "-1 Clientul nu e autentificat\n\n";
                    
                    cout << prompt;
                    cout.flush();
                }
                //Else tell the server you want to share the file
                else{
                    string file = command.substr(pos + 1, command.length() - pos - 1);
                    string msg("share:");
                    msg += file;

                    flog << prompt << command << "\n";
                    send(sockfd, msg.c_str(), msg.length(), 0);
                }
            }

            //Unshare procedure
            else if(instr == "unshare"){
                //If user is not logged in print error message
                if(loggedIn == 0){
                    cout << "-1 Clientul nu e autentificat\n";
                    flog << prompt << command << "\n" << "-1 Clientul nu e autentificat\n\n";
                    
                    cout << prompt;
                    cout.flush();
                }
                //Else tell the server you want to unshare the file
                else{
                    string file = command.substr(pos + 1, command.length() - pos - 1);
                    string msg("unshare:");
                    msg += file;

                    flog << prompt << command << "\n";
                    send(sockfd, msg.c_str(), msg.length(), 0);
                }
            }

            //Delete procedure
            else if(instr == "delete"){
                //If user is not logged in print error message
                if(loggedIn == 0){
                    cout << "-1 Clientul nu e autentificat\n";
                    flog << prompt << command << "\n" << "-1 Clientul nu e autentificat\n\n";
                    
                    cout << prompt;
                    cout.flush();
                }
                //Else tell the server you want to delete the file
                else{
                    string file = command.substr(pos + 1, command.length() - pos - 1);
                    string msg("delete:");
                    msg += file;

                    flog << prompt << command << "\n";
                    send(sockfd, msg.c_str(), msg.length(), 0);
                }
            }

            //Quit procedure
            else if(instr == "quit"){
                //Block command line and tell the server you are quitting
                flog << prompt << command << "\n";
                FD_CLR(0, &read_fds);
                quitting = 1;
                send(sockfd, "quit:Client quitting", 20, 0);
                //If there are no ungoing transfers close connection to server
                if(uploadFiles.empty() && downloadFiles.empty()){
                    FD_CLR(sockfd, &read_fds);
                    close(sockfd);
                    return 0;
                }
            }

            //Wrong command
            else{
                cout << "Not a valid command\n";
                cout << prompt;
                cout.flush();
            }
        }

        //Receive message from server
        if(FD_ISSET(sockfd, &tmp_fds)){
            memset(rec, 0, BUFLEN);
            int n;
            if((n = recv(sockfd, rec, BUFLEN, 0)) <= 0)
                error("Receive error");

            //Identify response
            string command(rec, n);
            int pos = command.find(":");
            string instr = command.substr(0, pos);

            //Login response
            if(instr == "login"){
                //Obtain login message components
                int pos2 = command.find(":", pos + 1);
                int pos3 = command.find(":", pos2 + 1);
                int pos4 = command.find(":", pos3 + 1);
                string user = command.substr(pos + 1, pos2 - pos - 1);
                string pass = command.substr(pos2 + 1, pos3 - pos2 - 1);
                string msg = command.substr(pos3 + 1, pos4 - pos3 - 1);
                string comm = "login " + user + " " + pass;
                flog << prompt << comm << "\n";
                //If login was successful the prompt is changed
                if(msg == "OK"){
                    loggedIn = 1;
                    loggedUser = user;
                    prompt = user + "> ";
                    flog << "\n";

                    cout << prompt;
                    cout.flush();
                }
                //Else we print error message
                else{
                    cout << msg << "\n";
                    flog << msg << "\n\n";

                    if(msg == "-8 Brute-force detectat"){
                        close(sockfd);
                        return 0;
                    }
                    else{
                        cout << prompt;
                        cout.flush();
                    }
                }
            }

            //Getuserlist response
            else if(instr == "getuserlist"){
                //Userlist received from server
                string res = command.substr(pos + 1, command.length() - pos - 1);
                int n = res.length();
                while(n == 4096){
                    memset(rec, 0, BUFLEN);
                    recv(sockfd, rec, sizeof(rec), 0);
                    res += string(rec);
                }
                //Delimitate users from userlist
                int pos1 = 0, pos2;
                pos2 = res.find(" ");
                while(pos2 > 0){
                    cout << res.substr(pos1, pos2 - pos1) << "\n";
                    flog << res.substr(pos1, pos2 - pos1) << "\n";
                    pos1 = pos2 + 1;
                    pos2 = res.find(" ", pos2 + 1);
                }
                cout << res.substr(pos1, pos2 - pos1) << "\n";
                flog << res.substr(pos1, pos2 - pos1) << "\n\n";

                cout << prompt;
                cout.flush();
            }

            //Getfilelist response
            else if(instr == "getfilelist"){
                //Filelist from server
                string res = command.substr(pos + 1, command.length() - pos - 1);
                int n = res.length();
                while(n == 4096){
                    memset(rec, 0, BUFLEN);
                    recv(sockfd, rec, sizeof(rec), 0);
                    res += string(rec);
                }
                //Delmitate files from filelist
                int pos1 = 0, pos2;
                pos2 = res.find(":");
                while(pos2 > 0){
                    cout << res.substr(pos1, pos2 - pos1) << "\n";
                    flog << res.substr(pos1, pos2 - pos1) << "\n";
                    pos1 = pos2 + 1;
                    pos2 = res.find(":", pos2 + 1);
                }
                flog << "\n";

                cout << prompt;
                cout.flush();
            }

            //Upload response
            else if(instr == "upload"){
                //Upload command arguments
                int pos2 = command.find(":", pos + 1);
                string file = command.substr(pos + 1, pos2 - pos - 1);
                string completeFile = "Users/" + loggedUser + "/" + file;
                string msg = command.substr(pos2 + 1, command.length() - pos2 - 1);
                //If upload attempt is unsuccessful print error message
                if(msg.find("OK") == -1){
                    cout << msg << "\n";
                    flog << msg << "\n\n";
                }
                //Else start upload
                else{
                    /*In case we interfere with another upload message we 
                    continue uploading the other file*/
                    string comm = "";
                    if(completeFile.find("OKup") != -1){
                        int pos = completeFile.find("up:");
                        comm = completeFile.substr(pos, command.length());
                        completeFile = completeFile.substr(0, pos - 1);
                    }
                    flog << "\n";

                    //Open file for reading and add it to uploadFiles map
                    FILE* f = fopen(completeFile.c_str(), "rb");
                    uploadFiles[file] = f;
                    //Read first block of data
                    memset(snd, 0, BUFLEN);
                    int read = fread(snd, sizeof(char), TRANS, f);
                    char toSend[BUFLEN] = {0};
                    //Compose packet header
                    string header("up:");
                    header += file;
                    header += string(BUFLEN - TRANS - header.length(), ':');
                    //Combine header and data
                    for(int i = 0; i < header.length(); i++)
                        toSend[i] = header[i];
                    for(int i = header.length(); i < header.length() + read; i++)
                        toSend[i] = snd[i - header.length()];
                    //Send packet to server
                    if(send(sockfd, toSend, read + BUFLEN - TRANS, 0) == -1)
                        error("Send error");
                    /*If we are done reading we print a finishing message 
                    remove file from map and close it*/
                    if(read < TRANS){
                        char strSize[10];
                        sprintf(strSize, "%d", read);
                        cout << "Upload finished: " << file << " - " << strSize << " bytes\n";
                        flog << prompt << "Upload finished: " << file << " - " << strSize << " bytes\n\n";
                        uploadFiles.erase(file);
                        fclose(f);
                    }
                    //If we interfered with another upload we upload that file
                    if(comm != "")
                        up(sockfd, comm, uploadFiles, prompt, flog);
                }
                cout << prompt;
                cout.flush();
            }

            //Up response
            else if(instr == "up")
                up(sockfd, command, uploadFiles, prompt, flog);

            //Download response
            else if(instr == "download"){
                //Find download command arguments
                int pos2 = command.find(":", pos + 1);
                int pos3 = command.find(":", pos2 + 1);
                string user = command.substr(pos + 1, pos2 - pos - 1);
                string file = command.substr(pos2 + 1, pos3 - pos2 - 1);
                string msg = command.substr(pos3 + 1, command.length() - pos3 - 1);
                //If the download attempt was unsuccessful print error message
                if(msg != "OK"){
                    cout << msg << "\n";
                    flog << msg << "\n\n";
                }
                //Else start download
                else{
                    flog << "\n";
                    msg = "down:" + user + ":" + file;
                    char pid[5];
                    sprintf(pid, "%d", PID);
                    string downFile(pid);
                    downFile += "_" + file;
                    string compDown = "Users/" + loggedUser + "/" + downFile;
                    //Open file for writing and add it to downloadFiles map
                    FILE* f = fopen(compDown.c_str(), "wb");
                    downloadFiles[downFile] = f;
                    send(sockfd, msg.c_str(), msg.length(), 0);
                }

                cout << prompt;
                cout.flush();
            }

            //Down response
            else if(instr == "down"){
                //Get data from download packets
                int pos2 = command.find(":", pos + 1);
                int pos3 = command.find(":", pos2 + 1);
                string user = command.substr(pos + 1, pos2 - pos - 1);
                string file = command.substr(pos2 + 1, pos3 - pos2 - 1);
                //Data to be written in file
                const char* toWrite = command.substr(BUFLEN - TRANS, command.length() - (BUFLEN - TRANS)).c_str();
                int size = command.length() - (BUFLEN - TRANS);
                char pid[5];
                sprintf(pid, "%d", PID);
                string downFile(pid);
                downFile += "_" + file;
                //Open download file and write data in it
                FILE* f = downloadFiles[downFile];
                fwrite(toWrite, sizeof(char), size, f);
                //If transfer is over print finishing message
                if(size < TRANS){
                    char strSize[10];
                    sprintf(strSize, "%ld", ftell(f));
                    fclose(f);
                    downloadFiles.erase(downFile);
                    cout << "Download finished: " << file << " - " << strSize << " bytes\n";
                    flog << prompt << "Download finished: " << file << " - " << strSize << " bytes\n\n";

                    cout << prompt;
                    cout.flush();
                }
                //Else send confirmation message to server
                else{
                    string toSend = "down:" + user + ":" + file;
                    send(sockfd, toSend.c_str(), toSend.length(), 0);
                }
            }

            //Share response
            else if(instr == "share"){
                //Find share response arguments and print message
                string msg = command.substr(pos + 1, command.length() - pos - 1);
                cout << msg << "\n";
                flog << msg << "\n\n";

                cout << prompt;
                cout.flush();
            }

            //Unshare response
            else if(instr == "unshare"){
                //Find unshare response arguments and print message
                string msg = command.substr(pos + 1, command.length() - pos - 1);
                cout << msg << "\n";
                flog << msg << "\n\n";

                cout << prompt;
                cout.flush();
            }

            //Delete response
            else if(instr == "delete"){
                //Find delete response arguments and print message
                string msg = command.substr(pos + 1, command.length() - pos - 1);
                cout << msg << "\n";
                flog << msg << "\n\n";

                cout << prompt;
                cout.flush();
            }

            //Server quitting response
            else if(instr == "quit"){
                //If server is quitting set client on quitting mode
                quitting = 1;
                cout << "Server quitting\n";
            }

            //Client quitting
            if(quitting && uploadFiles.empty() && downloadFiles.empty()){
                /*If client is quitting and it finished the transfers close 
                connection to server*/
                FD_CLR(sockfd, &read_fds);
                close(sockfd);
                flog.close();
                return 0;
            }
        }
    }
	return 0;
}

void up(int sockfd, string& command, map<string, FILE*>& uploadFiles, string& prompt, ofstream& flog){
    //In case we have more than one upload request we make a list of them
    vector<string> comms;
    int pos1, pos2 = command.find("up:");
    while(pos2 >= 0){
        pos1 = pos2 + 3;
        pos2 = command.find("up:", pos1);
        string comm = command.substr(pos1, pos2 - pos1);
        comms.push_back(comm);
    }
    //We serve each upload request
    for(int j = 0; j < comms.size(); j++){
        //Open file and read from it
        string file = comms[j];
        FILE* f = uploadFiles[file];
        char snd[BUFLEN] = {0};
        int read = fread(snd, sizeof(char), TRANS, f);
        
        char toSend[BUFLEN] = {0};
        //Prepare packet header
        string header("up:");
        header += file;
        header += string(BUFLEN - TRANS - header.length(), ':');
        //Put header and data together
        for(int i = 0; i < header.length(); i++)
            toSend[i] = header[i];
        for(int i = header.length(); i < header.length() + read; i++)
            toSend[i] = snd[i - header.length()];
        //Send packet to server
        if(send(sockfd, toSend, read + BUFLEN - TRANS, 0) == -1)
            error("Send error");
        //If all packets were transfered print finishing message
        if(read < TRANS){
            char strSize[10];
            sprintf(strSize, "%ld", ftell(f));
            cout << "Upload finished: " << file << " - " << strSize << " bytes\n";
            flog << prompt << "Upload finished: " << file << " - " << strSize << " bytes\n\n";
            uploadFiles.erase(file);
            fclose(f);

            cout << prompt;
            cout.flush();
        }
    }
}