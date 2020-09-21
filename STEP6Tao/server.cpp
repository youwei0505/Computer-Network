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

void printINFO(int port);
Packet my_recv(int socket, struct sockaddr_in *from ,socklen_t *fromlen);
void my_send(int socket, Packet *p, struct sockaddr_in *to ,socklen_t *tolen);

int main(int argc, char **argv)
{
	srand(time(NULL));
	
	/*creat random txt*/
	fstream fp;
	fp.open("1.txt",ios::out);
	char ch;
	for(int i=0;i<10240;i++)
	{
		ch = rand() % 95 + 32;
		fp << ch;
		if(i%100 == 0)
		{
			fp << '\n';
			i++;
		}
	}
	
	fp.close();
	
	cout << "file is creat..." << endl;
	/*----------------*/
	
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
	
	printINFO(server_port);
	
	
	struct sockaddr_in client_addr;
	socklen_t clnt_addr_size = sizeof(client_addr);
	
	cout << "listening....." << endl;
	
	Packet temp;
	temp = my_recv(server_socket, &client_addr, &clnt_addr_size);
	
	client_port = temp.Header.srcPort;
	 
	int seq,ack;
	
	if(temp.Header.SYN)
	{
		cout << "starting three-way handshake....." << endl;
		seq = makeRandNUM();
		ack = temp.Header.seqNum + 1;
		
		Packet packet3;
		packet3.set("SYN/ACK", server_port, client_port, seq, ack, BUFFER_SIZE, NULL);
		my_send(server_socket, &packet3, &client_addr, &clnt_addr_size);
		
		temp = my_recv(server_socket, &client_addr, &clnt_addr_size);
		if(temp.Header.ACK && (temp.Header.ackNum == seq+1))
			cout << "complete the three-way handshake....." << endl;
	}
	
	cout << "===================" << endl;
	
	struct stat buf;
	int fstat = stat("1.txt",&buf);
	int fsize = buf.st_size;
	
	fstream ff;
	ff.open("1.txt",ios::in);
	
	cout << "start to send file 1.txt to " << inet_ntoa(client_addr.sin_addr) << ":" << temp.Header.desPort ;
	cout << " the file size is " << fsize << " bytes" << endl;
	
	char buffer[MSS];
	int count;
	int cwnd = 1;
	int SendPacket = 0;
	int GetACK = 0;
	int alreadySendByte = 0;
	int thisRoundSendByte = 0;
	int reSend = 0;
	int pAckNum = 0;
	bool finshThisRound = false;
	bool CdstAvoid = false;
	bool flag = false;
	Packet fastPacket;
	
	THRESHOLD = 8192;
	cout << "*****Slow start*****" << endl;
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
			ff.get(buffer[count]);
			count++;
			if(ff.eof())
				break;
		}
		
		Packet packet2;
		seq++;
		packet2.set("DATA", server_port, client_port, seq, 0, BUFFER_SIZE, buffer);
		
		/*故意遺失封包*/
		if(cwnd == 4096 && flag == false)
		{
			
			fastPacket.set("DATA", server_port, client_port, seq, 0, BUFFER_SIZE, buffer);
			flag = true;
			continue;
		}
		else
		{
			cout << "\t" ;
			my_send(server_socket, &packet2, &client_addr, &clnt_addr_size);
		}
		
		SendPacket++;
		alreadySendByte += packet2.Header.checkSum;
		thisRoundSendByte += packet2.Header.checkSum;
		if(thisRoundSendByte >= cwnd)
			finshThisRound = true;
		cout << "\t" ;
		cout << "Already send " << alreadySendByte << " byte..." << endl;//已傳送多少byte
		
		
		
		cout << "\t" ;
		pAckNum = temp.Header.ackNum;
		temp = my_recv(server_socket, &client_addr, &clnt_addr_size);
		 
		cout << "-------  ACK " << pAckNum <<" , next ACK  " << temp.Header.ackNum <<endl;
		if(temp.Header.ackNum == pAckNum)
		{
			reSend++;
			while(reSend >= 3)
			{
				/*快速重傳*/
				cout << "------------------Dupicate333----------" << endl;
				cout << "cwnd = " << cwnd << ", rwnd = " << BUFFER_SIZE << ", threshold = " << THRESHOLD << endl;
				cout << "------------------Dupicate333----------" << endl;
				//cout << fastPacket.Header.seqNum << endl;
				cout << "\t" ;
				//seq++;
				my_send(server_socket, &fastPacket, &client_addr, &clnt_addr_size);
				alreadySendByte += packet2.Header.checkSum;
				cout << "\t" ;
				cout << "Already send " << alreadySendByte << " byte..." << endl;
				cout << "\t" ;
				temp = my_recv(server_socket, &client_addr, &clnt_addr_size);
				if(temp.Header.ACK)
				{
					reSend = 0;
					THRESHOLD = cwnd/2;
					cwnd = MSS;
					cout << "cwnd = " << cwnd << ", rwnd = " << BUFFER_SIZE << ", threshold = " << THRESHOLD << endl;
					break;
				}
			}
		}
		
		
		
		if(ff.eof())
		{
			/*while(1)
			{
				cout << "\t" ;
				temp = my_recv(server_socket, &client_addr, &clnt_addr_size);
				if(temp.Header.ACK)
					GetACK++;
				if(temp.Header.ackNum == seq+1)
					break;
			}*/
			
			Packet packet3;
			seq++;
			packet3.set("FIN", server_port, client_port, seq, 0, BUFFER_SIZE, NULL);
			
			my_send(server_socket, &packet3, &client_addr, &clnt_addr_size);
			
			cout << "data transmission is complete..." << endl;
			break;
		}
		
		if(finshThisRound)//if not the last packet
		{	
			cout << "cwnd = " << cwnd << ", rwnd = " << BUFFER_SIZE << ", threshold = " << THRESHOLD << endl;
			if(cwnd >= THRESHOLD)
			{
				cout << "\n****Condestion avoidance****\n" << endl;
				CdstAvoid = true;
			}
			
			if(CdstAvoid)
				cwnd += MSS;
			else
				cwnd *= 2;
			
			
			
			thisRoundSendByte = 0;
			cout << "cwnd = " << cwnd << ", rwnd = " << BUFFER_SIZE << ", threshold = " << THRESHOLD << endl;
		}
	}
	
	
	close(server_socket);
	
	return 0;
}

void printINFO(int port)
{
	cout << "=====Parameter=====" << endl;
	cout << "The RTT delay = " << RTT << " ms" << endl;
	cout << "The threshold = " << THRESHOLD << " bytes" << endl;
	cout << "The MSS = " << MSS << " bytes" << endl;
	cout << "The buffer size = " << BUFFER_SIZE << " bytes" << endl;
	cout << "Server's IP is 127.0.0.1" << endl;
	cout << "Server is listening on port " << port << endl;
	cout << "===================" << endl;
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








