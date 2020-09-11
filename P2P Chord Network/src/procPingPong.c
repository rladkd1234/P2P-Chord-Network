#include "header.c"

unsigned WINAPI procPingPong(void *arg)
{
	srand((unsigned int)(time(NULL)));
	Sleep((rand() % 2001) + 5000);

	if (!sMode)
		printf("procPingPong 시작!!!\n");

	int *exitFlag = (int*)arg;
	int AddrLen = sizeof(struct sockaddr_in);
	int retval;
	struct sockaddr_in receiveAddr;
	int p_try = 0;
	chordHeaderType rqMsg, rpMsg;

	//	msgInfoType msgInfo;
	memset((char*)&rqMsg, 0, sizeof(chordHeaderType));
	memset((char*)&rpMsg, 0, sizeof(chordHeaderType));

	int optVal = 10000;
	setsockopt(ppSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&optVal, sizeof(optVal));
	int left_node_id;
	while (!(*exitFlag))
	{
		int i = 1;
		p_try = 0;
		while (i < baseM)
		{
			//printf("%d %d\n", p_try, i);
			if (p_try == 2)
			{
				break;
			}
			/*요청 메시지 생성*/
			memset(&rqMsg, 0, sizeof(rqMsg));

			rqMsg.msgType = REQUEST;
			rqMsg.msgID = PINGPONGCOMMAND;
			rqMsg.nodeInfo = myNode.nodeInfo;
			rqMsg.moreInfo = 0;
			rqMsg.bodySize = 0;

			if (!sMode)
				printf("procPP)i : %d, PINGPONG 메시지 요청.\n", i);
			//p_try = 0;
			/*fingertable의 Node들에게 PINGPONG 요청*/
			retval = sendto(ppSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&myNode.chordInfo.fingerInfo.finger[i].addrInfo, AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] procPP) PINGPONG REQUEST timed out!\n");
					/*에러(SOCKET_ERROR || WSAETIMEOUT 시 p_try++*/
					p_try++;
					continue;
				}
				printf("\a[ERROR] procPP) PINGPONG Sendto Error!\n");

				p_try++;
				continue;
			}

			if (!sMode)
				printf("procPP)i : %d, PINGPONG waiting...\n", i);

			retval = recvfrom(ppSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] procPP) PINGPONG RECEIVE timed out!\n");
					p_try++;

					continue;
				}
				printf("\a[ERROR] procPP) PINGPONG Recvfrom Error!\n");
				p_try++;

				continue;
			}

			if (retval != SOCKET_ERROR) {
				p_try = 0;
				break;
			}

			if (!sMode)
				printf("procPP)i : %d, PINGPONG시 응답노드정보 : addr: %s, port : %d, id : %d\n",
					i, inet_ntoa(rpMsg.nodeInfo.addrInfo.sin_addr), ntohs(rpMsg.nodeInfo.addrInfo.sin_port), rpMsg.nodeInfo.ID);

			i++;
		}


		left_node_id = myNode.chordInfo.fingerInfo.finger[(i)].ID;

		if (p_try == 2) /*p_try가 2이된 경우 나갔다고 간주*/
		{	
			
			if (myNode.chordInfo.fingerInfo.finger[baseM - 1].ID == left_node_id) /*FingerTable 마지막 요소가 나갔을 경우*/
			{
				myNode.chordInfo.fingerInfo.finger[baseM - 1] = myNode.nodeInfo;
			}

			for (int k = baseM - 2; k > -1; k--) /*FingerTable 마지막 요소빼고*/
			{
				if (myNode.chordInfo.fingerInfo.finger[k].ID == left_node_id)
				{
					myNode.chordInfo.fingerInfo.finger[k] = myNode.chordInfo.fingerInfo.finger[k + 1];
				}
			}

			p_try = 0;
			//sharing.msg.moreInfo = i;
			int j = baseM - 1;
			int try = 0;
			struct sockaddr_in temp_addr;
			struct sockaddr_in temp_addr_pre;

			//if (!sMode)
			printf("proPP) left node index : %d\n", (i));

			for (;;)
			{
				//if (!sMode)
				printf("proPP) left node id : %d\n", left_node_id);
			
				memset(&rqMsg, 0, sizeof(chordHeaderType));

				rqMsg.msgType = REQUEST;
				rqMsg.msgID = PREINFOCOMMAND;
				rqMsg.nodeInfo = myNode.nodeInfo;
				rqMsg.moreInfo = left_node_id;
				rqMsg.bodySize = 0;

				if (try == 0)	//처음 
				{
					if (!sMode)
						printf("procPP)try == 0] i : %d, PREINFO 메시지 요청.\n", i);
					/*FingerTable 마지막 Node에게 PREINFO 요청*/
					retval = sendto(ppSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&myNode.chordInfo.fingerInfo.finger[j].addrInfo, AddrLen);

					if (!sMode)
						printf("procPP)try == 0] i : %d, PREINFO 메시지 waiting..\n", i);
					retval = recvfrom(ppSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);

					if (!sMode)
						printf("procPP)try == 0] i : %d, PREINFO 메시지 응답.\n", i);

					temp_addr = rpMsg.nodeInfo.addrInfo; /*Pre 정보*/
					temp_addr_pre = myNode.chordInfo.fingerInfo.finger[j].addrInfo; // temp_addr의 Pre

					try++;

					if (!sMode)
						printf("procPP) try == 0     00\n");
				}

				if (try != 0)
				{
					if (!sMode)
						printf("procPP)try != 0] i : %d, PREINFO 메시지 요청.\n", i);
					/*Pre에게  PREINFO 요청*/
					retval = sendto(ppSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&temp_addr, AddrLen);

					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] procPP) PREINFO REQUEST timed out!\n");
							/*에러(SOCKET_ERROR || WSAETIMEOUT 시 p_try++*/
							p_try++;
							continue;
						}
						printf("\a[ERROR] procPP) PREINFO Sendto Error!\n");
					}

					if (!sMode)
						printf("procPP)try != 0] i : %d, PREINFO 메시지 waiting...\n", i);

					retval = recvfrom(ppSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] procPP) PREINFO Receive timed out!\n");
							/*에러(SOCKET_ERROR || WSAETIMEOUT 시 p_try++*/
							p_try++;
							continue;
						}
						printf("\a[ERROR] procPP) PREINFO Revefrom Error!\n");
					}

					if (!sMode)
						printf("procPP)try != 0] i : %d, PREINFO 메시지 응답받음.\n", i);
				
					temp_addr_pre = temp_addr;
					temp_addr = rpMsg.nodeInfo.addrInfo;

					if (!sMode)
						printf("procPP)try != 0] try != 0     01\n");
				}


				if (!sMode)
					printf("procPP) rpMsg.node.id : %d rpMsg.node.addr : %s \n", rpMsg.nodeInfo.ID, inet_ntoa(temp_addr.sin_addr));
				
				if (rpMsg.nodeInfo.ID == myNode.nodeInfo.ID)	//Initial Node 일 때
				{
					myNode.chordInfo.fingerInfo.finger[(i)].ID = myNode.nodeInfo.ID;
					myNode.chordInfo.fingerInfo.finger[(i)].addrInfo = myNode.nodeInfo.addrInfo;
					return 0;
				}

				if (rpMsg.nodeInfo.ID == left_node_id)	// Pre가 나간 Node이면
				{

					memset(&rqMsg, 0, sizeof(chordHeaderType));

					rqMsg.msgType = REQUEST;
					rqMsg.msgID = PREUPDATECOMMAND;
					rqMsg.nodeInfo = myNode.nodeInfo;
					rqMsg.moreInfo = 0;
					rqMsg.bodySize = 0;

					if (!sMode)
						printf("procPP)rpMsg.ID == left.id] i : %d, PREUPDATE 메시지 요청.\n", i);
				
					/*Pre가 Leave Node인 Node에게 PREUPDATE 요청*/
					retval = sendto(ppSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&temp_addr_pre, AddrLen);
					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] procPP) PREUPDATE REQUEST timed out!\n");
							/*에러(SOCKET_ERROR || WSAETIMEOUT 시 p_try++*/
							p_try++;
							continue;
						}
						printf("\a[ERROR] procPP) PREUPDATE Sendto Error!\n");
					}

					if (!sMode)
						printf("procPP)rpMsg.ID == left.id] i : %d, PREUPDATE 메시지 waiting...\n", i);

					retval = recvfrom(ppSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] procPP) PREUPDATE Receive timed out!\n");
							/*에러(SOCKET_ERROR || WSAETIMEOUT 시 p_try++*/
							p_try++;
							continue;
						}
						printf("\a[ERROR] procPP) PREUPDATE Recvfrom Error!\n");
					}

					if (!sMode)
						printf("procPP)rpMsg.ID == left.id] i : %d, PREUPDATE 메시지 응답.\n", i);

					if (rpMsg.moreInfo == SUCCESS)
					{
						myNode.chordInfo.fingerInfo.finger[(i)] = rpMsg.nodeInfo;
						/*myNode.chordInfo.fingerInfo.finger[(left_node)].addrInfo = rpMsg.nodeInfo.addrInfo;
						myNode.chordInfo.fingerInfo.finger[(left_node)].ID = rpMsg.nodeInfo.ID;*/
						myNode.chordInfo.fingerInfo.finger[0] = rpMsg.nodeInfo;
						//myNode.chordInfo.fingerInfo.Pre = rpMsg.nodeInfo;
					//	if (!sMode)
						printf("procPP) Stablize leave complete!\n");
						break;
					}
					else
					{
						perror("ERROR] procPP) Stablize leave\n");
						exit(1);
					}
				}
			}
			

			/*파일 stablize*/
			for (int i = 0; i < (int)myNode.chordInfo.FRefInfo.fileNum; i++)
			{
				if (myNode.chordInfo.FRefInfo.fileRef[i].owner.ID == left_node_id)	//RefInfo가 나간 Node이면
				{
					//delete
					if (i != myNode.chordInfo.FRefInfo.fileNum)
					{
						for (int j = i + 1; j < (int)myNode.chordInfo.FRefInfo.fileNum; j++)
						{
							myNode.chordInfo.FRefInfo.fileRef[j - 1] = myNode.chordInfo.FRefInfo.fileRef[j];
						}
					}
					myNode.chordInfo.FRefInfo.fileNum--;
				}
			}

			for (int i = 0; i < (int)myNode.fileInfo.fileNum; i++)
			{
				//new add
				if (myNode.fileInfo.fileRef[i].refOwner.ID == left_node_id)	/*소유한 file의 Ref가 나간 Node이면*/
				{
					int fingerId = myNode.fileInfo.fileRef[i].Key;

					memset(&rqMsg, 0, sizeof(chordHeaderType));
					rqMsg.msgType = REQUEST;
					rqMsg.msgID = FINDPRECOMMAND;
					rqMsg.nodeInfo = myNode.nodeInfo;
					rqMsg.moreInfo = fingerId;
					rqMsg.bodySize = 0;

					if (!sMode)
						printf("PINGPONG FILE) SUCC에게 findpred 요청함.\n");
					/*Succ에게 fileKey의 FINDPRE 요청*/
					retval = sendto(ppSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&myNode.chordInfo.fingerInfo.finger[0].addrInfo, AddrLen);
					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] PINGPONG FILE) findpred REQUEST timed out!\n");

							continue;
						}
						printf("\a[ERROR]  PINGPONG FILE) findpred Sendto Error!\n");
						continue;
					}


					if (!sMode)
						printf("PINGPONG FILE) findpred waiting...\n");

					retval = recvfrom(ppSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] PINGPONG FILE) findpred Receive timed out!\n");

							continue;
						}
						printf("\a[ERROR] PINGPONG FILE) findpred Recvfrom Error!\n");
						continue;
					}

					if (!sMode)
						printf("PINGPONG FILE) SUCC에게 findpred 응답받음.\n");

					//보낸 노드가 자신이 요청한노드일경우 그리고 자신이 원하는 리스폰스 타입일경우 
					/*Pre에게 SUCCINFO요청*/
					memset(&rqMsg, 0, sizeof(chordHeaderType));
					rqMsg.msgType = REQUEST;
					rqMsg.msgID = SUCCINFOCOMMAND;
					rqMsg.nodeInfo = myNode.nodeInfo;
					rqMsg.moreInfo = 0;
					rqMsg.bodySize = 0;

					if (!sMode)
						printf("FILE ADD) succinfo 요청함.\n");

					retval = sendto(ppSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&rpMsg.nodeInfo.addrInfo, AddrLen);
					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] PINGPONG FILE) succinfo REQUEST timed out!\n");

							continue;
						}
						printf("\a[ERROR] PINGPONG FILE) succinfo Sendto Error!\n");
						continue;
					}

					if (!sMode)
						printf("PINGPONG FILE) succinfo waiting...\n");

					retval = recvfrom(ppSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] PINGPONG FILE) succinfo Receive timed out!\n");

							continue;
						}
						printf("\a[ERROR] PINGPONG FILE) succinfo recvfrom Error!\n");
						continue;
					}

					if (!sMode)
						printf("PINGPONG FILE) succinfo 응답받음.\n");

					if (rpMsg.nodeInfo.ID == myNode.nodeInfo.ID)	//fileSucc가 myNode일 때
					{
						myNode.fileInfo.fileRef[i].refOwner = myNode.nodeInfo;
						//수정된 부분
						myNode.chordInfo.FRefInfo.fileRef[i].Key = myNode.fileInfo.fileRef[i].Key;
						myNode.chordInfo.FRefInfo.fileRef[i].refOwner = myNode.nodeInfo;
						myNode.chordInfo.FRefInfo.fileRef[i].owner = myNode.nodeInfo;
						strcpy(myNode.chordInfo.FRefInfo.fileRef[i].Name, myNode.fileInfo.fileRef[i].Name);

						//printf("PINGPONG FILE) PINGPONG FILE 응답노드 : %d\n", rpMsg.nodeInfo.ID);
						//	printf("PINGPONG FILE) 파일 add 완료됨!\n 파일이름: %s 파일키: %d\n", myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Name, myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Key);

					}
					else
					{
						myNode.fileInfo.fileRef[i].refOwner = rpMsg.nodeInfo;

						memset(&rqMsg, 0, sizeof(chordHeaderType));
						rqMsg.msgType = REQUEST;
						rqMsg.msgID = FILEADDCOMMAND;
						rqMsg.nodeInfo = myNode.nodeInfo;
						rqMsg.moreInfo = 0;
						rqMsg.bodySize = 0;
						rqMsg.fileInfo = myNode.fileInfo.fileRef[i];

						if (!sMode)
							printf("PINGPONG FILE) fileadd 요청!!!\n");
						/*file Succ에게 FILEADD 요청*/
						retval = sendto(ppSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&rpMsg.nodeInfo.addrInfo, AddrLen);
						if (retval == SOCKET_ERROR) {
							if (WSAGetLastError() == WSAETIMEDOUT) {
								printf("\a[ERROR] PINGPONG FILE) fileadd REQUEST timed out!\n");

								continue;
							}
							printf("\a[ERROR] PINGPONG FILE) fileadd sendto Error!\n");
							continue;
						}
						if (!sMode)
							printf("PINGPONG FILE) fileadd waiting...\n");

						retval = recvfrom(ppSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
						if (retval == SOCKET_ERROR) {
							if (WSAGetLastError() == WSAETIMEDOUT) {
								printf("\a[ERROR] PINGPONG FILE) fileadd RECEIVE timed out!\n");

								continue;
							}
							printf("\a[ERROR] PINGPONG FILE) fileadd recvfrom Error!\n");
							continue;
						}

						if (!sMode)
							printf("PINGPONG FILE) PINGPONG FILE 응답받음!!!\n");

						printf("PINGPONG FILE) PINGPONG FILE 응답노드 : %d\n", rpMsg.nodeInfo.ID);
						printf("PINGPONG FILE) 파일 add 완료됨!\n 파일이름: %s 파일키: %d\n", myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Name, myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Key);
					}

				}
			}
			
		}

		Sleep((rand() % 2001) + 3000);

	}
	if (!sMode)
		printf("procPingPong Thread 종료 !!!\n");
	return 0;
}