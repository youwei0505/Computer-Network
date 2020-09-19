#include "packet.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <ctime>
#include <fstream>
#include <random>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>

#include <chrono>
#include <iomanip>



using namespace std;

void printINFO(int port);
Packet my_recv(int socket, struct sockaddr_in *from ,socklen_t *fromlen);
void my_send(int socket, Packet *p, struct sockaddr_in *to ,socklen_t *tolen);

int main(int argc, char **argv)
{
	
	
	int threshold = 65536;
	
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
														
	/*assign a port number and address to socket*/														
	if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		cout << "bind error" << endl;
		exit(1);
	}
	
	
	
	
	struct sockaddr_in client_addr;
	socklen_t clnt_addr_size = sizeof(client_addr);
	
	int   client_port;
	cout << "\n====Original=======" << endl;
	cout << "     server_IP     " << inet_ntoa(server_addr.sin_addr) <<endl;
	cout << "     server_port   " <<server_port << endl;
	cout << "     client_IP     " << inet_ntoa(client_addr.sin_addr) <<endl;	
	cout << "     client_port   " <<client_port << endl;
	cout << "===================" << endl;
	
	
	cout << "listening....." << endl;
	
	Packet temp;
	temp = my_recv(server_socket, &client_addr, &clnt_addr_size);
	
	client_port = temp.Header.srcPort;
	cout << "\n====Connect========" << endl;
	cout << "     server_IP     " << inet_ntoa(server_addr.sin_addr) <<endl;
	cout << "     server_port   " <<server_port << endl;
	cout << "     client_IP     " << inet_ntoa(client_addr.sin_addr) <<endl;	
	cout << "     client_port   " <<client_port << endl;
	cout << "===================" << endl;
	//int seq,ack;
	
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
	Packet losspacke;
	
	cout << "*****Slow start*****" << endl;
	
	//傳送速率 : MSS/RTT
	while(1)
	{
		cout << "cwnd = " << cwnd << ", rwnd = " << BUFFER_SIZE << ", threshold = " << threshold << endl;
		//cwnd 從 1 開始 
		int dataByte;
		//窗格大小 跟cwnd 分開多個檔案傳送		
		if(cwnd > MSS)
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
		bzero(buffer,sizeof(buffer));
		
		while(count < dataByte)
		{
			ff.get(buffer[count]);
			count++;
			if(ff.eof())
				break;
		}
		
		//正常傳送封包（資料）
		Packet packet2;
		seq++;
		packet2.set("DATA", server_port, client_port, seq, 0, BUFFER_SIZE, buffer);
		
		
		/*
		//Call library
		//Poisson 分佈
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		default_random_engine generator (4);
		double num;	
		num = 0.000001;//1E-6
		cout << setprecision(8)  << num <<endl;
		poisson_distribution<int> distribution (4);
		//故意遺失封包//
		distribution(generator);
		//因為很小 所以不等於0 我都算分佈到其他 成功Loss packet
		if(distribution(generator) >= 0 && flag == false)
		{			
			losspacke.set("DATA", server_port, client_port, seq, 0, BUFFER_SIZE, buffer);
			flag = true;
			continue;
		}
		else
		{
			//正常傳送
			cout << "\t" ;
			my_send(server_socket, &packet2, &client_addr, &clnt_addr_size);
		}		
		*/
		
		//---------test-----------------
		//Poisson 分佈
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  		std::default_random_engine generator (seed);
		int num =6;	
		cout << "Poission num : "<<num <<endl;
		poisson_distribution<int> distribution (num);
		//故意遺失封包
		int number = distribution(generator);
		//因為很小 所以不等於0 我都算分佈到其他 成功Loss packet
		//if(cwnd == 1024 && flag == false)
		if( number > 8 && flag == false)
		{			
			cout << "\n\n--------Loss Packet at here: --------" <<endl;
			losspacke.set("DATA", server_port, client_port, seq, 0, BUFFER_SIZE, buffer);
			flag = true;
			continue;
		}
		else
		{
			//正常傳送
			cout << "\t" ;
			my_send(server_socket, &packet2, &client_addr, &clnt_addr_size);
		}		
		//---------test-----------------
		
		
		//計算總共的送出封包
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
		
		cout << "Now ACK : "<<pAckNum<<" ,ackNum "<<temp.Header.ackNum<<endl; 
		
		//loss packet
		if(temp.Header.ackNum == pAckNum)
		{
			cout << "Lost Data" << endl;
			/*
			reSend++;
			while(reSend >= 3)
			{
				cout << "---------Data LOOOOOSS---------" << endl;
				//cout << losspacke.Header.seqNum << endl;
				cout << "\t" ;
				//seq++;
				my_send(server_socket, &losspacke, &client_addr, &clnt_addr_size);
				cout << "\t" ;
				temp = my_recv(server_socket, &client_addr, &clnt_addr_size);
				if(temp.Header.ACK)
				{
					reSend = 0;
					break;
				}
			}
			*/
		}
		
		
		//send whole file end
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
		
		//if not sending finish
		if(finshThisRound)
		{	
			cwnd *= 2;
			/*
			if(CdstAvoid)
				cwnd += MSS;
			else
				cwnd *= 2;
			
			if(cwnd >= threshold)
			{
				cout << "****Condestion avoidance****" << endl;
				CdstAvoid = true;
			}
			*/
			thisRoundSendByte = 0;
			cout << "cwnd = " << cwnd << ", rwnd = " << BUFFER_SIZE << ", threshold = " << threshold << endl;
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








