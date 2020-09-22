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
int main(int argc , char *argv[])

{
	

    //socket的建立
    char Buf1er[1024] = {};
    char message[] = {"Hi,this is server.\n"};
    int ack,seq;
    int server_socket;
    int server_port;
    int i=0;
    
    while(1)
    {
    cout << i << endl; 
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
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
      
	if(bind(server_socket,(struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		cout << "bind error" << endl;
		return 0;
	}
	
	
	struct sockaddr_in client_addr;  	
    socklen_t client_addr_size = sizeof(client_addr); 
    
    char* server_IP = inet_ntoa(server_addr.sin_addr);
	char* client_IP = inet_ntoa(client_addr.sin_addr);
	int   client_port = ntohs(client_addr.sin_port);
		
	
		
    
   	Packet packet_sender;	
    Packet packet_receiver;
    
    
    cout << "listening....." << endl;
    
   	cout << "starting three-way handshake....." << endl;
   	//wait for SYN
   	//1st
    cout << "\nReceived ";  
    
    int flag;
    flag = recvfrom(server_socket, &packet_receiver, sizeof(Packet), 0, (struct sockaddr*)&client_addr, &client_addr_size);  
    cout << "from " << inet_ntoa(client_addr.sin_addr) << ":" << client_port << endl;
    printPacketINFO(packet_receiver);
    
	

    int pid;
    pid = fork();
       
    
    if( pid > 0 )
    {
    	cout <<"parent end";
    	continue;
    }
    if( pid == 0 )
    {
    //both server and client
    //2nd
    seq = makeRandNUM();
    ack = packet_receiver.Header.seqNum + 1;
    
    //send SYN ACK  
    packet_sender.set("SYN/ACK", server_port, packet_receiver.Header.srcPort, seq, 0, BUFFER_SIZE, NULL);	
    cout << "\nSend ";
	sendto(server_socket, &packet_sender ,sizeof(Packet), 0,(struct sockaddr*)&client_addr, client_addr_size);
    cout << "to " << inet_ntoa(client_addr.sin_addr) << ":" << client_port << endl; 
    printPacketINFO(packet_sender);
    
    //wait for ACK 3
    //3rd
    cout << "\nReceived ";    
    recvfrom(server_socket, &packet_receiver, sizeof(Packet), 0, (struct sockaddr*)&client_addr, &client_addr_size);  
    cout << "from " << inet_ntoa(client_addr.sin_addr) << ":" << client_port << endl;
    printPacketINFO(packet_receiver); 
    
    cout << "\n\ncomplete the three-way handshake....." << endl;
      	
	cout << "A server " << server_IP << " has connected via port num " << server_port << endl;	
	cout << "A client " << client_IP << " has connected via port num " << client_port << endl;	
        
    cout << "\n\n................................................\n\n" << endl;  
         
	Packet packet_sender2;	
    Packet packet_receiver2;
    
    seq = makeRandNUM();
    
	ifstream file1;
	ifstream f1;
	file1.open("Video1.mp4", ios::in | ios::binary | ios::ate);
	f1.open("Video1.mp4", ios::in | ios::binary);
	int fileSize = file1.tellg();
	cout << "start to send file1 to client, the file size is " << fileSize << " byte\n";	
	cout << "start to send file1 to " << inet_ntoa(client_addr.sin_addr) << ":" << packet_receiver.Header.desPort ;		
	int count;
	int cwnd = 1;
	int position = 1;	
	int remainSize = fileSize;		
	Packet temp;
	int buffer[1]; 
	char fff[1];
	buffer[0]=fileSize;		
	cout << fileSize << endl;
   	sendto(server_socket, &buffer ,sizeof(buffer), 0,(struct sockaddr*)&client_addr, client_addr_size);
    cout << fileSize << " ";        
    while(fileSize)
	{		
	f1.get(fff[0]);
	sendto(server_socket, &fff ,sizeof(fff), 0,(struct sockaddr*)&client_addr, client_addr_size); 	
    recvfrom(server_socket, &packet_receiver, sizeof(Packet), 0, (struct sockaddr*)&client_addr, &client_addr_size);         
    fileSize--;    
	}
    
    cout << "data transmission is complete..." << endl;
	
	
    close(server_socket);
    cout <<"child end";
    return 0;
    
    }    
    
    cout <<"total end";
    
    }//while lopp
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
