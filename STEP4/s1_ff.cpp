#include "packet.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <ctime>
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;


Packet my_recv(int socket, struct sockaddr_in *from ,socklen_t *fromlen);
void my_send(int socket, Packet *p, struct sockaddr_in *to ,socklen_t *tolen);

int main(int argc, char **argv)
{
	
	int server_port = atoi(argv[1]);
	int client_port;
	int server_socket = socket(AF_INET, SOCK_DGRAM, 0);//creat server's socket
	
	/*Creat & Initialize server's address*/
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));		
	server_addr.sin_family = AF_INET;					//for IPv4 protocol
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	//INADDR_ANY:   When receiving, received from any IP; 
														//When sending, sending by the lowest-numbered interface. 
	server_addr.sin_port = htons(server_port);			//set the port number, 
														//and using htons() ("Host TO Network Short integer")
														
	/*assign a port number and address to socket*/														
	if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		cout << "bind error" << endl;
		exit(1);
	}
	
	
	
	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(client_addr);
	
	cout << "listening....." << endl;
	
	Packet temp;
	temp = my_recv(server_socket, &client_addr, &client_addr_size);
	
	client_port = temp.Header.srcPort;
	 
	int seq,ack;
	
	if(temp.Header.SYN)
	{
		cout << "starting three-way handshake....." << endl;
		seq = makeRandNUM();
		ack = temp.Header.seqNum + 1;
		
		Packet packet3;
		packet3.set("SYN/ACK", server_port, client_port, seq, ack, BUFFER_SIZE, NULL);
		my_send(server_socket, &packet3, &client_addr, &client_addr_size);
		
		temp = my_recv(server_socket, &client_addr, &client_addr_size);
		if(temp.Header.ACK && (temp.Header.ackNum == seq+1))
			cout << "complete the three-way handshake....." << endl;
	}
	
	cout << "===================" << endl;
	
	ifstream file1;
	ifstream f1;
	file1.open("1.txt", ios::in | ios::binary | ios::ate);
	f1.open("1.txt", ios::in | ios::binary);
	int fileSize = file1.tellg();
	cout << "start to send file1 to client, the file size is " << fileSize << " byte\n";	
	cout << "start to send file1 to " << inet_ntoa(client_addr.sin_addr) << ":" << temp.Header.desPort ;
	
	int size[1]; 
	char fff[1];
	size[0]=fileSize;
	
	cout << fileSize << endl;
	
    //temp.set("DATA", server_port, client_addr.sin_port, seq, 0, BUFFER_SIZE, buffer);	
    //cout << "\nSend ";
    
	sendto(server_socket, &size ,sizeof(size), 0,(struct sockaddr*)&client_addr, client_addr_size);
	
    //cout << "to " << inet_ntoa(client_addr.sin_addr) << ":" << client_port << endl; 
    //printPacketINFO(temp);
    
    cout << fileSize << " "<<endl;
	
	
	
	char buffer[MSS];
	int count;
	int cwnd = 1;
	int SendPacket = 0;
	int GetACK = 0;
	int alreadySendByte = 0;
	int thisRoundSendByte = 0;
	bool finshThisRound = false;
	
	
	cout << "cwnd = " << cwnd << ", rwnd = " << BUFFER_SIZE << ", threshold = " << THRESHOLD << endl;
	while(1)
	{
		int dataByte;
				
		if(cwnd > MSS)//要拆成數個封包傳送
		{
			finshThisRound = false;
			dataByte = MSS;
		}
		else
		{
			finshThisRound = true;
			dataByte = cwnd;
		}
		
		count = 0;
		memset(buffer, 0, sizeof(buffer));
		while(count < dataByte)
		{
			f1.get(buffer[count]);
			count++;
			if(f1.eof())
				break;
		}
		
		Packet packet2;
		seq++;
		packet2.set("DATA", server_port, client_port, seq, 0, BUFFER_SIZE, buffer);
		
		cout << "\t" ;
		my_send(server_socket, &packet2, &client_addr, &client_addr_size);
		SendPacket++;
		alreadySendByte += packet2.Header.checkSum;
		thisRoundSendByte += packet2.Header.checkSum;
		if(thisRoundSendByte >= cwnd)
			finshThisRound = true;
		cout << "\t" ;
		cout << "Already send " << alreadySendByte << " byte..." << endl;//已傳送多少byte
		
		if(finshThisRound)
		{
			while(GetACK < SendPacket/2)
			{
				cout << "\t" ;
				temp = my_recv(server_socket, &client_addr, &client_addr_size);
				if(temp.Header.ACK)
				{
					GetACK++;
				}
			}
		
			cwnd*=2;
			thisRoundSendByte = 0;
			cout << "cwnd = " << cwnd << ", rwnd = " << BUFFER_SIZE << ", threshold = " << THRESHOLD << endl;
		}
		
		if(f1.eof())
		{
			while(1)
			{
				cout << "\t" ;
				temp = my_recv(server_socket, &client_addr, &client_addr_size);
				if(temp.Header.ACK)
					GetACK++;
				if(temp.Header.ackNum == seq+1)
					break;
			}
			
			Packet packet3;
			seq++;
			packet3.set("FIN", server_port, client_port, seq, 0, BUFFER_SIZE, NULL);
			
			my_send(server_socket, &packet3, &client_addr, &client_addr_size);
			
			cout << "data transmission is complete..." << endl;
			break;
		}
	}
	
	
	close(server_socket);
	
	return 0;
}



Packet my_recv(int socket, struct sockaddr_in *from ,socklen_t *fromlen)
{
	Packet buf;
	recvfrom(socket, &buf, sizeof(Packet), 0, (struct sockaddr*)from, fromlen);
	cout << "Received";
	printPacketINFO(buf);
	cout << "from " << inet_ntoa(from->sin_addr) << ":" << buf.Header.srcPort << endl;
	return buf;
}

void my_send(int socket, Packet *p, struct sockaddr_in *to ,socklen_t *tolen)
{
	sendto(socket, p, sizeof(Packet), 0, (struct sockaddr*)to, *tolen);
	cout << "Send";
    printPacketINFO(*p);
    cout << "to " << inet_ntoa(to->sin_addr) << ":" << p->Header.desPort << endl;
}








