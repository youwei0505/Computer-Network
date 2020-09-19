// Client side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 

#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#include <fstream>
#include <iostream> 

#define RTT 15
#define MSS 1024
#define THRESHOLD 65536
#define BUFFER_SIZE 52428

using namespace std;

class TCPheader{
	public:
		short int srcPort;
		short int desPort;
		
		int seqNum;
		int ackNum;
		
		bool ACK;
		bool SYN;
		bool FIN;
		
		short int rwnd;		
		short int checkSum;
	
};
class Packet{
	public:
		TCPheader Header;
		Packet();
		char Data[MSS];			
		void set(string type, short int srcP, short int desP, int seqN, int ackN, short int rwnD, char data[]);
};
Packet::Packet(){
	Header.srcPort = 0;
	Header.desPort = 0;
	Header.seqNum = 0;
	Header.ackNum = 0;
	Header.ACK = false;
	Header.SYN = false;
	Header.FIN = false;
	Header.rwnd = 0;
	Header.checkSum = 0;
	bzero(Data,sizeof(Data));
}
void Packet::set(string type, short int srcP, short int desP, int seqN, int ackN, short int rwnD, char *data){
	Header.srcPort = srcP;
	Header.desPort = desP;
	Header.seqNum = seqN;
	Header.ackNum = ackN;
	Header.rwnd = rwnD;
	
	if(type == "ACK")
	{
		Header.ACK = true;
		Header.SYN = false;
		Header.checkSum = 1;
	}
	else if(type == "SYN")
	{
		Header.SYN = true;
		Header.ACK = false;
		Header.checkSum = 1;
	}
	else if(type == "SYN/ACK")
	{
		Header.ACK = true;
		Header.SYN = true;
		Header.checkSum = 1;
	}
	else if(type == "FIN")
	{
		Header.FIN = true;
		Header.checkSum = 1;
	}
	else if(type == "DATA")
	{
		if(data != NULL)
		{
			int dataSize;
			if((int)strlen(data) > MSS)
				dataSize = MSS;
			else
				dataSize = strlen(data);
			
			strncpy(this->Data, data, dataSize);
			Header.checkSum = dataSize;
		}
	}
}

void printPacketINFO(Packet p)
{
	cout << " a packet";
	if(p.Header.ACK == true && p.Header.SYN == true)
		cout << "(SYN/ACK)";
	else if(p.Header.ACK == true)
		cout << "(ACK)";
	else if(p.Header.SYN == true)
		cout << "(SYN)";
	else if(p.Header.FIN == true)
		cout << "(FIN)";
	
	cout << "(seq = " << p.Header.seqNum << ", ack = " << p.Header.ackNum << ") " ;
}

Packet my_recv(int socket, struct sockaddr_in *from ,socklen_t *fromlen);
void my_send(int socket, Packet *p, struct sockaddr_in *to ,socklen_t *tolen);


int makeRandNUM()
{
	int num;
	num = rand()%10000+1;
	return num;
}

