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
	//socket的建立
    char Buf1er[1024] = {};
    char message[] = {"Hi,this is server.\n"};
    int ack,seq;
    int server_socket;
    int server_port;
    int i=0;
    
   
    
    server_socket = socket(AF_INET ,SOCK_DGRAM ,0);    
	server_port = atoi(argv[1]);
	
	
	
    if (server_socket == -1){
        printf("Fail to create a socket.");
    }
	
	
    //socket的連線
    struct sockaddr_in server_addr;     
    bzero(&server_addr,sizeof(server_addr));
	
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);     
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.2");
	
	if(bind(server_socket,(struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		cout << "bind error" << endl;
		return 0;
	}
	
	
	struct sockaddr_in client_addr;  	
    socklen_t client_addr_size = sizeof(client_addr); 
    
    //char* server_IP = inet_ntoa(server_addr.sin_addr);
	//char* client_IP = inet_ntoa(client_addr.sin_addr);
	int   client_port;
	cout << "\nserver_IP " << inet_ntoa(server_addr.sin_addr) <<endl;
	cout << "server_port " <<server_port << endl;
	cout << "client_IP " << inet_ntoa(client_addr.sin_addr) <<endl;	
	cout << "client_port " <<client_port << endl;



	cout << "listening....." << endl;
	
	Packet temp;
	temp = my_recv(server_socket, &client_addr, &client_addr_size);
	
	client_port = temp.Header.srcPort;
	 
	
	
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
	bool finshThisRound = false;
	bool CdstAvoid = false;
	
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
			
			if(CdstAvoid)
				cwnd += MSS;
			else
				cwnd *= 2;
			
			if(cwnd >= THRESHOLD)
			{
				cout << "****Condestion avoidance****" << endl;
				CdstAvoid = true;
			}
			
			thisRoundSendByte = 0;
			cout << "cwnd = " << cwnd << ", rwnd = " << BUFFER_SIZE << ", threshold = " << THRESHOLD << endl;
		}
		
		if(ff.eof())
		//if(alreadySendByte >= fsize)
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








