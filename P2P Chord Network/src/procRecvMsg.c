#include "header.c"

unsigned WINAPI procRecvMsg(void *arg)
{

	if (!sMode)
		printf("procRecvMsg ����!!!\n");
	
	int *exitFlag = (int*)arg;
	int AddrLen = sizeof(struct sockaddr_in);
	int retval;
	chordHeaderType Msg;
	chordHeaderType rpMsg;
	msgInfoType msgInfo;
	fileRefType fileRefList[FileMax];
	int keysCount = 0;
	int clientID = 0;

	FILE *fp;
	int temp_file_index;

	/*Bind*/
	if (-1 == bind(rpSock, (struct sockaddr*)&myNode.nodeInfo.addrInfo, sizeof(struct sockaddr)))
	{
		perror("procRecv) bind() error!");
		exit(1);
	}

	while (!(*exitFlag)) {

		memset((char *)&Msg, 0, sizeof(Msg));
		memset((char *)&msgInfo, 0, sizeof(msgInfo));
		/*��û �޽��� ���� ���*/
		retval = recvfrom(rpSock, (char *)&msgInfo.msg, sizeof(msgInfo.msg), 0, (struct sockaddr*)&msgInfo.senderSockAddr, &AddrLen);
		
		if (!sMode)	
			printf("procRecvMsg) Msg Addr : %s, port : %d\n", inet_ntoa(msgInfo.msg.nodeInfo.addrInfo.sin_addr), ntohs(msgInfo.msg.nodeInfo.addrInfo.sin_port));

		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("CHORD> procRecvMsg recvfrom timed out.\n");
			}
			else {
				printf("CHORD> error %d\n", WSAGetLastError());
			}
			continue;
		}
		//		printf("senderAddr : %s\nport : %d\n", inet_ntoa(senderAddr.sin_addr), ntohs(senderAddr.sin_port));		
				/*��û �޽��� Ÿ�� Ȯ��*/
		if (Msg.msgType == REQUEST) { // ��û �޽����̸�
		
			if (!sMode)	
				printf("��û �޽��� ��û����.\n");

			switch (msgInfo.msg.msgID)
			{

			case JOINCOMMAND: // ��û �޽����� JOIN�̸�

				//if (!sMode)
					printf("procRecv) JOIN request ����.\n");
			
				joinThread = (HANDLE)_beginthreadex(NULL, 0, (void*)procJoin, (void *)&msgInfo, 0, NULL);
				WaitForSingleObject(joinThread, INFINITE);

				break;

			case FINDPRECOMMAND:
				
				if (!sMode)
					printf("procRecv) FINDPRED request ����.\n");
				
				findPreThread = (HANDLE)_beginthreadex(NULL, 0, (void*)procFindPred, (void *)&msgInfo, 0, NULL);
				WaitForSingleObject(findPreThread, INFINITE);

				break;

			case SUCCINFOCOMMAND:
				
				if (!sMode)
					printf("procRecv) SUCCINFO request ����.\n");
				
				memset(&rpMsg, 0, sizeof(rpMsg));
				/*���� �޽��� ���ڵ�*/
				rpMsg.msgType = RESPOND; // respond
				rpMsg.msgID = SUCCINFOCOMMAND;   // JoinInfo
				rpMsg.nodeInfo = myNode.chordInfo.fingerInfo.finger[0];
				rpMsg.moreInfo = SUCCESS;
				rpMsg.bodySize = 0;
				if (!sMode)		printf("procRecv) SUCCINFO �޽��� ����.\n");

				retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr *) &msgInfo.senderSockAddr, AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] procRecv) SUCCINFO REQUEST timed out!\n");
						return -1;
					}
					printf("\a[ERROR] procRecv) SUCCINFO Sendto Error!\n");
					return -1;
				}

				WaitForSingleObject(findPreThread, INFINITE);
				break;

			case PREINFOCOMMAND:
				
				if (!sMode)
					printf("procRecv) PREINFO request ����.\n");
			
				memset(&rpMsg, 0, sizeof(rpMsg));
				/*���� �޽��� ���ڵ�*/
				rpMsg.msgType = RESPOND; // respond
				rpMsg.msgID = PREINFOCOMMAND;   // JoinInfo
				rpMsg.nodeInfo = myNode.chordInfo.fingerInfo.Pre;
				rpMsg.moreInfo = SUCCESS;
				rpMsg.bodySize = 0;

				if (!sMode)	
					printf("procRecv) PREINFO �޽��� ����.\n");

				retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr *) &msgInfo.senderSockAddr, AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] procRecv) SUCCINFO REQUEST timed out!\n");
						return -1;
					}
					printf("\a[ERROR] procRecv) SUCCINFO Sendto Error!\n");
					return -1;
				}
				WaitForSingleObject(findPreThread, INFINITE);
				break;

			case PREUPDATECOMMAND:
				
				//if (!sMode)
					printf("procRecv) PREUPDATE request ����.\n");
				
				memset(&rpMsg, 0, sizeof(rpMsg));

				WaitForSingleObject(hMutex, INFINITE);
				myNode.chordInfo.fingerInfo.Pre = msgInfo.msg.nodeInfo;
				ReleaseMutex(hMutex);

				rpMsg.msgType = RESPOND; // respond
				rpMsg.msgID = PREUPDATECOMMAND;
				rpMsg.nodeInfo = myNode.nodeInfo;
				rpMsg.moreInfo = SUCCESS;
				rpMsg.bodySize = 0;
				if (!sMode)
					printf("procRecv) PREUPDATE �޽��� ����.\n");
				retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr *)&msgInfo.senderSockAddr, AddrLen);

				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] procRecv) PREUPDATE REQUEST timed out!\n");
						return -1;
					}
					printf("\a[ERROR] procRecv) PREUPDATE Sendto Error!\n");
					return -1;
				}
				if (!sMode)
					printf("procRecv) PREUPDATE myId : %d, msgInfo.msg.nodeId : %d\n", myNode.nodeInfo.ID, msgInfo.msg.nodeInfo.ID);
				WaitForSingleObject(findPreThread, INFINITE);
				break;

			case SUCCUPDATECOMMAND:
			
				//if (!sMode)
					printf("procRecv) SUCCUPDATE request ����.\n");
			
				memset(&rpMsg, 0, sizeof(rpMsg));

				WaitForSingleObject(hMutex, INFINITE);
				myNode.chordInfo.fingerInfo.finger[0] = msgInfo.msg.nodeInfo;
				ReleaseMutex(hMutex);

				rpMsg.msgType = RESPOND;
				rpMsg.msgID = SUCCUPDATECOMMAND;   // JoinInfo
				rpMsg.nodeInfo = myNode.nodeInfo;
				rpMsg.moreInfo = SUCCESS;
				rpMsg.bodySize = 0;
				if (!sMode)	printf("procRecv) SUCCUPDATE �޽��� ����.\n");
				retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr *)&msgInfo.senderSockAddr, AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] procRecv) SUCCUPDATE REQUEST timed out!\n");
						return -1;
					}
					printf("\a[ERROR] procRecv) SUCCUPDATE Sendto Error!\n");
					return -1;
				}
				WaitForSingleObject(findPreThread, INFINITE);
				break;

			case PINGPONGCOMMAND:
				
				if (!sMode)
					printf("procRecv) PINGPONG request ����.\n");
			
				memset(&rpMsg, 0, sizeof(rpMsg));

				rpMsg.msgType = RESPOND;
				rpMsg.msgID = SUCCUPDATECOMMAND;
				rpMsg.nodeInfo = myNode.nodeInfo;
				rpMsg.moreInfo = SUCCESS;
				rpMsg.bodySize = 0;
				if (!sMode)	printf("procRecv) PINGPONG �޽��� ����.\n");
				retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr *)&msgInfo.senderSockAddr, AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] procRecv) PINGPONG REQUEST timed out!\n");
						return -1;
					}
					printf("\a[ERROR] procRecv) PINGPONG Sendto Error!\n");
					return -1;
				}
				WaitForSingleObject(findPreThread, INFINITE);
				break;

			case FILEREFCOMMAND:
				
				//if (!sMode)
					printf("procRecv) FILEREF request ����.\n");
				
				//���� ������ ����
				//���� �ٿ� Ŀ�ǵ忡�� ���� ������ �ڽ��� ���� ���� ���̺��� �����ڸ� ������
				temp_file_index = -1;

				for (int i = 0; i < (int)myNode.chordInfo.FRefInfo.fileNum; i++)
				{
					if (myNode.chordInfo.FRefInfo.fileRef[i].Key == msgInfo.msg.moreInfo)
					{
						temp_file_index = i;
						break;
					}
				}
				if (temp_file_index == -1)
				{
					perror("������ �����ϰ� ���� �ʴ�.\n");
					continue;
				}
				printf("���� ���� ���̺� �ε���:%d\n", temp_file_index);
				memset(&rpMsg, 0, sizeof(rpMsg));

				rpMsg.msgType = RESPOND;
				rpMsg.msgID = FILEREFCOMMAND;
				rpMsg.nodeInfo = myNode.nodeInfo;
				rpMsg.moreInfo = 0;
				rpMsg.fileInfo = myNode.chordInfo.FRefInfo.fileRef[temp_file_index];
				rpMsg.bodySize = 0;

				retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&msgInfo.senderSockAddr, AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] procRecv) FILEREF REQUEST timed out!\n");
						return -1;
					}
					printf("\a[ERROR] procRecv) FILEREF Sendto Error!\n");
					return -1;
				}
				WaitForSingleObject(findPreThread, INFINITE);
				//���� 
				break;

			case FILEADDCOMMAND:
				
				//if (!sMode) 
					printf("procRecv) FILEADD request ����.\n");

				myNode.chordInfo.FRefInfo.fileNum++;

				myNode.chordInfo.FRefInfo.fileRef[myNode.chordInfo.FRefInfo.fileNum - 1].Key = msgInfo.msg.fileInfo.Key;
				strcpy(myNode.chordInfo.FRefInfo.fileRef[myNode.chordInfo.FRefInfo.fileNum - 1].Name, msgInfo.msg.fileInfo.Name); // �ȵǸ� strcpy
				myNode.chordInfo.FRefInfo.fileRef[myNode.chordInfo.FRefInfo.fileNum - 1].owner = msgInfo.msg.fileInfo.owner;
				myNode.chordInfo.FRefInfo.fileRef[myNode.chordInfo.FRefInfo.fileNum - 1].refOwner = myNode.nodeInfo;

				//���� �� �߰��ƴٰ� rpmsg �������
				memset(&rpMsg, 0, sizeof(rpMsg));
				rpMsg.msgType = RESPOND;
				rpMsg.msgID = FILEADDCOMMAND;
				rpMsg.nodeInfo = myNode.nodeInfo;
				rpMsg.moreInfo = 0;
				rpMsg.bodySize = 0;

				/*��û Node���� FILLADD ����*/
				retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&msgInfo.senderSockAddr, AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] procRecv) FILEADD REQUEST timed out!\n");
						return -1;
					}
					printf("\a[ERROR] procRecv) FILEADD Sendto Error!\n");
					return -1;
				}

				//���� ������ �������� ���� ������ ������ ���� �־����
				//���� ������ : owner : �ڽ� refowner : �����ϰ� �ִ³�
				//���� ������ : owner �ڽſ��� ���� �ֵ� Ŀ�ǵ带 ������ refowner : �ڽ�
				WaitForSingleObject(findPreThread, INFINITE);
				break;

			case FILEDOWNCOMMAND:
				/* 1125 ??? ���� �̻��ѵ�? */
			
				//if (!sMode) 
					printf("procRecv) FILEDOWN request ����.\n");

				for (int i = 0; i < (int)myNode.fileInfo.fileNum; i++)
				{
					if (msgInfo.msg.moreInfo == myNode.fileInfo.fileRef[i].Key)
					{
						temp_file_index = i;
						break;
					}
				}
				fp = fopen(myNode.fileInfo.fileRef[temp_file_index].Name, "r");
				fseek(fp, 0, SEEK_END);
				rpMsg.moreInfo = (short)ftell(fp);
				printf("procRecv) file size  : %d\n", rpMsg.moreInfo);
				fclose(fp);
				memset(&rpMsg, 0, sizeof(rpMsg));
				rpMsg.msgType = RESPOND;
				rpMsg.msgID = FILEDOWNCOMMAND;
				rpMsg.nodeInfo = myNode.nodeInfo;
				rpMsg.bodySize = 0;
				/*procFileListen�� ���� �̸� �Ѱ���*/
				/*����Node���� FILEDOWN ����*/
				retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&msgInfo.senderSockAddr, AddrLen);

				sharing.msg.moreInfo = temp_file_index;
			

				//���� ���⼭ ũ�⸦ ��������?
				//���� �ٿ� Ŀ�ǵ忡�� ���� ũ�Ⱑ �����Ѵ�.
				WaitForSingleObject(findPreThread, INFINITE);
				break;

			case FILEDELETECOMMAND:
			
				//if (!sMode)
					printf("procRecv) FILEDELETE request ����.\n");

				for (int i = 0; i < (int)myNode.chordInfo.FRefInfo.fileNum; i++)
				{
					if (msgInfo.msg.fileInfo.Key == myNode.chordInfo.FRefInfo.fileRef[i].Key)
					{
						memset(&rpMsg, 0, sizeof(rpMsg));
						rpMsg.msgType = RESPOND;
						rpMsg.msgID = FILEDELETECOMMAND;
						rpMsg.nodeInfo = myNode.nodeInfo;
						rpMsg.moreInfo = SUCCESS;
						rpMsg.bodySize = 0;


						if (i != myNode.chordInfo.FRefInfo.fileNum - 1)
						{
							for (int j = i + 1; j < (int)myNode.chordInfo.FRefInfo.fileNum; j++)
							{
								myNode.chordInfo.FRefInfo.fileRef[j - 1] = myNode.chordInfo.FRefInfo.fileRef[j];
							}
						}
						myNode.chordInfo.FRefInfo.fileNum--;

						retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&msgInfo.senderSockAddr, AddrLen);
						if (retval == SOCKET_ERROR) {
							if (WSAGetLastError() == WSAETIMEDOUT) {
								printf("\a[ERROR] procRecv) FILEDELETE REQUEST timed out!\n");
								return -1;
							}
							printf("\a[ERROR] procRecv) FILEDELETE Sendto Error!\n");
							return -1;
						}

						break;
					}

				}
				WaitForSingleObject(findPreThread, INFINITE);
				break;

			case MOVEKEYSCOMMAND:
					
				//if(!sMode)
					printf("procRecv) MOVEKEYS request ����.\n");

				clientID = msgInfo.msg.nodeInfo.ID;
				//keyscount
				for (int i = 0; i < (int)myNode.chordInfo.FRefInfo.fileNum; i++)
				{
					printf("procRecv) MyfileRef[%d].key : %d\n", i, myNode.chordInfo.FRefInfo.fileRef[i].Key);
					if (modIn(ringSize, myNode.chordInfo.FRefInfo.fileRef[i].Key, myNode.nodeInfo.ID, clientID, 1, 0))
					{
						printf("procRecv) MOVEKEYS ��ȣ : %d\n", myNode.chordInfo.FRefInfo.fileRef[i].Key);
						fileRefList[keysCount] = myNode.chordInfo.FRefInfo.fileRef[i];
						keysCount++;

						for (int j = i; j < (int)myNode.chordInfo.FRefInfo.fileNum - 1; j++)
						{
							myNode.chordInfo.FRefInfo.fileRef[j] = myNode.chordInfo.FRefInfo.fileRef[j + 1];
						}
						i--;
						myNode.chordInfo.FRefInfo.fileNum--;

					}
				}

				memset(&rpMsg, 0, sizeof(rpMsg));
				rpMsg.msgType = RESPOND;
				rpMsg.msgID = MOVEKEYSCOMMAND;
				rpMsg.nodeInfo = myNode.nodeInfo;
				rpMsg.moreInfo = keysCount;
				rpMsg.bodySize = 0;

				//if (!sMode)
					printf("procRecv) MOVEKEYS ����.\n");
			
				/*���� Node���� MOVEKEYS ����*/
				retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&msgInfo.senderSockAddr, AddrLen);
				
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] procRecv) MOVEKEYS Respond timed out!\n");
						return -1;
					}
					printf("\a[ERROR] procRecv) MOVEKEYS sendto Error!\n");
					return -1;
				}


				for (int i = 0; i < keysCount; i++)
				{
					if (!sMode)
						printf("procRecv) MOVEKEYS Body ����.\n");
					/*���� Node���� MOVEKEYS Body ����*/
					retval = sendto(rpSock, (char *)&fileRefList[i], sizeof(fileRefType), 0, (struct sockaddr*)&msgInfo.senderSockAddr, AddrLen);
					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] procRecv) MOVEKEYS Body Respond timed out!\n");
							return -1;
						}
						printf("\a[ERROR] procRecv) MOVEKEYS Body sendto Error!\n");
						return -1;
					}


				}

				WaitForSingleObject(findPreThread, INFINITE);
				break;

			case LEAVEKEYSCOMMAND:
				if (!sMode)
					printf("procRecv) LEAVEKEYS request ����.\n");

				myNode.chordInfo.FRefInfo.fileNum++;

				myNode.chordInfo.FRefInfo.fileRef[myNode.fileInfo.fileNum - 1].Key = msgInfo.msg.fileInfo.Key;
				strcpy(myNode.chordInfo.FRefInfo.fileRef[myNode.fileInfo.fileNum - 1].Name, msgInfo.msg.fileInfo.Name);
				myNode.chordInfo.FRefInfo.fileRef[myNode.fileInfo.fileNum - 1].owner = msgInfo.msg.fileInfo.owner;
				myNode.chordInfo.FRefInfo.fileRef[myNode.fileInfo.fileNum - 1].refOwner = myNode.nodeInfo;
				//���ؽ� ����� ���ϰ� �����ϸ鼭 �ʿ��� �κп� ������ ��

				memset(&rpMsg, 0, sizeof(rpMsg));
				rpMsg.msgType = RESPOND;
				rpMsg.msgID = LEAVEKEYSCOMMAND;
				rpMsg.nodeInfo = myNode.nodeInfo;
				rpMsg.moreInfo = 0;
				rpMsg.bodySize = 0;
				
				/*LEAVEKYES*/
				retval = sendto(rpSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&msgInfo.senderSockAddr, AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] procRecv) LEAVEKEYS REQUEST timed out!\n");
						return -1;
					}
					printf("\a[ERROR] procRecv) LEAVEKEYS Sendto Error!\n");
					return -1;
				}

				break;
			}


		}

	}
	WaitForSingleObject(findPreThread, INFINITE);
	return 1;
}