#include "header.c"

void printHelpCommand()
{
	printf("Enter a command - <c>reate : Create the chord network\n");
	printf("Enter a command - <j>oin : Join the chord network\n");
	printf("Enter a command - <l>eave : Leave the chord network\n");
	printf("Enter a command - <a>dd : Add a file to the nework\n");
	printf("Enter a command - <d>elete : Delete a file to the newtork\n");
	printf("Enter a command - <s>earch : File search and download\n");
	printf("Enter a command - <f>inger : Show the finger table\n");
	printf("Enter a command - <i>nfo : Show the node information\n");
	printf("Enter a command - <m>ute : Toggle the silent mode\n");
	printf("Enter a command - <h>elp : Show the help message\n");
	printf("Enter a command - <q>uit : Quit the program\n");
}

void printFileTable()
{
	printf("myNode IP address : %s, Port Number : %d, ID : %d\nown filenum : %d, ref filenum : %d\n",
		inet_ntoa(myNode.nodeInfo.addrInfo.sin_addr), ntohs(myNode.nodeInfo.addrInfo.sin_port),
		myNode.nodeInfo.ID, myNode.fileInfo.fileNum, myNode.chordInfo.FRefInfo.fileNum);

	printf("owned file Info : \n");
	if (myNode.fileInfo.fileNum != 0) {
		for (int i = 0; i < (int)myNode.fileInfo.fileNum; i++)
		{
			printf("table[%d] : name : %s, filekey : %d, owner_id : %d, ref_owner_id : %d\n",
				i, myNode.fileInfo.fileRef[i].Name, myNode.fileInfo.fileRef[i].Key,
				myNode.fileInfo.fileRef[i].owner.ID, myNode.fileInfo.fileRef[i].refOwner.ID);
		}
	}
	else {
		printf("haven't owned file\n");
	}

	printf("Ref file Info : \n");
	if (myNode.chordInfo.FRefInfo.fileNum != 0) {
		for (int i = 0; i < (int)myNode.chordInfo.FRefInfo.fileNum; i++)
		{
			printf("table[%d] : name : %s, filekey : %d, owner_id : %d, ref_owner_id : %d\n",
				i, myNode.chordInfo.FRefInfo.fileRef[i].Name, myNode.chordInfo.FRefInfo.fileRef[i].Key,
				myNode.chordInfo.FRefInfo.fileRef[i].owner.ID, myNode.chordInfo.FRefInfo.fileRef[i].refOwner.ID);
		}
	}
	else {
		printf("haven't Ref file\n");
	}
}

int CheckMsgGood(int a, int b, int c, int d)//���� �޽����� �ڽ��� ���ϴ°����� Ȯ���ϱ� ���� �Լ�
{


	return 1;
}

