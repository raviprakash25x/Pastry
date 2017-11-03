/*
 * get.cpp
 *
 *  Created on: 10-Nov-2016
 *      Author: ravi
 */

#include "header.h"

void getFromNode(string key){
	string ip = pNode.IP;
	string port = getString(pNode.port);
	initializeClient((char*)ip.c_str(), (char*)port.c_str());
	int sockfd = createSocketAndConnect();
	ackFromServer(sockfd);
	formStringAndGet(key, sockfd);
}

void formStringAndGet(string key, int sockfd){
	string toSend = "get$";
	toSend.append(pNode.IP);
	toSend.append("$");
	toSend.append(getString(pNode.port));
	toSend.append("$");
	toSend.append(key);
	toSend.append("#");
	printf("Initial send:%s\n", toSend.c_str());
	write(sockfd, toSend.c_str(), toSend.length());
	close(sockfd);
}

void handleGetQuery(string received){
	vector <string> data;
	split(received, '$', data);
	string key = data[3].substr(0, data[3].length()-1);
	string destnIP = data[1];
	string destnPort = data[2];
	//string value = data[2].substr(0, data[2].length()-1);
	//string hashVal = MD5_HASH(destnIP, atoi(destnPort.c_str()));
	tableEntry nextNode = getNextNode(key);

	/**
	 * Forwarding the routing to next node
	 */
	if(nextNode.nodeId != pNode.nodeId)
	{
		string str = "get$";
		str.append(destnIP);
		str.append("$");
		str.append(destnPort);
		str.append("$");
		str.append(key);
		str.append("#");
		initializeClient((char*)nextNode.IP.c_str(), (char*)getString(nextNode.port).c_str());
		int fd = createSocketAndConnect();
		ackFromServer(fd);
		write(fd, str.c_str(), str.length());
		close(fd);
	}
	else{
		string value = "getVal$";
		value.append(pNode.hashTable[key]);
		value.append("#");
		printf("key match at: %s\n", pNode.nodeId.c_str());
		printf("key: %s, value:%s\n", key.c_str(), value.c_str());
		initializeClient((char*)destnIP.c_str(), (char*)destnPort.c_str());
		int fd = createSocketAndConnect();
		ackFromServer(fd);
		write(fd, value.c_str(), value.length());
		close(fd);
	}
}

void displayResult(string received){
		vector <string> data;
		split(received, '$', data);
		string value = data[1].substr(0, data[1].length()-1);
		printf("Fetched result: %s\n", value.c_str());
}

