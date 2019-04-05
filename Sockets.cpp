#include "pch.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")

#define UDP_PORT 4000
#define TCP_PORT "4001"

using namespace std;

int main()
{	
	//--------------------------------------------------------------------------------Section 1-----------------------------------------------------------------------------------------------------------------

	//UDP Stuff
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET sock;
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	cout << "Udp sock: <socket._socketobject object at 0x000000000" << &sock << ">" << endl;

	char broadcast = '1';
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

	struct sockaddr_in Recv_addr;
	struct sockaddr_in Send_addr;

	int len = sizeof(struct sockaddr_in);
	char sendbuff[] = "where";
	char recvbuff[5] = "";
	int recvbufflen = 5;

	Send_addr.sin_family = AF_INET;
	Send_addr.sin_port = htons(UDP_PORT);
	Send_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	//inet_pton(AF_INET,"127.0.0.1",&(Send_addr.sin_addr));

	Recv_addr.sin_family = AF_INET;
	Recv_addr.sin_port = htons(UDP_PORT);
	Recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	cout << "Broadcasting \"where\"" << endl;
	sendto(sock, sendbuff, strlen(sendbuff), 0, (sockaddr *)&Send_addr, sizeof(Send_addr));
	bind(sock, (sockaddr *)&Recv_addr, sizeof(Recv_addr));
	cout << "Receiving data on broadcast socket.." << endl;
	recvfrom(sock, recvbuff, recvbufflen, 0, (sockaddr *)&Recv_addr, &len);

	char add_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(Recv_addr.sin_addr), add_str, INET_ADDRSTRLEN);
	cout << "Found server at ('" << add_str << "', 4000) connecting.." << endl;



	//TCP Stuff
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	int iResult;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(add_str, TCP_PORT, &hints, &result);

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);

		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}

		break;
	}

	freeaddrinfo(result);

	cout << "Connected" << endl;

	
	//-----------------------------------------------------------------------------Section 2-----------------------------------------------------------------------------------------------------------------
	cout << "Appending login command to Tcp buffer : \"login:00010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899\"" << endl;
	char sendbuffTCP[] = "login:00010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899";
	char recvbuffTCP[1024];
	int recvbufflenTCP = 1024;
	memset(recvbuffTCP, 0, sizeof(recvbuffTCP));

	iResult = send(ConnectSocket, sendbuffTCP, (int)strlen(sendbuffTCP), 0);
	cout << "Sending Tcp output buffer... sent " << iResult << "B" << endl;

	for (int i = 0; i < 100; i++) {
		iResult = recv(ConnectSocket, recvbuffTCP, recvbufflenTCP, 0);

		if (recvbuffTCP[0] == 'y') {
			cout << "Parsing input Tcp buffer.. it's a yes" << endl;
			cout << "Password: " << i << endl;
			cout << "Appending \"image:93\" to Tcp output buffer" << endl;
			memset(sendbuffTCP, 0, sizeof(sendbuffTCP));
			string s = "image:" + to_string(i);
			strcpy_s(sendbuffTCP,sizeof(sendbuffTCP),s.c_str());
		}

		else {
			cout << "Parsing input Tcp buffer.. it's a no" << endl;
		}

		memset(recvbuffTCP, 0, sizeof(recvbuffTCP));
	}
	//----------------------------------------------------------------------------Section 3----------------------------------------------------------------------------------------------------------------
	iResult = send(ConnectSocket, sendbuffTCP, (int)strlen(sendbuffTCP), 0);
	cout << "Sending Tcp output buffer... sent " << iResult << "B" << endl;
	iResult = recv(ConnectSocket, recvbuffTCP, recvbufflenTCP, 0);
	
	int numOfBytes = atoi(recvbuffTCP);
	int  numOfRequests = 1;
	while (numOfBytes > numOfRequests * 1024) {
		numOfRequests++;
	}

	cout << "Parsing Tcp input buffer..Image size: " << numOfBytes << " appending " << numOfRequests << " requests" << endl;

	//----------------------------------------------------------------------------Section 4----------------------------------------------------------------------------------------------------------------
	char packetNum[10];
	char* imageBuff = (char*)malloc(numOfBytes);
	int currentByte = 0;

	char chunkBuff[1034];
	int chunkBuffLen = 1034;
	memset(chunkBuff, 0, sizeof(chunkBuff));


	for (int i = 0; i < numOfRequests; i++) {
		string s = "00000000" + to_string(i);
		strcpy_s(packetNum, sizeof(packetNum), s.c_str());
	
		bool dropped;
		do {
			dropped = false;
			cout << "Sending udp request for packet " << i << " data:\"" << i << "\"" << endl;
			sendto(sock, packetNum, strlen(packetNum), 0, (sockaddr *)&Recv_addr, sizeof(Recv_addr));

			struct timeval timeout;
			struct fd_set fds;
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;

			FD_ZERO(&fds);
			FD_SET(sock, &fds);

			cout << "Receiving data on Udp file socket.." << endl;
			int st = select(0, &fds, 0, 0, &timeout);
			switch (st) {
				case 0:
					dropped = true;
					cout << "Packet dropped! Rescheduling chunk " << i << endl;
					break;
				default: 
					int bytesReceived = recvfrom(sock, chunkBuff, chunkBuffLen, 0, (sockaddr *)&Recv_addr, &len);
					cout << "Received data for file chunk " << i << " " << bytesReceived << "B" << endl;
					
					for (int j = 9; j < bytesReceived; j++) {
						imageBuff[currentByte] = chunkBuff[j];
						currentByte++;
					}
					break;
			}
		} while (dropped);

		memset(chunkBuff, 0, sizeof(chunkBuff));
		memset(packetNum, 0, sizeof(packetNum));

	}

	cout << "We received all chunks! Assembling file.." << endl;

	//----------------------------------------------------------------------------Section 5----------------------------------------------------------------------------------------------------------------
	ofstream fs("result.jpg", std::ios::out | std::ios::binary | std::ios::app);
	fs.write(imageBuff, numOfBytes);
	fs.close();
	cout << "File result.jpg created" << endl;

	free(imageBuff);
	closesocket(sock);
	closesocket(ConnectSocket);
	WSACleanup();
}