int main(int argc, char* argv[])
{
	//FILE * fp; //������ ������ �����ϳ� �˻��ϴ� �Լ���
	WSADATA wsaData;         // Structure for WinSock setup communication
	int exitFlag = 0;       //���������� �ؾߵɰ� ����
	nodeInfoType helperNode;
	nodeInfoType rpNode;
	chordHeaderType rqMsg, rpMsg;
	fileRefType rpFileRef;
	int AddrLen = sizeof(struct sockaddr_in);
	char *file_name_temp = "\0";
	int flag = 0;
	int file_key_temp; //�ƹ������� �ؽ��Լ��� �ð��Һ��� �ƴ�
	int keyCounts = 0;
	struct sockaddr_in receiveAddr;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // Load Winsock 2.2 DLL
		fprintf(stderr, "WSAStartup() failed");
		exit(1);
	}

	//WaitForSingleObjects(2, hThread, TRUE, INFINITE);

	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	if (atoi(argv[2]) > 65535 || atoi(argv[2]) < 49152) {
		printf("CHORD> \a[ERROR] <Port No> should be in [49152, 65535]!\n");
		exit(1);
	}

	/*MyNode�� �����ּ� ����*/
	memset(&myNode, 0, sizeof(myNode));
	myNode.nodeInfo.addrInfo.sin_family = AF_INET;
	myNode.nodeInfo.addrInfo.sin_addr.s_addr = inet_addr(argv[1]);
	//myNode.nodeInfo.addrInfo.sin_addr.s_addr = inet_addr(IN_ADDR_ANY);
	myNode.nodeInfo.addrInfo.sin_port = htons(atoi(argv[2]));

	char *curIPstr = inet_ntoa(myNode.nodeInfo.addrInfo.sin_addr);
	myNode.nodeInfo.ID = str_hash(curIPstr);

	printf("myNode IP address : %s, Port Number : %d, ID : %d \n\n",
		inet_ntoa(myNode.nodeInfo.addrInfo.sin_addr), ntohs(myNode.nodeInfo.addrInfo.sin_port), myNode.nodeInfo.ID);

	printHelpCommand();

	hMutex = CreateMutex(NULL, FALSE, NULL); /*����ȭ*/

	int createFlag = 0;
	int joinFlag = 0;

	char helperNodeIP[30];
	int helperNodePort = 0;
	char buf[1024];
	int retval;

	srand((unsigned int)(time(NULL)));

	while (1) {
		char userInput = ' ';
		printf("UserInput : <'help' for help message>.\n");
		scanf(" %c", &userInput);


		switch (userInput) {

			/*Create �ҽ� ���� !!!*/
		case 'C':
		case 'c':
			if (createFlag) {
				printf("�̹� Create�� Node�Դϴ�!!!\n");
				break;
			}

			if (joinFlag) {
				printf("�̹� join�� Node�Դϴ�!!!\n");
				break;
			}
			printf("Network Create!!!\n");
			sMode = 1;

			//181103 FingerTable ����
			myNode.fileInfo.fileNum = 0;
			myNode.chordInfo.fingerInfo.Pre = myNode.nodeInfo;
			for (int i = 0; i < baseM; i++)
			{
				myNode.chordInfo.fingerInfo.finger[i] = myNode.nodeInfo;
			}

			/*Socket Init*/
			if ((rqSock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
				perror("Error: RQsocket failed!");
				exit(1);
			}

			if ((rpSock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
				perror("Error: RPsocket failed!");
				exit(1);
			}
			if ((ffSock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
				perror("Error: FFsocket failed!");
				exit(1);
			}
			if ((ppSock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
				perror("Error: PPsocket failed!");
				exit(1);
			}

			procRecvThread[0] = (HANDLE)_beginthreadex(NULL, 0, (void*)procRecvMsg, (void*)&exitFlag, 0, NULL);
			fixFingerThread = (HANDLE)_beginthreadex(NULL, 0, (void*)procFixFinger, (void*)&exitFlag, 0, NULL);
			pingPongThread = (HANDLE)_beginthreadex(NULL, 0, (void*)procPingPong, (void*)&exitFlag, 0, NULL);
			fileListenThread = (HANDLE)_beginthreadex(NULL, 0, (void*)procFileListen, (void*)&exitFlag, 0, NULL);

			createFlag = 1;

			break;

			/*Join �ҽ� ���� !!!*/
		case 'J':
		case 'j':

			if (createFlag) {
				printf("�̹� Create�� Node�Դϴ�!!!\n");
				break;
			}

			if (joinFlag) {
				printf("�̹� join�� Node�Դϴ�!!!\n");
				break;
			}

			printf("Join ����!!!\n");
			sMode = 1;

			myNode.fileInfo.fileNum = 0;
			/*�ΰ� ���̺� �ʱ�ȭ*/
			myNode.chordInfo.fingerInfo.Pre.ID = -1;
			for (int i = 0; i < baseM; i++)
				myNode.chordInfo.fingerInfo.finger[i].ID = -1;

			/*Helper ����� ���� �Է�*/
			printf("HelperNode�� IP�ּ� : \n");
			scanf("%s", helperNodeIP);
			//printf("�Էµ� HelperNode�� �ּ� : %s\n", helperNodeIP);

			printf("HelperNode�� Port��ȣ : \n");
			scanf("%d", &helperNodePort);
			//	printf("�Էµ� HelperNode�� Port��ȣ : %d\n", helperNodePort);

			memset(&helperNode, 0, sizeof(helperNode));
			helperNode.addrInfo.sin_family = AF_INET;
			helperNode.addrInfo.sin_addr.s_addr = inet_addr(helperNodeIP);
			helperNode.addrInfo.sin_port = htons(helperNodePort);


			//if (!sMode)	
				printf("My IP Address: %s, Port No: %d, ID: %d\n",
				inet_ntoa(myNode.nodeInfo.addrInfo.sin_addr), ntohs(myNode.nodeInfo.addrInfo.sin_port), myNode.nodeInfo.ID);

			//if (!sMode)
				printf("HelperNode IP address : %s, Port Number : %d\n",
				inet_ntoa(helperNode.addrInfo.sin_addr), ntohs(helperNode.addrInfo.sin_port));


			/*Sock Init*/
			if ((rqSock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
				perror("Error: rqsocket failed!");
				exit(1);
			}

			if ((rpSock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
				perror("Error: rpsocket failed!");
				exit(1);
			}
			if ((ffSock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
				perror("Error: ffsocket failed!");
				exit(1);
			}
			if ((ppSock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
				perror("Error: ppsocket failed!");
				exit(1);
			}

			/*Helper���� SuccInfo��û*/
			memset((char*)&rqMsg, 0, sizeof(rqMsg));
			memset((char*)&rpMsg, 0, sizeof(rpMsg));

			rqMsg.msgType = REQUEST; // request
			rqMsg.msgID = JOINCOMMAND; // JoinInfo
			rqMsg.nodeInfo = myNode.nodeInfo;
			rqMsg.moreInfo = 0;
			rqMsg.bodySize = 0;

			//	printf("rqMsg->addrInfo : %s\nMsg->addr_port : %d\n", inet_ntoa(rqMsg.nodeInfo.addrInfo.sin_addr), ntohs(rqMsg.nodeInfo.addrInfo.sin_port));

			//if (!sMode)
				printf("JOIN) JoinInfo ��û��.\n");

			retval = sendto(rqSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr *) &helperNode.addrInfo, AddrLen);
			
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] JOIN) joininfo REQUEST timed out!\n");

					continue;
				}

				printf("\a[ERROR] JOIN) Joininfo Sendto Error!\n");

				continue;
			}


			if (!sMode)
				printf("JOIN) joininfo waiting...\n");

			retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr *) &receiveAddr, &AddrLen);
			
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] JOIN) joininfo Receive timed out!\n");

					continue;
				}
				printf("\a[ERROR] JOIN) joininfo Recvfrom Error!\n");

				continue;
			}

			//if (!sMode) 
				printf("JOIN) JoinInfo �������.\n");

			if (!sMode)
				printf("JOIN) JoinInfo ���� ��� �ּ� : %s, ���� ��� ��Ʈ : %d\n",
				inet_ntoa(receiveAddr.sin_addr), ntohs(receiveAddr.sin_port));

			if (!sMode)
				printf("JOIN) JoinInfo���� succInfo �������.\n");

			if (!sMode)
				printf("JOIN) JoinInfo���� succInfo : addr: %s, port : %d, id : %d\n",
				inet_ntoa(rpMsg.nodeInfo.addrInfo.sin_addr), ntohs(rpMsg.nodeInfo.addrInfo.sin_port), rpMsg.nodeInfo.ID);

			WaitForSingleObject(hMutex, INFINITE);
			rpNode = rpMsg.nodeInfo;
			myNode.chordInfo.fingerInfo.finger[0] = rpMsg.nodeInfo;
			ReleaseMutex(hMutex);

			/*proRecvThread ����*/
			procRecvThread[0] = (HANDLE)_beginthreadex(NULL, 0, (void*)procRecvMsg, (void*)&exitFlag, 0, NULL);
			
			if (!sMode)
				printf("Join ��� proRecvThread ����!!\n");

			rqMsg.msgType = REQUEST;
			rqMsg.msgID = MOVEKEYSCOMMAND;
			rqMsg.nodeInfo = myNode.nodeInfo;
			rqMsg.moreInfo = 0;
			rqMsg.bodySize = 0;
			//	printf("rqMsg->addrInfo : %s\nMsg->addr_port : %d\n", inet_ntoa(rqMsg.nodeInfo.addrInfo.sin_addr), ntohs(rqMsg.nodeInfo.addrInfo.sin_port));

			if (!sMode)
				printf("my succ : %d\n", myNode.chordInfo.fingerInfo.finger[0].ID);
		
			/*Succ���� movekeys ��û*/

			//if (!sMode)
				printf("JOIN) movekeys ��û��.\n");
		
			retval = sendto(rqSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr *) &rpNode.addrInfo, AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] JOIN) movekeys REQUEST timed out!\n");
					continue;
				}

				printf("\a[ERROR] JOIN) movekeys sendto Error!\n");
				continue;
			}

			if (!sMode)
				printf("JOIN) movekeys waiting...\n");

			retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr *) &receiveAddr, &AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] JOIN) movekeys Receive timed out!\n");

					continue;
				}
				printf("\a[ERROR] JOIN) movekeys recvfrom Error!\n");

				continue;
			}
			//if (!sMode)
				printf("JOIN) movekeys �������.\n");
			
			/*ó��*/

			keyCounts = rpMsg.moreInfo;
			printf("JOIN) keyCounts : %d\n", keyCounts);

			for (int i = 0; i < keyCounts; i++)
			{
				retval = recvfrom(rqSock, (char*)&rpFileRef, sizeof(rpFileRef), 0, (struct sockaddr *) &receiveAddr, &AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] JOIN) movekeys body Receive timed out!\n");

						continue;
					}
					printf("\a[ERROR] JOIN) movekeys body recvfrom Error!\n");

					continue;
				}

				//if (!sMode)
					printf("JOIN) movekeys body �������.\n");

				//myNode.fileInfo.fileRef[myNode.fileInfo.fileNum] = rpFileRef;
				myNode.chordInfo.FRefInfo.fileRef[myNode.chordInfo.FRefInfo.fileNum] = rpFileRef;
				myNode.chordInfo.FRefInfo.fileRef[myNode.chordInfo.FRefInfo.fileNum].refOwner = myNode.nodeInfo;
				myNode.chordInfo.FRefInfo.fileNum++;
			}

			/*PRE_INFO Req �޽��� ����*/
			memset(&rqMsg, 0, sizeof(chordHeaderType));

			/*PRE_Info Req �޽��� ���ڵ�*/
			rqMsg.msgID = PREINFOCOMMAND;
			rqMsg.msgType = REQUEST;
			rqMsg.nodeInfo = myNode.nodeInfo;
			rqMsg.moreInfo = 0;
			rqMsg.bodySize = 0;

			//if (!sMode)
				printf("JOIN) Succ���� PredInfo ��û��.\n");
			
			retval = sendto(rqSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr *) &myNode.chordInfo.fingerInfo.finger[0].addrInfo, AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] JOIN) predinfo REQUEST timed out!\n");

					continue;
				}

				printf("\a[ERROR] JOIN) predinfo Sendto Error!\n");
				continue;
			}
			if (!sMode)
				printf("JOIN)  predinfo waiting...\n");

			memset((char*)&rpMsg, 0, sizeof(rpMsg));
			retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr *) &receiveAddr, &AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] JOIN) predinfo Receive timed out!\n");

					continue;
				}
				printf("\a[ERROR] JOIN) predinfo Recvfrom Error!\n");
				continue;
			}
			//if (!sMode)
				printf("JOIN) PreInfo �������.\n");

			if (!sMode)
				printf("JOIN) SuccInfo�� ���� PreInfo : addr: %s, port : %d, id : %d\n",
				inet_ntoa(rpMsg.nodeInfo.addrInfo.sin_addr), ntohs(rpMsg.nodeInfo.addrInfo.sin_port), rpMsg.nodeInfo.ID);

			WaitForSingleObject(hMutex, INFINITE);
			myNode.chordInfo.fingerInfo.Pre = rpMsg.nodeInfo;
			ReleaseMutex(hMutex);
			/*PRE���� SUCCESSOR_UPDATE ���� !!!*/

			/*SUCC_UPDATE �޽��� ����*/
			memset(&rqMsg, 0, sizeof(chordHeaderType));

			/*PRE_Info Req �޽��� ���ڵ�*/
			rqMsg.msgID = SUCCUPDATECOMMAND;
			rqMsg.msgType = REQUEST;
			rqMsg.nodeInfo = myNode.nodeInfo;
			rqMsg.moreInfo = 0;
			rqMsg.bodySize = 0;

			//if (!sMode)
				printf("JOIN) Pred���� SUCCUPDATE ��û��.\n");
		
			retval = sendto(rqSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr *) &myNode.chordInfo.fingerInfo.Pre.addrInfo, AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] JOIN) succupdate REQUEST timed out!\n");

					continue;
				}
				printf("\a[ERROR] JOIN) succupdate Sendto Error!\n");
				continue;
			}
		
			if (!sMode) 
				printf("JOIN) succupdate waiting...\n");

			memset((char*)&rpMsg, 0, sizeof(rpMsg));
			retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr *) &receiveAddr, &AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] JOIN) succupdate Receive timed out!\n");

					continue;
				}
				printf("\a[ERROR] JOIN) succupdate Recvfrom Error!\n");
				continue;
			}

			//if (!sMode) 
				printf("JOIN) Pred���� succupdate �������.\n");


			/*SUCC���� PRE_UPDATE ��û*/

			/*PRE UPDATE �޽��� ����*/
			memset(&rqMsg, 0, sizeof(chordHeaderType));

			/*PRE_Info Req �޽��� ���ڵ�*/
			rqMsg.msgID = PREUPDATECOMMAND;
			rqMsg.msgType = REQUEST;
			rqMsg.nodeInfo = myNode.nodeInfo;
			rqMsg.moreInfo = 0;
			rqMsg.bodySize = 0;

			//if (!sMode)	
				printf("JOIN) SUCC���� predupdate ��û��.\n");
			
			retval = sendto(rqSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr *) &myNode.chordInfo.fingerInfo.finger[0].addrInfo, AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] JOIN) predupdate REQUEST timed out!\n");

					continue;
				}
				printf("\a[ERROR] JOIN) predupdate Sendto Error!\n");
				continue;
			}
			if (!sMode)
				printf("JOIN) predupdate waiting...\n");
		
			memset((char*)&rpMsg, 0, sizeof(rpMsg));
			retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr *) &receiveAddr, &AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] JOIN) predupdate Receive timed out!\n");

					continue;
				}
				printf("\a[ERROR] JOIN) predupdate Recvfrom Error!\n");
				continue;
			}
			
			//if (!sMode)
				printf("JOIN) predupdate SUCC���� preupdate �������.\n");

			fixFingerThread = (HANDLE)_beginthreadex(NULL, 0, (void*)procFixFinger, (void*)&exitFlag, 0, NULL);
			pingPongThread = (HANDLE)_beginthreadex(NULL, 0, (void*)procPingPong, (void*)&exitFlag, 0, NULL);
			fileListenThread = (HANDLE)_beginthreadex(NULL, 0, (void*)procFileListen, (void*)&exitFlag, 0, NULL);

			joinFlag = 1;
			break;

			/*Leave �ҽ� ���� !!!*/
		case 'L':
		case 'l':
			printf("���긦 �����մϴ�.\n");
			leaveKeysThread = (HANDLE)_beginthreadex(NULL, 0, (void*)procLeaveKeys, (void*)&exitFlag, 0, NULL);
			//���⼭ �ڽ��� ������������ ���θ� 
			//���������� �����
			//�����ϰ� ������ �ű��

			/*1203 ���� �ҽ�*/
			//printf("Leave ����!!!\n");
			//for (int i = 0; i< (int)myNode.fileInfo.fileNum; i++)
			//{
			//	if (myNode.fileInfo.fileRef[i].refOwner.ID == myNode.nodeInfo.ID)
			//	{
			//		memset(&rqMsg, 0, sizeof(chordHeaderType));

			//		rqMsg.msgID = LEAVEKEYSCOMMAND;
			//		rqMsg.msgType = REQUEST;
			//		rqMsg.nodeInfo = myNode.nodeInfo;
			//		rqMsg.moreInfo = 0;
			//		rqMsg.bodySize = 0;
			//		rqMsg.fileInfo = myNode.fileInfo.fileRef[i];

			//		if (!sMode)	printf("JOIN) SUCC���� leavekeys ��û��.\n");
			//		retval = sendto(rqSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&myNode.chordInfo.fingerInfo.finger[0].addrInfo, AddrLen);
			//		if (retval == SOCKET_ERROR) {
			//			if (WSAGetLastError() == WSAETIMEDOUT) {
			//				printf("\a[ERROR] JOIN)leavekeys REQUEST timed out!\n");

			//				continue;
			//			}
			//			printf("\a[ERROR] JOIN) leavekeys Sendto Error!\n");
			//			continue;
			//		}

			//		if(!sMode)	printf("JOIN) leavekeys waiting..\n");
			//		retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr *) &receiveAddr, &AddrLen);
			//		
			//		if (retval == SOCKET_ERROR) {
			//			if (WSAGetLastError() == WSAETIMEDOUT) {
			//				printf("\a[ERROR] JOIN)leavekeys REQUEST timed out!\n");

			//				continue;
			//			}
			//			printf("\a[ERROR] JOIN) leavekeys Sendto Error!\n");
			//			continue;
			//		}
			//		//���⼱ �ű��
			//	}
			//	else
			//	{

			//		memset(&rqMsg, 0, sizeof(chordHeaderType));
			//		rqMsg.msgType = REQUEST;
			//		rqMsg.msgID = FILEDELETECOMMAND;
			//		rqMsg.nodeInfo = myNode.nodeInfo;
			//		rqMsg.moreInfo = 0;
			//		rqMsg.bodySize = 0;
			//		rqMsg.fileInfo = myNode.fileInfo.fileRef[i];

			//		sendto(rqSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&myNode.fileInfo.fileRef[i].refOwner, AddrLen);
			//		//����°� �״�� ���
			//		//���⼱ �����

			//	}
			//}
			//���� ����ȭ�� �϶�� ��ȣ�� ������ ������ ����

			break;

			/*Add �ҽ� ���� !!!*/
		case 'A':
		case 'a':

			memset(buf, 0x00, 1024);

			if (!sMode)
				printf("���� �߰� ����!!!\n");
			
			printf("�߰��� ���� �̸� : \n");
			scanf("%s", buf);
			if (NULL == fopen(buf, "r"))
			{
				printf("�׷��� ������ �������� �ʽ��ϴ�.\n");
				continue;
			}

			myNode.fileInfo.fileNum++;
			strcpy(myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Name, buf);
			myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Key = str_hash(buf);
			int fingerId = myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Key;
			myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].owner = myNode.nodeInfo;

			//int AddrLen = sizeof(struct sockaddr_in);
			//int retval;
			////nodeInfoType recvNode;
			//struct sockaddr_in receiveAddr;
			//chordHeaderType rqMsg, rpMsg;
			//�ӽ÷� �̺κп����� ����ϵ��� �����س���

			/*Succ����  fingerID(����Ű) findpred ��û*/
			memset(&rqMsg, 0, sizeof(chordHeaderType));
			rqMsg.msgType = REQUEST;
			rqMsg.msgID = FINDPRECOMMAND;
			rqMsg.nodeInfo = myNode.nodeInfo;
			rqMsg.moreInfo = fingerId;
			rqMsg.bodySize = 0;

			//if (!sMode)	
				printf("FILE ADD) SUCC���� fingerID(����Ű)findpred ��û��.\n");

			retval = sendto(rqSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&myNode.chordInfo.fingerInfo.finger[0].addrInfo, AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] FILE ADD) findpred REQUEST timed out!\n");

					continue;
				}
				printf("\a[ERROR]  FILE ADD) findpred Sendto Error!\n");
				continue;
			}


			if (!sMode)
				printf("FILE ADD) findpred waiting...\n");

			retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] FILE ADD) findpred Receive timed out!\n");

					continue;
				}
				printf("\a[ERROR] FILE ADD) findpred Recvfrom Error!\n");
				continue;
			}

			//if (!sMode)
				printf("FILE ADD) SUCC���� findpred �������.\n");
			
			/*pre���� ����Ű�� Successor ��û*/
			//���� ��尡 �ڽ��� ��û�ѳ���ϰ�� �׸��� �ڽ��� ���ϴ� �������� Ÿ���ϰ�� 
			memset(&rqMsg, 0, sizeof(chordHeaderType));
			rqMsg.msgType = REQUEST;
			rqMsg.msgID = SUCCINFOCOMMAND;
			rqMsg.nodeInfo = myNode.nodeInfo;
			rqMsg.moreInfo = 0;
			rqMsg.bodySize = 0;

			//if (!sMode)	
				printf("FILE ADD) succinfo ��û��.\n");

			retval = sendto(rqSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&rpMsg.nodeInfo.addrInfo, AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] FILE ADD) succinfo REQUEST timed out!\n");

					continue;
				}
				printf("\a[ERROR] FILE ADD) succinfo Sendto Error!\n");
				continue;
			}

			if (!sMode)
				printf("FILE ADD) succinfo waiting...\n");

			retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] FILE ADD) succinfo Receive timed out!\n");

					continue;
				}
				printf("\a[ERROR] FILE ADD) succinfo recvfrom Error!\n");
				continue;
			}
			//if (!sMode)
				printf("FILE ADD) succinfo �������.\n");

			if (rpMsg.nodeInfo.ID == myNode.nodeInfo.ID)	//myNode�� ������ Succ�϶�
			{
				myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].refOwner = myNode.nodeInfo;
				//������ �κ�
				myNode.chordInfo.FRefInfo.fileNum++;
				myNode.chordInfo.FRefInfo.fileRef[myNode.chordInfo.FRefInfo.fileNum - 1].Key = myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Key;
				myNode.chordInfo.FRefInfo.fileRef[myNode.chordInfo.FRefInfo.fileNum - 1].refOwner = myNode.nodeInfo;
				myNode.chordInfo.FRefInfo.fileRef[myNode.chordInfo.FRefInfo.fileNum - 1].owner = myNode.nodeInfo;
				strcpy(myNode.chordInfo.FRefInfo.fileRef[myNode.chordInfo.FRefInfo.fileNum - 1].Name, myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Name);

				printf("FILE ADD) file add ������ : %d\n", rpMsg.nodeInfo.ID);
				printf("FILE ADD) ���� add �Ϸ��!\n �����̸�: %s ����Ű: %d\n", myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Name, myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Key);
	
			}
			else
			{
				myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].refOwner = rpMsg.nodeInfo;

				memset(&rqMsg, 0, sizeof(chordHeaderType));
				rqMsg.msgType = REQUEST;
				rqMsg.msgID = FILEADDCOMMAND;
				rqMsg.nodeInfo = myNode.nodeInfo;
				rqMsg.moreInfo = 0;
				rqMsg.bodySize = 0;
				rqMsg.fileInfo = myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1];

				//if (!sMode)
					printf("FILE ADD) fileadd ����!!!\n");
				/*file�� Succ���� File Add ��û*/
				retval = sendto(rqSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&rpMsg.nodeInfo.addrInfo, AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] FILE ADD) fileadd REQUEST timed out!\n");

						continue;
					}
					printf("\a[ERROR] FILE ADD) fileadd sendto Error!\n");
					continue;
				}
				if (!sMode)
					printf("FILE ADD) fileadd waiting...\n");

				retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] FILE ADD) fileadd RECEIVE timed out!\n");

						continue;
					}
					printf("\a[ERROR] FILE ADD) fileadd recvfrom Error!\n");
					continue;
				}

				//if (!sMode)
					printf("FILE ADD) file add �������!!!\n");

				//üũ���� �ʿ�
				/*printf("���� add �Ϸ�!\n���� �̸�: %s Ű: %d, ������ : %d, ����(SUCC) : %d \n",
					myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Name, myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Key,
					myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].owner.ID, myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].refOwner.ID);
			}*/
				printf("FILE ADD) file add ������ : %d\n", rpMsg.nodeInfo.ID);
				printf("FILE ADD) ���� add �Ϸ��!\n �����̸�: %s ����Ű: %d\n", myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Name, myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Key);
			}

				//�ڽ��� �����Ҷ��� �ڽ��� ������ �� ������ ������ ������ ���� �ִµ�

				//���� ���� �����带 ũ������Ʈ�� �������������� ������ �� ����
				//������ ���� �߰����ڸ��� �����尡 ����Ǵ°��� ���� ��������
				//���� id�߰� 
				//id����Ͽ� succ �˾Ƴ��� �������� �߰���
				//�ڽ��� ����ü�� ���������߰�
				//������ ���� �������Ƿ� file send thread ���� 
				//2018 11 07

			printFileTable();

				break;

				/*Delete �ҽ� ���� !!!*/
		case 'D':
		case 'd':
			printf("�����Ͻ� ����Ű :\n");
			//�ؽ��Լ� �ð� �Һ����� �ٲٸ� �̸����� �Է¹���
			//ó���ؽ��Ҷ��� �ּҶ� ��Ʈ��ȣ strcat�ؼ� Ű ���
			scanf("%d", &file_key_temp);
			flag = 0;
			//�ڽ��� �������� ���� ��������?
			//or �ڽ��� �������� �����
			//�ڿ��ִ� ���̺� ������ ���ܿ;���
			for (int i = 0; i < (int)(myNode.fileInfo.fileNum); i++)
			{
				if (file_key_temp == myNode.fileInfo.fileRef[i].Key) {
					flag = 1;
					//�ڿ� �ִ� ���̺� ���ܿ;ߵ�
					//owner�� ��쿡�� �����ϰ� refowner���� ������û
					//owner üũ ��� : ��Ʈ��ȣ??
					//refowner �ϰ�� �������� ����

					/*fileRef���� FILEDELETE ��û*/
					memset(&rqMsg, 0, sizeof(chordHeaderType));
					rqMsg.msgType = REQUEST;
					rqMsg.msgID = FILEDELETECOMMAND;
					rqMsg.nodeInfo = myNode.nodeInfo;
					rqMsg.moreInfo = 0;
					rqMsg.bodySize = 0;
					rqMsg.fileInfo = myNode.fileInfo.fileRef[i];

					//if (!sMode) 
						printf("file delete) FileDelete ��û!!!\n");

					retval = sendto(rqSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&myNode.fileInfo.fileRef[i].refOwner.addrInfo, AddrLen);
					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] REQUEST timed out!\n");

							continue;
						}
						printf("\a[ERROR] Sendto Error!\n");
						continue;
					}

					if (!sMode)
						printf("file delete) FileDelete wating...\n");

					retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
					if (i != myNode.fileInfo.fileNum - 1)
					{
						for (int j = i + 1; j < (int)myNode.fileInfo.fileNum; j++)
						{
							myNode.fileInfo.fileRef[j - 1] = myNode.fileInfo.fileRef[j];
						}

						//�迭�� ���� ���� ���� ���� ������ �о���� 
					}

					//if (!sMode)
						printf("file delete) FileDelete �������!!!\n");
					
					myNode.fileInfo.fileNum--;
				}
			}

			if (!flag)
				printf("you haven't own file. \n");

			break;

			/*Search �ҽ� ���� !!!*/
		case 'S':
		case 's':
			flag = 0;
			memset(buf, 0x00, 1024);
			printf("ã���� ���� Ű�� �Է��Ͻÿ�. :\n");
			scanf("%d", &file_key_temp);
			//scanf("%s", buf);
			//file_key_temp = str_hash(buf);
			//			printf("�ؽ̵� ����Ű : %d", file_key_temp);

			printf("���� �����ִ� ������ �� : %d\n", myNode.fileInfo.fileNum);
			printf("���� �����ϰ� �ִ� ������ �� : %d\n", myNode.chordInfo.FRefInfo.fileNum);

			for (int i = 0; i < (int)myNode.chordInfo.FRefInfo.fileNum; i++)
			{
				
				printf("i : %d, filetable : %d\n", i, myNode.fileInfo.fileRef[i].Key);
				
				if (myNode.chordInfo.FRefInfo.fileRef[i].Key == file_key_temp) //���� ���� ������ ���� �� 
				{
					printf("���������� �ڽ��� �����ֽ��ϴ�.\n");
					memset(&rqMsg, 0, sizeof(chordHeaderType));
					rqMsg.msgType = REQUEST;
					rqMsg.msgID = FILEDOWNCOMMAND;
					rqMsg.nodeInfo = myNode.nodeInfo;
					rqMsg.moreInfo = file_key_temp;
					rqMsg.bodySize = 0;

				//	if (!sMode)
						printf("file search) FILEDOWN ��û!!!\n");
					/*Ref���� fileDown ��û*/
					retval = sendto(rqSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&myNode.fileInfo.fileRef[i].owner.addrInfo, AddrLen);
					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] file search) FILEDOWN REQUEST timed out!\n");

							continue;
						}
						printf("\a[ERROR] file search) FILEDOWN Sendto Error!\n");
						continue;
					}

					if (!sMode) 
						printf("file search) FILEDOWN waiting...\n");

					retval = recvfrom(rqSock, (char*)&sharing.msg, sizeof(sharing.msg), 0, (struct sockaddr*)&sharing.senderSockAddr, &AddrLen);
					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] file search) FILEDOWN RECEVIE timed out!\n");

							continue;
						}
						printf("\a[ERROR] file search) FILEDOWN recvfrom Error!\n");
						continue;
					}

				//	if (!sMode)
						printf("file search) FILEDOWN ���� ����!!!\n");

					//sharing.msg.nodeInfo.addrInfo = receiveAddr;
					fileReceiveThread = (HANDLE)_beginthreadex(NULL, 0, (void*)procFileRecv, (void*)&exitFlag, 0, NULL);
					flag = 1;
					break;
				}
				break;

			}
			if (flag == 0)	//���������� ���� ��
			{
				/*file_key�� pre ��û*/
				memset(&rqMsg, 0, sizeof(rqMsg));
				rqMsg.msgType = REQUEST;
				rqMsg.msgID = FINDPRECOMMAND;
				rqMsg.nodeInfo = myNode.nodeInfo;
				rqMsg.moreInfo = file_key_temp;
				rqMsg.bodySize = 0;

			//	if (!sMode) 
					printf("file search) FINDPRED ��û!!!\n");
			
					retval = sendto(rqSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&myNode.chordInfo.fingerInfo.finger[0].addrInfo, AddrLen);
				
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] file search) FILEDOWN Request timed out!\n");

						continue;
					}
					printf("\a[ERROR] file search) FILEDOWN Sendto Error!\n");
					continue;
				}

				if (!sMode)
					printf("file search) FINDPRED Waiting...\n");

				retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
				//���� ��尡 �ڽ��� ��û�ѳ���ϰ�� �׸��� �ڽ��� ���ϴ� �������� Ÿ���ϰ�� 
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] file search) FINDPRED RECEVIE timed out!\n");

						continue;
					}
					printf("\a[ERROR] file search) FILEDOWN recvfrom Error!\n");
					continue;
				}

			 //	if (!sMode)
					printf("file search) FILEDOWN �������!!!\n");
				
				/*filekey�� Succ ��û*/
				memset(&rqMsg, 0, sizeof(chordHeaderType));
				rqMsg.msgType = REQUEST;
				rqMsg.msgID = SUCCINFOCOMMAND;
				rqMsg.nodeInfo = myNode.nodeInfo;
				rqMsg.moreInfo = 0;
				rqMsg.bodySize = 0;

				//if (!sMode)
					printf("file search) SUCCINFO ��û!!!\n");

				retval = sendto(rqSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&rpMsg.nodeInfo.addrInfo, AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] file search) REQUEST timed out!\n");

						continue;
					}
					printf("\a[ERROR] file search) sendto Error!\n");
					continue;
				}

				if (!sMode)
					printf("file search) SUCCINFO Wainting...\n");

				retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] file search) RECEVIE timed out!\n");

						continue;
					}
					printf("\a[ERROR] file search) recvfrom Error!\n");
					continue;
				}

				//if (!sMode)
					printf("file search) SUCCINFO �������!!!\n");

				printf("���� �����ڸ� ã�ҽ��ϴ�. : %d\n", rpMsg.nodeInfo.ID);
				/*���� �����ڿ��� FILEREF ��û�ؼ� Fileinfo ���� */
				memset(&rqMsg, 0, sizeof(chordHeaderType));
				rqMsg.msgType = REQUEST;
				rqMsg.msgID = FILEREFCOMMAND;
				rqMsg.nodeInfo = myNode.nodeInfo;
				rqMsg.moreInfo = file_key_temp;
				rqMsg.bodySize = 0;

				//if (!sMode)
					printf("file search) FILEREFINFO ��û!!!\n");

				retval = sendto(rqSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&rpMsg.nodeInfo.addrInfo, AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] file search) REQUEST timed out!\n");

						continue;
					}
					printf("\a[ERROR] file search) sendto Error!\n");
					continue;
				}
				if (!sMode)
					printf("file search) FILEREFINFO waiting...\n");

				retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] file search) RECEVIE timed out!\n");

						continue;
					}
					printf("\a[ERROR] file search) recvfrom Error!\n");
					continue;
				}
				//if (!sMode)
					printf("file search) FILEREFINFO �������!!!\n");

				printf("���� �����ڸ� ã�ҽ��ϴ�. %d\n", rpMsg.nodeInfo.ID);

				/*file_owner���� FILEDOWN ��û*/
				memset(&rqMsg, 0, sizeof(chordHeaderType));
				rqMsg.msgType = REQUEST;
				rqMsg.msgID = FILEDOWNCOMMAND;
				rqMsg.nodeInfo = myNode.nodeInfo;
				rqMsg.moreInfo = file_key_temp;
				rqMsg.bodySize = 0;

				//if (!sMode)
					printf("file search) FILEDOWN ��û!!!\n");

				retval = sendto(rqSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&rpMsg.fileInfo.owner.addrInfo, AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] file search) REQUEST timed out!\n");

						continue;
					}
					printf("\a[ERROR] file search) Sendto Error!\n");
					continue;
				}

				if (!sMode)
					printf("file search) FILEDOWN waiting...\n");
				
				retval = recvfrom(rqSock, (char*)&sharing.msg, sizeof(sharing.msg), 0, (struct sockaddr*)&sharing.senderSockAddr, &AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] file search) RECEVIE timed out!\n");

						continue;
					}
					printf("\a[ERROR] file search) recvfrom Error!\n");
					continue;
				}
				//if (!sMode)
					printf("file search) FILEDOWN �������!!!\n");

				fileReceiveThread = (HANDLE)_beginthreadex(NULL, 0, (void*)procFileRecv, (void*)&exitFlag, 0, NULL);
			}
			//���� ���⼭ ����ũ�⸦ �ް� ���� ���ú� �����带 �����ϸ� ��
			break;

			/*Finger �ҽ� ���� !!!*/
		case 'F':
		case 'f':
			printf("Finger table Information:\n");
			printf("My IP Address: %s, Port No: %d, ID: %d\n",
				inet_ntoa(myNode.nodeInfo.addrInfo.sin_addr), ntohs(myNode.nodeInfo.addrInfo.sin_port), myNode.nodeInfo.ID);

			printf("Pre IP Addr : %s, Port No : %d, ID : %d\n",
				inet_ntoa(myNode.chordInfo.fingerInfo.Pre.addrInfo.sin_addr),
				ntohs(myNode.chordInfo.fingerInfo.Pre.addrInfo.sin_port), myNode.chordInfo.fingerInfo.Pre.ID);
			for (int i = 0; i < baseM; i++)
				printf("Finger[%d] IP Addr : %s, Port No : %d, ID : %d\n",
					i, inet_ntoa(myNode.chordInfo.fingerInfo.finger[i].addrInfo.sin_addr),
					ntohs(myNode.chordInfo.fingerInfo.finger[i].addrInfo.sin_port), myNode.chordInfo.fingerInfo.finger[i].ID);
			break;
			/*Info �ҽ� ���� !!!*/

		case 'I':
		case 'i':

			printFileTable();
			break;

			/*Mute �ҽ� ���� !!!*/
		case 'M':
		case 'm':
			if (sMode == 1) {
				sMode = 0;
				printf("sMode is Off!!!\n");
			}
			else {
				sMode = 1;
				printf("sMode is On!!!\n");
			}
			break;

			/*Help �ҽ� ���� !!!*/
		case 'H':
		case 'h':
			printHelpCommand();
			break;

			/*Quit �ҽ� ���� !!!*/
		case 'Q':
		case 'q':
			printf(" ------------------------------------------------------------\n");
			printf("|                         Good Bye                           |\n");
			printf(" ------------------------------------------------------------\n");
			exit(1);
			break;

			}
		}
		//WaitForSingleObject(procRecvMsg, INFINITE);
		closesocket(rpSock);
		closesocket(rqSock);
		WSACleanup();

	}
	//���� �ٿ����Ʈ �ö��� ���� ���������带 ������.

	int modIn(int modN, int targNum, int range1, int range2, int leftmode, int rightmode)
		// leftmode, rightmode: 0 => range boundary not included, 1 => range boundary included   
	{
		int result = 0;

		if (range1 == range2) {
			if ((leftmode == 0) || (rightmode == 0))
				return 0;
		}

		if (modPlus(ringSize, range1, 1) == range2) {
			if ((leftmode == 0) && (rightmode == 0))
				return 0;
		}

		if (leftmode == 0)
			range1 = modPlus(ringSize, range1, 1);
		if (rightmode == 0)
			range2 = modMinus(ringSize, range2, 1);

		if (range1 < range2) {
			if ((targNum >= range1) && (targNum <= range2))
				result = 1;
		}
		else if (range1 > range2) {
			if (((targNum >= range1) && (targNum < modN))
				|| ((targNum >= 0) && (targNum <= range2)))
				result = 1;
		}
		else if ((targNum == range1) && (targNum == range2))
			result = 1;

		return result;
	}

	int twoPow(int power)
	{
		int i;
		int result = 1;

		if (power >= 0)
			for (i = 0; i < power; i++)
				result *= 2;
		else
			result = -1;

		return result;
	}

	int modMinus(int modN, int minuend, int subtrand)
	{
		if (minuend - subtrand >= 0)
			return minuend - subtrand;
		else
			return (modN - subtrand) + minuend;
	}

	int modPlus(int modN, int addend1, int addend2)
	{
		if (addend1 + addend2 < modN)
			return addend1 + addend2;
		else
			return (addend1 + addend2) - modN;
	}
	static const unsigned char sTable[256] =
	{
		0xa3,0xd7,0x09,0x83,0xf8,0x48,0xf6,0xf4,0xb3,0x21,0x15,0x78,0x99,0xb1,0xaf,0xf9,
		0xe7,0x2d,0x4d,0x8a,0xce,0x4c,0xca,0x2e,0x52,0x95,0xd9,0x1e,0x4e,0x38,0x44,0x28,
		0x0a,0xdf,0x02,0xa0,0x17,0xf1,0x60,0x68,0x12,0xb7,0x7a,0xc3,0xe9,0xfa,0x3d,0x53,
		0x96,0x84,0x6b,0xba,0xf2,0x63,0x9a,0x19,0x7c,0xae,0xe5,0xf5,0xf7,0x16,0x6a,0xa2,
		0x39,0xb6,0x7b,0x0f,0xc1,0x93,0x81,0x1b,0xee,0xb4,0x1a,0xea,0xd0,0x91,0x2f,0xb8,
		0x55,0xb9,0xda,0x85,0x3f,0x41,0xbf,0xe0,0x5a,0x58,0x80,0x5f,0x66,0x0b,0xd8,0x90,
		0x35,0xd5,0xc0,0xa7,0x33,0x06,0x65,0x69,0x45,0x00,0x94,0x56,0x6d,0x98,0x9b,0x76,
		0x97,0xfc,0xb2,0xc2,0xb0,0xfe,0xdb,0x20,0xe1,0xeb,0xd6,0xe4,0xdd,0x47,0x4a,0x1d,
		0x42,0xed,0x9e,0x6e,0x49,0x3c,0xcd,0x43,0x27,0xd2,0x07,0xd4,0xde,0xc7,0x67,0x18,
		0x89,0xcb,0x30,0x1f,0x8d,0xc6,0x8f,0xaa,0xc8,0x74,0xdc,0xc9,0x5d,0x5c,0x31,0xa4,
		0x70,0x88,0x61,0x2c,0x9f,0x0d,0x2b,0x87,0x50,0x82,0x54,0x64,0x26,0x7d,0x03,0x40,
		0x34,0x4b,0x1c,0x73,0xd1,0xc4,0xfd,0x3b,0xcc,0xfb,0x7f,0xab,0xe6,0x3e,0x5b,0xa5,
		0xad,0x04,0x23,0x9c,0x14,0x51,0x22,0xf0,0x29,0x79,0x71,0x7e,0xff,0x8c,0x0e,0xe2,
		0x0c,0xef,0xbc,0x72,0x75,0x6f,0x37,0xa1,0xec,0xd3,0x8e,0x62,0x8b,0x86,0x10,0xe8,
		0x08,0x77,0x11,0xbe,0x92,0x4f,0x24,0xc5,0x32,0x36,0x9d,0xcf,0xf3,0xa6,0xbb,0xac,
		0x5e,0x6c,0xa9,0x13,0x57,0x25,0xb5,0xe3,0xbd,0xa8,0x3a,0x01,0x05,0x59,0x2a,0x46
	};

#define PRIME_MULT 1717

	unsigned int str_hash(const char *str)  /* Hash: String to Key */
	{
		unsigned int len = sizeof(str);
		unsigned int hash = len, i;
		//shuffle_hash_table();
		srand((unsigned int)time(NULL));

		/*������ Chord �ùķ����� �ҽ� �ϴ� �� (IP�� �ؽ������� rand() �߰�����)*/
		for (i = 0; i != len; i++, str++) {
			hash ^= sTable[rand() % (*str + i) & 255];
			hash = hash * PRIME_MULT;
		}
		//suha : ��� ����� ���� ������ Ȯ�� , ������ �ؽ̰��� ��������

		return hash % ringSize;
	}


