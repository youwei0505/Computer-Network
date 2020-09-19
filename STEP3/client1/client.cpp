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



using namespace std;

void printINFO(int port);
Packet my_recv(int socket, struct sockaddr_in *from ,socklen_t *fromlen);
void my_send(int socket, Packet *p, struct sockaddr_in *to ,socklen_t *tolen);

int main(int argc, char **argv)
{
	srand(time(NULL));	
	int client_port = atoi(argv[1]);
	int server_port;
	char serverIP[20];
	
	printINFO(client_port);
	
	cout << "Please Input Sever [IP] [Port] you want to connect:" << endl;
	cin >> serverIP >> server_port;
	
	
	int client_sock = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	
	if(inet_pton(AF_INET, serverIP, &server_addr.sin_addr) <= 0)
	{
		cout << "[" << serverIP << "] is not a valid IPaddress/n" << endl; 
		exit(1);
	}
	else
	{
		cout << "IP OK" << endl;
	}

	if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		cout << "connect error" << endl;
		exit(1);
	}
	else
	{
		cout << "connect success" << endl;
	}
	
	
	
	cout << "starting three-way handshake....." << endl;
	int seq = makeRandNUM();
	int ack = 0;
	
	socklen_t svr_addr_size = sizeof(server_addr);
	
	Packet packet1;
	packet1.set("SYN", client_port, server_port, seq, ack, BUFFER_SIZE, NULL);
	
	my_send(client_sock, &packet1, &server_addr, &svr_addr_size);
	
	Packet temp;
	temp = my_recv(client_sock, &server_addr, &svr_addr_size);
	if(temp.Header.ACK && (temp.Header.ackNum == seq+1))
	{
		if(temp.Header.SYN)
		{
			seq++;
			ack = temp.Header.seqNum + 1;
			packet1.set("ACK", client_port, server_port, seq, ack, BUFFER_SIZE, NULL);
			my_send(client_sock, &packet1, &server_addr, &svr_addr_size);
			cout << "complete the three-way handshake....." << endl;
		}
		
	}
    
    cout << "Receive a file from " << inet_ntoa(server_addr.sin_addr) << " : " << server_port << endl;
    fstream ff;
	ff.open("2.txt",ios::out);
	char buffer[MSS];
	
	bool wrong = false;
	bool thisRoundRecovery = false;
	int countWrong = 0;
	char dataBeforeRecovery[5000];
	int tempDataSize = 0;
	int nowseq = 0;
	bool flagg = false;
	
	fstream fr;
	fr.open("temp",ios::out);
	
	while(1)
	{
		
		Packet packet2;
		cout << "\t" ;
		packet2 = my_recv(client_sock, &server_addr, &svr_addr_size);
		
		
		thisRoundRecovery = false;
		/*
		if(wrong && packet2.Header.seqNum == ack)
		{
			//以收到復原資料並復原//
			cout << "Data Recovery!!!!!" << endl;
			fr.close();
			fr.open("temp",ios::in);
			
			for(int i=0;i<tempDataSize;i++)
			{
				char ch;
				fr.get(ch);
				ff << ch;
			}
			fr.close();
			
			packet2.Header.seqNum = nowseq;
			ack = nowseq+1;
			wrong = false;
			flagg = true;
			thisRoundRecovery = true;
		}
		*/
		/*
		if(wrong)
		{
			//暫存快速重送前送來的資料//
			
			fr.write(packet2.Data ,packet2.Header.checkSum);
				
			tempDataSize += packet2.Header.checkSum;
			
		}
		*/
		if(packet2.Header.seqNum != ack && flagg == false)//遺失封包，順序錯亂
		{
			cout << "---Packet LOOOOOSS-----" << endl;
			close(client_sock);	
			return 0;
		}
		/*
		if(packet2.Header.seqNum != ack && flagg == false)//遺失封包，順序錯亂
		{
			cout << "---Packet LOOOOOSS-----" << endl;
			//sleep(5);
			nowseq = packet2.Header.seqNum;
			wrong = true;
			Packet packet4;
			seq++;
			packet4.set("ACK", client_port, server_port, seq, ack, BUFFER_SIZE, NULL);
			cout << "\t" ;
			my_send(client_sock, &packet4, &server_addr, &svr_addr_size);
			continue;
		}
		else
			wrong = false;
		*/
		Packet packet3;
		seq++;
		ack = packet2.Header.seqNum +1;
		packet3.set("ACK", client_port, server_port, seq, ack, BUFFER_SIZE, NULL);
		cout << "\t" ;
		my_send(client_sock, &packet3, &server_addr, &svr_addr_size);
		
		if(packet2.Header.FIN)
		{
			cout << "data transmission is complete..." << endl;
			break;
		}
		/*
		if(packet2.Header.FIN && !wrong)
		{
			cout << "data transmission is complete..." << endl;
			break;
		}
		*/
		if(thisRoundRecovery == false)
			ff.write(packet2.Data,packet2.Header.checkSum);
	}
    
    
    close(client_sock);
	return 0;
}

void printINFO(int port)
{
	cout << "=====Parameter=====" << endl;
	cout << "The RTT delay = " << RTT << " ms" << endl;
	cout << "The threshold = " << THRESHOLD << " bytes" << endl;
	cout << "The MSS = " << MSS << " bytes" << endl;
	cout << "The buffer size = " << BUFFER_SIZE << " bytes" << endl;
	cout << "Client's IP is 127.0.0.1" << endl;
	cout << "Client is listening on port " << port << endl;
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
