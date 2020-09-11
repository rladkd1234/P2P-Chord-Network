#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#include <process.h>

#define FNameMax 32 /* Max length of File Name */
#define FileMax 32 /* Max number of Files */
#define baseM 6 /* base number */
#define ringSize 64 /* ringSize = 2^baseM */
#define fBufSize 1024 /* file buffer size */

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