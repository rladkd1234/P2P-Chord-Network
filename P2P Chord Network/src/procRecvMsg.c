#include "header.c"

unsigned WINAPI procRecvMsg(void *arg)
{

	if (!sMode)
		printf("procRecvMsg 시작!!!\n");
	
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
		/*요청 메시지 수신 대기*/
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
				/*요청 메시지 타입 확인*/
		if (Msg.msgType == REQUEST) { // 요청 메시지이면
		
			if (!sMode)	
				printf("요청 메시지 요청받음.\n");

			switch (msgInfo.msg.msgID)
			{

			case JOINCOMMAND: // 요청 메시지가 JOIN이면

				//if (!sMode)
					printf("procRecv) JOIN request 들어옴.\n");
			
				joinThread = (HANDLE)_beginthreadex(NULL, 0, (void*)procJoin, (void *)&msgInfo, 0, NULL);
				WaitForSingleObject(joinThread, INFINITE);

				break;

			case FINDPRECOMMAND:
				
				if (!sMode)
					printf("procRecv) FINDPRED request 들어옴.\n");
				
				findPreThread = (HANDLE)_beginthreadex(NULL, 0, (void*)procFindPred, (void *)&msgInfo, 0, NULL);
				WaitForSingleObject(findPreThread, INFINITE);

				break;

			case SUCCINFOCOMMAND:
				
				if (!sMode)
					printf("procRecv) SUCCINFO request 들어옴.\n");
				
				memset(&rpMsg, 0, sizeof(rpMsg));
				/*수신 메시지 인코딩*/
				rpMsg.msgType = RESPOND; // respond
				rpMsg.msgID = SUCCINFOCOMMAND;   // JoinInfo
				rpMsg.nodeInfo = myNode.chordInfo.fingerInfo.finger[0];
				rpMsg.moreInfo = SUCCESS;
				rpMsg.bodySize = 0;
				if (!sMode)		printf("procRecv) SUCCINFO 메시지 응답.\n");

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
					printf("procRecv) PREINFO request 들어옴.\n");
			
				memset(&rpMsg, 0, sizeof(rpMsg));
				/*수신 메시지 인코딩*/
				rpMsg.msgType = RESPOND; // respond
				rpMsg.msgID = PREINFOCOMMAND;   // JoinInfo
				rpMsg.nodeInfo = myNode.chordInfo.fingerInfo.Pre;
				rpMsg.moreInfo = SUCCESS;
				rpMsg.bodySize = 0;

				if (!sMode)	
					printf("procRecv) PREINFO 메시지 응답.\n");

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
					printf("procRecv) PREUPDATE request 들어옴.\n");
				
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
					printf("procRecv) PREUPDATE 메시지 응답.\n");
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
					printf("procRecv) SUCCUPDATE request 들어옴.\n");
			
				memset(&rpMsg, 0, sizeof(rpMsg));

				WaitForSingleObject(hMutex, INFINITE);
				myNode.chordInfo.fingerInfo.finger[0] = msgInfo.msg.nodeInfo;
				ReleaseMutex(hMutex);

				rpMsg.msgType = RESPOND;
				rpMsg.msgID = SUCCUPDATECOMMAND;   // JoinInfo
				rpMsg.nodeInfo = myNode.nodeInfo;
				rpMsg.moreInfo = SUCCESS;
				rpMsg.bodySize = 0;
				if (!sMode)	printf("procRecv) SUCCUPDATE 메시지 응답.\n");
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
					printf("procRecv) PINGPONG request 들어옴.\n");
			
				memset(&rpMsg, 0, sizeof(rpMsg));

				rpMsg.msgType = RESPOND;
				rpMsg.msgID = SUCCUPDATECOMMAND;
				rpMsg.nodeInfo = myNode.nodeInfo;
				rpMsg.moreInfo = SUCCESS;
				rpMsg.bodySize = 0;
				if (!sMode)	printf("procRecv) PINGPONG 메시지 응답.\n");
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
					printf("procRecv) FILEREF request 들어옴.\n");
				
				//파일 참조자 입장
				//파일 다운 커맨드에서 받은 정보로 자신의 파일 참조 테이블에서 소유자를 보내줌
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
					perror("파일을 참조하고 있지 않다.\n");
					continue;
				}
				printf("파일 참조 테이블 인덱스:%d\n", temp_file_index);
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
				//파일 
				break;

			case FILEADDCOMMAND:
				
				//if (!sMode) 
					printf("procRecv) FILEADD request 들어옴.\n");

				myNode.chordInfo.FRefInfo.fileNum++;

				myNode.chordInfo.FRefInfo.fileRef[myNode.chordInfo.FRefInfo.fileNum - 1].Key = msgInfo.msg.fileInfo.Key;
				strcpy(myNode.chordInfo.FRefInfo.fileRef[myNode.chordInfo.FRefInfo.fileNum - 1].Name, msgInfo.msg.fileInfo.Name); // 안되면 strcpy
				myNode.chordInfo.FRefInfo.fileRef[myNode.chordInfo.FRefInfo.fileNum - 1].owner = msgInfo.msg.fileInfo.owner;
				myNode.chordInfo.FRefInfo.fileRef[myNode.chordInfo.FRefInfo.fileNum - 1].refOwner = myNode.nodeInfo;

				//이제 잘 추가됐다고 rpmsg 보내면됨
				memset(&rpMsg, 0, sizeof(rpMsg));
				rpMsg.msgType = RESPOND;
				rpMsg.msgID = FILEADDCOMMAND;
				rpMsg.nodeInfo = myNode.nodeInfo;
				rpMsg.moreInfo = 0;
				rpMsg.bodySize = 0;

				/*요청 Node에게 FILLADD 응답*/
				retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&msgInfo.senderSockAddr, AddrLen);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) {
						printf("\a[ERROR] procRecv) FILEADD REQUEST timed out!\n");
						return -1;
					}
					printf("\a[ERROR] procRecv) FILEADD Sendto Error!\n");
					return -1;
				}

				//파일 참조자 측에서도 파일 소유자 정보를 갖고 있어야함
				//파일 소유자 : owner : 자신 refowner : 참조하고 있는놈
				//파일 참조자 : owner 자신에게 파일 애드 커맨드를 보낸놈 refowner : 자신
				WaitForSingleObject(findPreThread, INFINITE);
				break;

			case FILEDOWNCOMMAND:
				/* 1125 ??? 뭔가 이상한데? */
			
				//if (!sMode) 
					printf("procRecv) FILEDOWN request 들어옴.\n");

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
				/*procFileListen에 파일 이름 넘겨줌*/
				/*응답Node에게 FILEDOWN 응답*/
				retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&msgInfo.senderSockAddr, AddrLen);

				sharing.msg.moreInfo = temp_file_index;
			

				//이제 여기서 크기를 보내야함?
				//파일 다운 커맨드에서 파일 크기가 가야한다.
				WaitForSingleObject(findPreThread, INFINITE);
				break;

			case FILEDELETECOMMAND:
			
				//if (!sMode)
					printf("procRecv) FILEDELETE request 들어옴.\n");

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
					printf("procRecv) MOVEKEYS request 들어옴.\n");

				clientID = msgInfo.msg.nodeInfo.ID;
				//keyscount
				for (int i = 0; i < (int)myNode.chordInfo.FRefInfo.fileNum; i++)
				{
					printf("procRecv) MyfileRef[%d].key : %d\n", i, myNode.chordInfo.FRefInfo.fileRef[i].Key);
					if (modIn(ringSize, myNode.chordInfo.FRefInfo.fileRef[i].Key, myNode.nodeInfo.ID, clientID, 1, 0))
					{
						printf("procRecv) MOVEKEYS 번호 : %d\n", myNode.chordInfo.FRefInfo.fileRef[i].Key);
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
					printf("procRecv) MOVEKEYS 응답.\n");
			
				/*응답 Node에게 MOVEKEYS 응답*/
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
						printf("procRecv) MOVEKEYS Body 응답.\n");
					/*응답 Node에게 MOVEKEYS Body 응답*/
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
					printf("procRecv) LEAVEKEYS request 들어옴.\n");

				myNode.chordInfo.FRefInfo.fileNum++;

				myNode.chordInfo.FRefInfo.fileRef[myNode.fileInfo.fileNum - 1].Key = msgInfo.msg.fileInfo.Key;
				strcpy(myNode.chordInfo.FRefInfo.fileRef[myNode.fileInfo.fileNum - 1].Name, msgInfo.msg.fileInfo.Name);
				myNode.chordInfo.FRefInfo.fileRef[myNode.fileInfo.fileNum - 1].owner = msgInfo.msg.fileInfo.owner;
				myNode.chordInfo.FRefInfo.fileRef[myNode.fileInfo.fileNum - 1].refOwner = myNode.nodeInfo;
				//뮤텍스 사용은 다하고 검토하면서 필요한 부분에 넣으면 됨

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