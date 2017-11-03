//============================================================================
// Name        : main.cpp
// Author      : Ravi Prakash
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include "header.h"

int port;

int main() {
	startCommandLine();
	return 0;
}

void startCommandLine(){
	int returnVal;
	do{
		returnVal = 1;
		string ip;
		getline(std::cin, ip);
		vector <string> tokens;
		split(ip, ' ', tokens);
		returnVal = processInput(tokens);
	}while(returnVal);
}

int processInput(vector <string>& tokens){
	int cmdSize = tokens.size();
	int returnVal = 1;

	if(cmdSize == 1){
		string cmd = tokens[0];

		if(cmd.compare("quit") == 0){
			returnVal = 0;
		}
		else if(cmd.compare("shutdown") == 0){
			//TODO do shutdown operation
			returnVal = 0;
		}
		else if(cmd.compare("lset") == 0){
			//TODO print leafset of current node
		}
		else if(cmd.compare("create") == 0){
			//TODO create a node itself
			create_node(port);
			thread portThread(listenToRequests,to_string(port));
			//t1.join();
			portThread.detach();
		}
		else if(cmd.compare("print") == 0){
			printNode();
		}

	}
	else if(cmdSize == 2){
        string cmd = tokens[0];

		if(cmd.compare("get") == 0){
			string key = tokens[1];
			getFromNode(key);
		}
		else if(cmd.compare("port") == 0){
			//Assign port to the process using port command like "port 3000"
			port = atoi(tokens[1].c_str());

			//thread portThread(listenToRequests, port);
			//portThread.join();
		}

	}
	else if(cmdSize == 3){

		string cmd = tokens[0];

		if(cmd.compare("put") == 0){
			string key = tokens[1];
			string value = tokens[2];
			putToNode(key, value);
		}

		else if(cmd.compare("join") == 0){
			string ip = tokens[1];
			string port = tokens[2];
			connectToNode(ip, port);
		}

	}

	return returnVal;
}

/**
 * splits a given string on the basis
 * of delimiter given
 */
void split(const std::string &s, char delim, vector<string> &elems) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
}

