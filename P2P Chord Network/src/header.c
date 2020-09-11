#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <fcntl.h>

#define FNameMax 32 /* Max length of File Name */
#define FileMax 32 /* Max number of Files */
#define baseM 6 /* base number */
#define ringSize 64 /* ringSize = 2^baseM */
#define fBufSize 1024 /* file buffer size */
#define nodefmax 10  /* Max number of files in a node */
#define FNameMax 32  /* Max length of File Data */

#define REQUEST 0
#define RESPOND 1

#define SUCCESS 0
#define FAILURE -1

#define JOINCOMMAND 1
#define FINDPRECOMMAND 2
#define SUCCINFOCOMMAND 3
#define PREINFOCOMMAND 4
#define PREUPDATECOMMAND 5
#define SUCCUPDATECOMMAND 6
#define MOVEKEYSCOMMAND 7
#define PINGPONGCOMMAND 8
#define FILEADDCOMMAND 9
#define FILEDOWNCOMMAND 10
#define FILEREFCOMMAND 11
#define FILEDELETECOMMAND 12
#define LEAVEKEYSCOMMAND 13


typedef struct { /* Node Info Type Structure */
	int ID; /* ID */
	struct sockaddr_in addrInfo; /* Socket address */
} nodeInfoType;

typedef struct { /* File Type Structure */
	char Name[FNameMax]; /* File Name */
	int Key; /* File Key */
	nodeInfoType owner; /* Owner's Node */
	nodeInfoType refOwner; /* Ref Owner's Node */
} fileRefType;

typedef struct { /* Global Information of Current Files */
	unsigned int fileNum; /* Number of files */
	fileRefType fileRef[FileMax]; /* The Number of Current Files */
} fileInfoType;

typedef struct { /* Finger Table Structure */
	nodeInfoType Pre; /* Predecessor pointer */
	nodeInfoType finger[baseM]; /* Fingers (array of pointers) */
} fingerInfoType;

typedef struct { /* Chord Information Structure */
	fileInfoType FRefInfo; /* File Ref Own Information */
	fingerInfoType fingerInfo; /* Finger Table Information */
} chordInfoType;

typedef struct { /* Node Structure */
	nodeInfoType nodeInfo; /* Node's IPv4 Address */
	fileInfoType fileInfo; /* File Own Information */
	chordInfoType chordInfo; /* Chord Data Information */
} nodeType;

typedef struct {
	unsigned short msgID; // message ID
	unsigned short msgType; // message type (0: request, 1: response)
	nodeInfoType nodeInfo;// node address info
	short moreInfo; // more info
	fileRefType fileInfo; // file (reference) info
	unsigned int bodySize; // body size in Bytes
} chordHeaderType; // CHORD message header type

typedef struct {
	chordHeaderType msg;
	struct sockaddr_in senderSockAddr; //받는놈 ip주소
}msgInfoType;//

int sMode;
SOCKET rqSock, rpSock, flSock, frSock, fsSock, ffSock, ppSock;
nodeType myNode;
HANDLE hMutex;

int NofProcRecvThread;
HANDLE hThread[4]; // procRecvMsg[], procPPandFF, 
HANDLE procRecvThread[100];
HANDLE joinThread;
HANDLE findPreThread;
HANDLE fixFingerThread;
HANDLE pingPongThread;
HANDLE fileListenThread;
HANDLE fileSendThread;
HANDLE fileReceiveThread;
HANDLE stabilizeLeaveThread;
HANDLE leaveKeysThread;

msgInfoType sharing;

unsigned WINAPI procRecvMsg(void *i);	// 메시지 처리
unsigned WINAPI procJoin(void *arg);
unsigned WINAPI procFindPred(void *arg);
unsigned WINAPI procFixFinger(void *arg);
unsigned WINAPI procPingPong(void *arg);
unsigned WINAPI procFileListen(void *arg);
unsigned WINAPI procFileRecv(void *arg);
unsigned WINAPI procLeaveKeys(void *arg);

unsigned int str_hash(const char *);
// A Hash Function from a string to the ID/key space
int modIn(int modN, int targNum, int range1, int range2, int leftmode, int rightmode);
// For checking if targNum is "in" the range using left and right modes 
// under modN modular environment 
int twoPow(int power);
// For getting a power of 2 
int modMinus(int modN, int minuend, int subtrand);
// For modN modular operation of "minend - subtrand"
int modPlus(int modN, int addend1, int addend2);
// For modN modular operation of "addend1 + addend2"