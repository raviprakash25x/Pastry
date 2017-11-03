/*
 * header.h
 *
 *  Created on: 08-Nov-2016
 *      Author: ravi
 */

#ifndef HEADER_H_
#define HEADER_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <sstream>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <thread>
#include <math.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string>

#define BACKLOG 10


using namespace std;

extern int port;


typedef struct tableEntry{
	string nodeId;
	string IP;
	int port;
}tableEntry;

typedef struct node{
	string nodeId;
	string IP;
	int port;
	tableEntry routingTable[8][16];
    tableEntry leafSet[4];
	map < string, string > hashTable;
}pastryNode;

extern pastryNode pNode;
extern int rowsUpdated;

void startCommandLine();
int processInput(vector <string>& );
void split(const std::string &, char, vector<string> &);
void listenToRequests(string);
int establishConnection(const char*);
void acceptRequests(int);
void listenForConnections(int);
void *get_in_addr(struct sockaddr *);
void sigchld_handler(int);
string MD5_HASH(string, int);
void create_node(int);
char* getIP(char* );
void initTable(pastryNode*);
void initLeafSet(pastryNode*);
void handleQueries(int);
int initializeClient(char*, char*);
int createSocketAndConnect(void);
void connectToNode(string, string);
void ackFromServer(int);
void createStringAndSend(int, string, string);
void handleJoinQuery(string);
string getTableStr(tableEntry[8][16]);
int prefixMatch(string, string);
void printNode(void);
int getInt(char);
tableEntry getNextNode(string);
long converthextodec(string hex);
long diffHex(string x, string y);
bool isSmall(string x, string y);
bool isLarge(string x, string y);
tableEntry getReceivedEntry(string);
void populateReceivedTable(tableEntry, vector <string>& );
void notifyAllNodes(void);
string getString(int);
void insertInLeaf(tableEntry);
void insertInRoutingTable(tableEntry);
bool isSmallOrEqual(string, string);
void putToNode(string, string);
void formStringAndPut(string, string, int);
void handlePutQuery(string);
void getFromNode(string);
void formStringAndGet(string, int);
void handleGetQuery(string);
void displayResult(string);
#endif /* HEADER_H_ */