int main(int argc, char **argv) 
{ 
	char bufileOut1er[1024]; 
	char message[] = {"Hi,this is client.\n"};
	// Creating socket file descriptor 
	int client_socket = socket (AF_INET, SOCK_DGRAM, 0);
	int client_port = atoi(argv[1]);
	int server_port;
	char server_IP[15]={};
	int ack,seq;
	
	cout << "Please Input Sever [IP] [Port] you want to connect:" << endl;
	cin >> server_IP >> server_port;
	
	if ( client_socket < 0 ) { 
		perror("socket creation failed"); 
		exit(1); 
	} 
	
	struct sockaddr_in server_addr; 	
	bzero(&server_addr,sizeof(server_addr)); 
		
	// Filling server information 
	server_addr.sin_family = AF_INET; 
	server_addr.sin_port = htons(server_port); 	
	
	socklen_t server_addr_size = sizeof(server_addr);
	
	if(inet_pton(AF_INET, server_IP,&server_addr.sin_addr)== -1)
	{
		cout << "[" << server_IP << "] is not a valid IPaddress/n" << endl; 
		exit(1);
	}
	if(connect(client_socket,(struct sockaddr*)&server_addr,server_addr_size)== -1)
	{
		cout << "connect error" << endl;
		exit(1);
	}
		
	
	cout << "starting three-way handshake....." << endl;
	seq = makeRandNUM();
	Packet packet_sender;	
    Packet packet_receiver;
    
	packet_sender.set("SYN", client_port, server_port, seq, 0, BUFFER_SIZE, NULL);	
	//first send
	cout << "\nSend ";
	sendto(client_socket, &packet_sender , sizeof(Packet), 0,(struct sockaddr*)&server_addr, server_addr_size);
    cout << "to " << inet_ntoa(server_addr.sin_addr) << ":" << server_port << endl; 
    printPacketINFO(packet_sender);
    
    
    //wait server ack
    cout << "\nReceived ";    
    recvfrom(client_socket, &packet_receiver , sizeof(Packet), 0, (struct sockaddr*)&server_addr, &server_addr_size);
    cout << "from " << inet_ntoa(server_addr.sin_addr) << ":" << server_port << endl;            
    printPacketINFO(packet_receiver);
    
    
    
    ack = packet_receiver.Header.seqNum + 1;
	seq++;
    
    //third send
    packet_sender.set("ACK", client_port, server_port, seq, ack, BUFFER_SIZE, NULL);
    cout << "\nSend ";    
	sendto(client_socket, &packet_sender , sizeof(Packet), 0,(struct sockaddr*)&server_addr, server_addr_size);
    cout << "to " << inet_ntoa(server_addr.sin_addr) << ":" << server_port << endl; 
    printPacketINFO(packet_sender);
    
    
    cout << "\n\ncomplete the three-way handshake....." << endl;
    
    cout << "\n\n................................................\n\n" << endl;
    
    
    
	
	int ttt[10]={};
	char fff[1];  
	int fileSize; 
    ofstream fileOut1;
	fileOut1.open("2.txt", ios::out | ios::binary);
       
       
       
    recvfrom(client_socket, &ttt , sizeof(ttt), 0, (struct sockaddr*)&server_addr, &server_addr_size);   
    fileSize=ttt[0];
	cout <<" size : "<<fileSize<<endl;
	packet_sender.set("ACK", client_port, server_port, seq, ack, BUFFER_SIZE, NULL);
	sendto(client_socket, &packet_sender , sizeof(Packet), 0,(struct sockaddr*)&server_addr, server_addr_size);
	
	char buffer[MSS];
	
	int count = 0;
	while(1)
	{
		
		Packet packet2;
		cout << "\t" ;
		packet2 = my_recv(client_socket, &server_addr, &server_addr_size);
		count++;
		
		if(count >= 2)//收到2個以上的封包，回傳ACK
		{
			Packet packet3;
			seq++;
			ack = packet2.Header.seqNum +1;
			packet3.set("ACK", client_port, server_port, seq, ack, BUFFER_SIZE, NULL);
			cout << "\t" ;
			my_send(client_socket, &packet3, &server_addr, &server_addr_size);
			count = 0;
		}
		
		
		if(packet2.Header.FIN)
		{
			cout << "data transmission is complete..." << endl;
			break;
		}
		
		fileOut1.write(packet2.Data,packet2.Header.checkSum);
	}
    
    
    close(client_socket);
	return 0;
	
	
	
    
    
    
	
	 
} 

Packet my_recv(int socket, struct sockaddr_in *from ,socklen_t *fromlen)
{
	Packet buf;
	recvfrom(socket, &buf, sizeof(Packet), 0, (struct sockaddr*)from, fromlen);
	//cout << "Received";
	//printPacketINFO(buf);
	//cout << "from " << inet_ntoa(from->sin_addr) << ":" << buf.Header.srcPort << endl;
	return buf;
}

void my_send(int socket, Packet *p, struct sockaddr_in *to ,socklen_t *tolen)
{
	sendto(socket, p, sizeof(Packet), 0, (struct sockaddr*)to, *tolen);
	//cout << "Send";
    //printPacketINFO(*p);
   // cout << "to " << inet_ntoa(to->sin_addr) << ":" << p->Header.desPort << endl;
}
