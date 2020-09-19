#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <ctime>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

#define RTT 200
#define MSS 1024
int THRESHOLD = 65535;
#define BUFFER_SIZE 32768


class TCPheader;
class Packet;

int makeRandNUM();
void printPacketINFO(Packet p);

class TCPheader{
	public:
		short int srcPort;	//來源端Port
		short int desPort;	//目的端port
		
		int seqNum;
		int ackNum;
		
		//flag
		bool ACK;
		bool SYN;
		bool FIN;
		
		short int rwnd;		//receive window
		short int checkSum;
	
};

class Packet{
	public:
		TCPheader Header;
		char Data[MSS];
		
		Packet();
		void set(string type, short int srcP, short int desP, int seqN, int ackN, short int rwnD, char *data);
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
	memset(Data, 0, sizeof(Data));
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




int makeRandNUM()
{
	int num;
	num = rand()%10000+1;
	return num;
}








