/*
 * put.cpp
 *
 *  Created on: 10-Nov-2016
 *      Author: ravi
 */

#include "header.h"

void putToNode(string key, string value){
	string ip = pNode.IP;
	string port = getString(pNode.port);
	initializeClient((char*)ip.c_str(), (char*)port.c_str());
	int sockfd = createSocketAndConnect();
	ackFromServer(sockfd);
	formStringAndPut(key, value, sockfd);
}

void formStringAndPut(string key, string value, int sockfd){
	string toSend = "put$";
	toSend.append(key);
	toSend.append("$");
	toSend.append(value);
	toSend.append("#");
	write(sockfd, toSend.c_str(), toSend.length());
	close(sockfd);
}

void handlePutQuery(string received){

	vector <string> data;
	split(received, '$', data);
	string key = data[1];
	string value = data[2].substr(0, data[2].length()-1);
	//string hashVal = MD5_HASH(destnIP, atoi(destnPort.c_str()));
	tableEntry nextNode = getNextNode(key);

	/**
	 * Forwarding the routing to next node
	 */
	if(nextNode.nodeId != pNode.nodeId)
	{
		string str = "put$";
		str.append(key);
		str.append("$");
		str.append(value);
		str.append("#");
		initializeClient((char*)nextNode.IP.c_str(), (char*)getString(nextNode.port).c_str());
		int fd = createSocketAndConnect();
		ackFromServer(fd);
		write(fd, str.c_str(), str.length());
		close(fd);
	}
	else{
		pNode.hashTable[key] = value;
		printf("put value key:%s at %s\n", key.c_str(), pNode.nodeId.c_str());
	}
}

