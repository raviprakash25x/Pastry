/*
 * node.cpp
 *
 *  Created on: 08-Nov-2016
 *      Author: ravi
 */
#include "header.h"

pastryNode pNode;
int rowsUpdated = 0;

void create_node(int port){
	pNode.port = port;
	char ipAddress[INET_ADDRSTRLEN];
	pNode.IP = getIP(ipAddress);
	pNode.nodeId = MD5_HASH(pNode.IP, pNode.port);
	initTable(&pNode);
	initLeafSet(&pNode);
}

char* getIP(char* ipaddress){
	struct ifaddrs * ifAddrStruct=NULL;
	struct ifaddrs * ifa=NULL;
	void * tmpAddrPtr=NULL;

	getifaddrs(&ifAddrStruct);
	int iteration = 0;

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr) {
			continue;
		}
		if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
			// is a valid IP4 Address
			iteration++;
			tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);

			if(iteration == 2)
				strcpy(ipaddress, addressBuffer);
		}
	}
	if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
	return ipaddress;
}

void initTable(pastryNode* node){
	tableEntry temp[8][16];

	for(int i=0; i<8; i++){
		for(int j=0; j<16; j++){
			node->routingTable[i][j].IP = "";
			node->routingTable[i][j].port = -1;
			node->routingTable[i][j].nodeId = "";
		}

		node->routingTable[i][getInt(node->nodeId[i])].nodeId = (char)node->nodeId[i];
	}
}

int getInt(char ch){
	if(ch >= '0' && ch <= '9'){
		return ch-'0';
	}

	return ch-'a'+10;
}

void initLeafSet(pastryNode* node){
	for(int i =0; i<4; i++){
		node->leafSet[i].IP = pNode.IP;
		node->leafSet[i].port = pNode.port;
		node->leafSet[i].nodeId = pNode.nodeId;
	}
}

void printNode(){
	/*printf("Node ID:%s\n", pNode.nodeId.c_str());
	printf("Routing table:\n");

	for(int i=0; i<8; i++){
		for(int j=0; j<16; j++){
			printf("%s ",pNode.routingTable[i][j].nodeId.c_str());
		}
		printf("\n");
	}

	printf("LeafSet :\n");

	for(int i=0; i<4; i++){
		printf("%s ",pNode.leafSet[i].nodeId.c_str());
	}

	printf("\n");*/
	printf("%50s\n",pNode.nodeId.c_str());
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	printf("+%74s%73s","LEAF NODES","+\n");
	printf("+%20s","");
	for(int i=0;i<4;i++)
		printf("%12s%12s",pNode.leafSet[i].nodeId.c_str(),"");
	printf("%29s+\n","");
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	printf("+%74s%73s","ROUTING TABLE","+\n");
	printf("+%74s%73s","","+\n");
	for(int i=0;i<8;i++)
	{
		printf("+ ");
		for(int j=0;j<16;j++)
		{
			if(j<15)
				printf("%8s|",pNode.routingTable[i][j].nodeId.c_str());
			else
				printf("%8s ",pNode.routingTable[i][j].nodeId.c_str());
		}
		cout<<"+"<<endl;
	}
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}
