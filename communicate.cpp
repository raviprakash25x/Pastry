/*
 * communicate.cpp
 *
 *  Created on: 08-Nov-2016
 *      Author: ravi
 */

#include "header.h"

struct addrinfo* servinfo;

/**
 * Listens to connection requests
 * on the specified port
 */
void listenToRequests(string port){
	int sockfd = establishConnection(port.c_str());
	listenForConnections(sockfd);
	acceptRequests(sockfd);
}

void connectToNode(string ip, string port){
	initializeClient((char*)ip.c_str(), (char*)port.c_str());
	int sockfd = createSocketAndConnect();
	ackFromServer(sockfd);
	createStringAndSend(sockfd, pNode.IP, getString(pNode.port));
}

void createStringAndSend(int sockfd, string ip, string port){
	string toSend = "join$";
	toSend.append(ip);
	toSend.append("$");
	toSend.append(port);
	toSend.append("#");
	int len = toSend.length();
	cout<<"sending :"<<toSend;
	write(sockfd, toSend.c_str(), len+1);
	cout<<"sent"<<endl;
}

int initializeClient(char* IP, char* PORT){
	struct addrinfo hints;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	/*------------------GET ADDRESS INFO OF LOCAL---------*/

	if ((rv = getaddrinfo(IP, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	return 0;
}

/**
 * Creates socket and connects to
 * the repo server
 */
int createSocketAndConnect(){
	struct addrinfo *p;
	int sockfd;
	char s[INET6_ADDRSTRLEN];
	/*---------------Create Socket and connect----------*/

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	//convert IPv4 and IPv6 addresses from binary to text form
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s %d\n", s, sockfd);

	freeaddrinfo(servinfo); // all done with this structure
	return sockfd;
}

/**
 * Received acknowledgments from server
 * after connection is successful
 */
void ackFromServer(int sockfd){
	int numbytes;
	char buf[100];
	if ((numbytes = recv(sockfd, buf, 100-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}

	buf[numbytes] = '\0';

	printf("client: received '%s'\n",buf);
}


void acceptRequests(int sockfd){
	int new_fd;
	string currentIP;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	char s[INET6_ADDRSTRLEN];
	/*------------------ACCEPT--------------*/
	printf("server: waiting for connections...\n");

	while(1){
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

		if (new_fd == -1) {
			perror("accept");
			continue;
		}
		//convert IPv4 and IPv6 addresses from binary to text form
		inet_ntop(their_addr.ss_family,	get_in_addr((struct sockaddr *)&their_addr),
				s, sizeof s);
		currentIP = s;

		printf("\nserver: got connection from %s\n", s);
		printf("Connection request from %s\n",currentIP.c_str());

		//readQuery(new_fd);
		thread t1(handleQueries, new_fd);
		//t1.join();
		t1.detach();
	}
}

void handleQueries(int new_fd){
	//close(sockfd); // child doesn't need the listener

	if (send(new_fd, "You are connected!", 20, 0) == -1)
		perror("send");

	char buffer[150];
	string received = "";
	bzero(buffer,150);
	int n = read(new_fd,buffer,150);

	while(n > 0){
		received.append(buffer);
		//cout<<received<<endl;
		fflush(stdout);

		if(buffer[n-2] == '#' || buffer[n-1] == '#') break;
		bzero(buffer,150);
		n = read(new_fd,buffer,150);
	}

	if(strstr(received.c_str(), "join") != NULL){
		handleJoinQuery(received);
	}

	else if(strstr(received.c_str(), "table") != NULL){
		vector <string> receivedData;
		split(received, '$', receivedData);
		tableEntry receivedEntry = getReceivedEntry(receivedData[1]);
		populateReceivedTable(receivedEntry, receivedData);

		if(receivedData[receivedData.size()-1] == "1#"){
			notifyAllNodes();
		}
	}

	else if(strstr(received.c_str(), "notify") != NULL){
		vector <string> receivedData;
		split(received, '$', receivedData);
		tableEntry te;
		te.nodeId = receivedData[1];
		te.IP = receivedData[2];
		te.port = atoi(receivedData[3].c_str());
		//insertInLeaf(te);
		insertInRoutingTable(te);
	}
	else if(strstr(received.c_str(), "put") != NULL){
		handlePutQuery(received);
	}
	else if(strstr(received.c_str(), "getVal") != NULL){
		printf("Received getVal :%s\n", received.c_str());
		displayResult(received);
	}
	else if(strstr(received.c_str(), "get") != NULL){
		printf("Received get :%s\n", received.c_str());
		handleGetQuery(received);
	}

	fflush(stdout);
	close(new_fd);
}

void insertInRoutingTable(tableEntry te){
	int row = prefixMatch(pNode.nodeId, te.nodeId);
	int col = getInt(te.nodeId[row]);
	cout<<"Inserting node in route table"<<endl;

	if(pNode.routingTable[row][col].nodeId.length() > 1){
		int diff1 = diffHex(pNode.nodeId, pNode.routingTable[row][col].nodeId);
		int diff2 = diffHex(pNode.nodeId, te.nodeId);

		if(diff2 < diff1){
			pNode.routingTable[row][col] = te;
		}
	}
	else{
		pNode.routingTable[row][col] = te;
	}
}

void insertInLeaf(tableEntry te){
	if(isSmall(te.nodeId, pNode.nodeId) &&
			!(pNode.leafSet[0].nodeId == te.nodeId || pNode.leafSet[1].nodeId == te.nodeId )){

		if((pNode.leafSet[0].nodeId == pNode.nodeId) &&
				(pNode.leafSet[1].nodeId == pNode.nodeId)){
			pNode.leafSet[0] = te;
		}
		else if(pNode.leafSet[1].nodeId == pNode.nodeId){

			if(isLarge(pNode.leafSet[0].nodeId, te.nodeId)){
				pNode.leafSet[1] = pNode.leafSet[0];
				pNode.leafSet[0] = te;
			}
			else{
				pNode.leafSet[1] = te;
			}
		}
		else if(diffHex(pNode.nodeId, pNode.leafSet[1].nodeId) > diffHex(pNode.nodeId, te.nodeId)){
			pNode.leafSet[0] = pNode.leafSet[1];
			pNode.leafSet[1] = te;
		}
		/*else if(diffHex(pNode.nodeId, pNode.leafSet[0].nodeId) > diffHex(pNode.nodeId, te.nodeId)){
			pNode.leafSet[0] = te;
		}*/
		else if(diffHex(pNode.nodeId, pNode.leafSet[0].nodeId) < diffHex(pNode.nodeId, te.nodeId) &&
				diffHex(pNode.nodeId, pNode.leafSet[1].nodeId) > diffHex(pNode.nodeId, te.nodeId)){
			pNode.leafSet[0] = te;
		}
	}
	else if(!(pNode.leafSet[2].nodeId == te.nodeId || pNode.leafSet[3].nodeId == te.nodeId )){

		if(pNode.leafSet[2].nodeId == pNode.nodeId &&
				pNode.leafSet[3].nodeId == pNode.nodeId){
			pNode.leafSet[3] = te;
		}
		else if(pNode.leafSet[2].nodeId == pNode.nodeId){

			if(isLarge(te.nodeId, pNode.leafSet[3].nodeId)){
				pNode.leafSet[2] = pNode.leafSet[3];
				pNode.leafSet[3] = te;
			}
			else{
				pNode.leafSet[2] = te;
			}
		}
		else if(diffHex(pNode.nodeId, pNode.leafSet[2].nodeId) >
		diffHex(pNode.nodeId, te.nodeId)){
			pNode.leafSet[3] = pNode.leafSet[2];
			pNode.leafSet[2] = te;
		}
		else if(diffHex(pNode.nodeId, pNode.leafSet[2].nodeId) < diffHex(pNode.nodeId, te.nodeId) &&
				diffHex(pNode.nodeId, pNode.leafSet[3].nodeId) > diffHex(pNode.nodeId, te.nodeId)){
			pNode.leafSet[3] = te;
		}
	}
}

void notifyAllNodes(){
	for(int i=0; i<8; i++){
		for(int j=0; j<16; j++){
			tableEntry te = pNode.routingTable[i][j];
			string toSend = "notify$";
			toSend.append(pNode.nodeId);
			toSend.append("$");
			toSend.append(pNode.IP);
			toSend.append("$");
			toSend.append(getString(pNode.port));
			toSend.append("#");

			if(te.nodeId.length() > 1){
				initializeClient((char*)te.IP.c_str(), (char*)getString(te.port).c_str());
				int fd = createSocketAndConnect();
				ackFromServer(fd);
				write(fd, toSend.c_str(), toSend.length());
				close(fd);
			}
		}
	}
}

void populateReceivedTable(tableEntry receivedEntry, vector <string>& receivedData){
	int match = prefixMatch(receivedEntry.nodeId, pNode.nodeId);
	int i = 0, start = 2;

	for(i=rowsUpdated; i<=match; i++){
		for(int j=0; j<16; j++){
			string nodeEntry = receivedData[start];
			start++;
			vector <string> temp;
			split(nodeEntry, ',', temp);
			tableEntry te;
			te.nodeId = temp[0];
			te.IP = temp[1];
			te.port = atoi(temp[2].c_str());

			if(pNode.routingTable[i][j].nodeId.length() != 1)
				pNode.routingTable[i][j] = te;
			//insertInRoutingTable(te);
		}
		//rowsUpdated++;
	}

	int row = match;
	int col = getInt(receivedEntry.nodeId[row]);
	pNode.routingTable[row][col] = receivedEntry;

	start = 130;
	tableEntry tempEntry[4];

	if(receivedData[receivedData.size()-1] == "1#"){

		for(int i=0; i<4; i++){
			vector <string> temp;
			split(receivedData[start], ',', temp);
			//if(temp[0] != receivedEntry.nodeId){
			tableEntry te;
			te.nodeId = temp[0];
			te.IP = temp[1];
			te.port = atoi(temp[2].c_str());
			tempEntry[i] = te;
			printf("debug :%s\n", te.nodeId.c_str());

			start ++;
		}
		fflush(stdout);

		if(isSmall(receivedEntry.nodeId, pNode.nodeId)){
			/*	if(tempEntry[1].nodeId != receivedEntry.nodeId)
				pNode.leafSet[0] = tempEntry[1];

			pNode.leafSet[1] = receivedEntry;

			if(tempEntry[2].nodeId != receivedEntry.nodeId)
				pNode.leafSet[2] = tempEntry[2];

			if(tempEntry[3].nodeId != receivedEntry.nodeId)
				pNode.leafSet[3] = tempEntry[3];*/
			if(tempEntry[1].nodeId != receivedEntry.nodeId)
				insertInLeaf(tempEntry[1]);

			insertInLeaf(receivedEntry);

			if(tempEntry[2].nodeId != receivedEntry.nodeId)
				insertInLeaf(tempEntry[2]);

			if(tempEntry[3].nodeId != receivedEntry.nodeId)
				insertInLeaf(tempEntry[3]);

		}
		else{
			/*if(tempEntry[0].nodeId != receivedEntry.nodeId)
				pNode.leafSet[0] = tempEntry[0];

			if(tempEntry[1].nodeId != receivedEntry.nodeId)
				pNode.leafSet[1] = tempEntry[1];

			if(tempEntry[2].nodeId != receivedEntry.nodeId)
				pNode.leafSet[3] = tempEntry[2];

			pNode.leafSet[2] = receivedEntry;*/
			if(tempEntry[0].nodeId != receivedEntry.nodeId)
				insertInLeaf(tempEntry[0]);

			if(tempEntry[1].nodeId != receivedEntry.nodeId)
				insertInLeaf(tempEntry[1]);

			insertInLeaf(receivedEntry);

			if(tempEntry[2].nodeId != receivedEntry.nodeId)
				insertInLeaf(tempEntry[2]);
		}

		if(isLarge(pNode.leafSet[0].nodeId, pNode.leafSet[1].nodeId)){
			tableEntry c = pNode.leafSet[0];
			pNode.leafSet[0] = pNode.leafSet[1];
			pNode.leafSet[1] = c;
		}

		if(isLarge(pNode.leafSet[2].nodeId, pNode.leafSet[3].nodeId)){
			tableEntry c = pNode.leafSet[2];
			pNode.leafSet[2] = pNode.leafSet[3];
			pNode.leafSet[3] = c;
		}
	}

}

tableEntry getReceivedEntry(string receivedNode){
	vector <string> temp;
	split(receivedNode, ',', temp);
	tableEntry te;
	te.nodeId = temp[0];
	te.IP = temp[1];
	te.port = atoi(temp[2].c_str());
	return te;
}

void handleJoinQuery(string received){
	vector <string> data;
	split(received, '$', data);
	string destnIP = data[1];
	string destnPort = data[2].substr(0, data[2].length()-1);
	string hashVal = MD5_HASH(destnIP, atoi(destnPort.c_str()));
	string tableToSend = getTableStr(pNode.routingTable);
	tableEntry nextNode = getNextNode(hashVal);

	if(nextNode.nodeId == pNode.nodeId){
		tableToSend.append("1");
	}
	else{
		tableToSend.append("0");
	}

	tableToSend.append("#"); //terminating character

	initializeClient((char*)destnIP.c_str(), (char*)destnPort.c_str());
	int fd = createSocketAndConnect();
	ackFromServer(fd);

	write(fd, tableToSend.c_str(), tableToSend.length());
	close(fd);
	/**
	 * Forwarding the routing to next node
	 */
	if(nextNode.nodeId != pNode.nodeId)
	{
		string str = "join$";
		str.append(destnIP);
		str.append("$");
		str.append(destnPort);
		str.append("#");
		initializeClient((char*)nextNode.IP.c_str(), (char*)getString(nextNode.port).c_str());
		int fd = createSocketAndConnect();
		ackFromServer(fd);
		write(fd, str.c_str(), str.length());
		close(fd);
	}

	//Extra added
	tableEntry receivedEntry;
	receivedEntry.nodeId = hashVal;
	receivedEntry.IP = destnIP;
	receivedEntry.port = atoi(destnPort.c_str());
	insertInRoutingTable(receivedEntry);
	insertInLeaf(receivedEntry);
}

tableEntry getNextNode(string nodeId){

	tableEntry leftLeaf = pNode.leafSet[0];
	tableEntry rightLeaf = pNode.leafSet[3];
	bool leftBound = isSmallOrEqual(leftLeaf.nodeId,nodeId);
	bool rightBound = isSmallOrEqual(nodeId,rightLeaf.nodeId);

	if(leftBound && rightBound){

	}
	else {
		int row = prefixMatch(nodeId, pNode.nodeId);
		int col = getInt(nodeId[row]);

		if(pNode.routingTable[row][col].nodeId.length() > 1){
			printf("Next node: %s\n",pNode.routingTable[row][col].nodeId.c_str());
			return pNode.routingTable[row][col];
		}
	}

	tableEntry minDistance;
	minDistance.IP = pNode.IP;
	minDistance.nodeId = pNode.nodeId;
	minDistance.port = pNode.port;

	long minDiff = diffHex(pNode.nodeId, nodeId);

	for(int i=0; i<4; i++){
		string leafNodeID = pNode.leafSet[i].nodeId;
		long diff = diffHex(leafNodeID, nodeId);

		if(diff < minDiff){
			minDiff = diff;
			minDistance = pNode.leafSet[i];
		}
	}

	for(int i=0; i<8; i++){
		for(int j=0; j<16; j++){
			string tableNodeID = pNode.routingTable[i][j].nodeId;
			long diff = diffHex(tableNodeID, nodeId);

			if(diff < minDiff && tableNodeID.length() > 1){
				minDiff = diff;
				minDistance = pNode.routingTable[i][j];
			}
		}
	}
	printf("Next node: %s\n",minDistance.nodeId.c_str());
	return minDistance;
}

string getTableStr(tableEntry routingTable[8][16]){
	string str = "table$";
	str.append(pNode.nodeId);
	str.append(",");
	str.append(pNode.IP);
	str.append(",");
	str.append(getString(pNode.port));
	str.append("$");


	for(int i=0; i<8; i++){
		for(int j=0; j<16; j++){
			string nodeId = routingTable[i][j].nodeId;
			string ip = routingTable[i][j].IP;
			int port = routingTable[i][j].port;
			str.append(nodeId);
			str.append(",");
			str.append(ip);
			str.append(",");
			str.append(getString(port));
			str.append("$");
		}
	}

	for(int i=0; i<4; i++){
		tableEntry te= pNode.leafSet[i];
		str.append(te.nodeId);
		str.append(",");
		str.append(te.IP);
		str.append(",");
		str.append(getString(te.port));
		str.append("$");
	}
	//str.append("#");moved to calling function

	return str;
}

int prefixMatch(string a,string b)
{
	int match=0;
	for(int i=0;i<a.length();i++)
	{
		if(a[i]==b[i])
			match++;
		else
			break;
	}
	return match;
}

long converthextodec(string hex)
{
	int p=0;
	long dec=0;
	int l=hex.length();
	for (int i = l-1; i >= 0; i-- )
	{
		if (hex[i] >= 'a')
			dec +=  (hex[i] - 87) * (long)pow(16, p);
		else
			dec +=  (hex[i] - 48) * (long)pow(16, p);

		p++;
	}
	return dec;
}

long diffHex(string x, string y)
{
	long diff=labs( converthextodec(x)-converthextodec(y) );
	return diff;
}

bool isSmall(string x, string y)
{
	long diff=converthextodec(x)-converthextodec(y);
	if(diff<0)
		return true;
	else
		return false;
}

bool isSmallOrEqual(string x, string y)
{
	long diff=converthextodec(x)-converthextodec(y);
	if(diff<=0)
		return true;
	else
		return false;
}

bool isLarge(string x, string y)
{
	long diff=converthextodec(x)-converthextodec(y);
	if(diff>0)
		return true;
	else
		return false;
}

/**
 *Establishes connection with local
 *port to listen
 */
int establishConnection(const char* PORT){
	/*
	 * variable declarations
	 */
	int sockfd = -1;
	struct addrinfo hints, *servinfo, *p;
	int yes=1;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	/*------------------GET ADDRESS INFO---------*/

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	/*------------------CREATE SOCKET AND BIND---------*/

	for(p = servinfo; p != NULL; p = p->ai_next) { //will run once
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
	return sockfd;
}

/**
 * listens for other clients for
 * incoming connections
 */
void listenForConnections(int sockfd){
	struct sigaction sa;
	/*-----------------LISTEN---------*/

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
	/*------------------KILL ZOMBIES----------*/

	sa.sa_handler = sigchld_handler; // reap all dead/zombie processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
}

void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

string getString(int val){
	return to_string(val);
}




