#include "header.c"

unsigned WINAPI procPingPong(void *arg)
{
	srand((unsigned int)(time(NULL)));
	Sleep((rand() % 2001) + 5000);

	if (!sMode)
		printf("procPingPong ����!!!\n");

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
			/*��û �޽��� ����*/
			memset(&rqMsg, 0, sizeof(rqMsg));

			rqMsg.msgType = REQUEST;
			rqMsg.msgID = PINGPONGCOMMAND;
			rqMsg.nodeInfo = myNode.nodeInfo;
			rqMsg.moreInfo = 0;
			rqMsg.bodySize = 0;

			if (!sMode)
				printf("procPP)i : %d, PINGPONG �޽��� ��û.\n", i);
			//p_try = 0;
			/*fingertable�� Node�鿡�� PINGPONG ��û*/
			retval = sendto(ppSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&myNode.chordInfo.fingerInfo.finger[i].addrInfo, AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] procPP) PINGPONG REQUEST timed out!\n");
					/*����(SOCKET_ERROR || WSAETIMEOUT �� p_try++*/
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
				printf("procPP)i : %d, PINGPONG�� ���������� : addr: %s, port : %d, id : %d\n",
					i, inet_ntoa(rpMsg.nodeInfo.addrInfo.sin_addr), ntohs(rpMsg.nodeInfo.addrInfo.sin_port), rpMsg.nodeInfo.ID);

			i++;
		}


		left_node_id = myNode.chordInfo.fingerInfo.finger[(i)].ID;

		if (p_try == 2) /*p_try�� 2�̵� ��� �����ٰ� ����*/
		{	
			
			if (myNode.chordInfo.fingerInfo.finger[baseM - 1].ID == left_node_id) /*FingerTable ������ ��Ұ� ������ ���*/
			{
				myNode.chordInfo.fingerInfo.finger[baseM - 1] = myNode.nodeInfo;
			}

			for (int k = baseM - 2; k > -1; k--) /*FingerTable ������ ��һ���*/
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

				if (try == 0)	//ó�� 
				{
					if (!sMode)
						printf("procPP)try == 0] i : %d, PREINFO �޽��� ��û.\n", i);
					/*FingerTable ������ Node���� PREINFO ��û*/
					retval = sendto(ppSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&myNode.chordInfo.fingerInfo.finger[j].addrInfo, AddrLen);

					if (!sMode)
						printf("procPP)try == 0] i : %d, PREINFO �޽��� waiting..\n", i);
					retval = recvfrom(ppSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);

					if (!sMode)
						printf("procPP)try == 0] i : %d, PREINFO �޽��� ����.\n", i);

					temp_addr = rpMsg.nodeInfo.addrInfo; /*Pre ����*/
					temp_addr_pre = myNode.chordInfo.fingerInfo.finger[j].addrInfo; // temp_addr�� Pre

					try++;

					if (!sMode)
						printf("procPP) try == 0     00\n");
				}

				if (try != 0)
				{
					if (!sMode)
						printf("procPP)try != 0] i : %d, PREINFO �޽��� ��û.\n", i);
					/*Pre����  PREINFO ��û*/
					retval = sendto(ppSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&temp_addr, AddrLen);

					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] procPP) PREINFO REQUEST timed out!\n");
							/*����(SOCKET_ERROR || WSAETIMEOUT �� p_try++*/
							p_try++;
							continue;
						}
						printf("\a[ERROR] procPP) PREINFO Sendto Error!\n");
					}

					if (!sMode)
						printf("procPP)try != 0] i : %d, PREINFO �޽��� waiting...\n", i);

					retval = recvfrom(ppSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] procPP) PREINFO Receive timed out!\n");
							/*����(SOCKET_ERROR || WSAETIMEOUT �� p_try++*/
							p_try++;
							continue;
						}
						printf("\a[ERROR] procPP) PREINFO Revefrom Error!\n");
					}

					if (!sMode)
						printf("procPP)try != 0] i : %d, PREINFO �޽��� �������.\n", i);
				
					temp_addr_pre = temp_addr;
					temp_addr = rpMsg.nodeInfo.addrInfo;

					if (!sMode)
						printf("procPP)try != 0] try != 0     01\n");
				}


				if (!sMode)
					printf("procPP) rpMsg.node.id : %d rpMsg.node.addr : %s \n", rpMsg.nodeInfo.ID, inet_ntoa(temp_addr.sin_addr));
				
				if (rpMsg.nodeInfo.ID == myNode.nodeInfo.ID)	//Initial Node �� ��
				{
					myNode.chordInfo.fingerInfo.finger[(i)].ID = myNode.nodeInfo.ID;
					myNode.chordInfo.fingerInfo.finger[(i)].addrInfo = myNode.nodeInfo.addrInfo;
					return 0;
				}

				if (rpMsg.nodeInfo.ID == left_node_id)	// Pre�� ���� Node�̸�
				{

					memset(&rqMsg, 0, sizeof(chordHeaderType));

					rqMsg.msgType = REQUEST;
					rqMsg.msgID = PREUPDATECOMMAND;
					rqMsg.nodeInfo = myNode.nodeInfo;
					rqMsg.moreInfo = 0;
					rqMsg.bodySize = 0;

					if (!sMode)
						printf("procPP)rpMsg.ID == left.id] i : %d, PREUPDATE �޽��� ��û.\n", i);
				
					/*Pre�� Leave Node�� Node���� PREUPDATE ��û*/
					retval = sendto(ppSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&temp_addr_pre, AddrLen);
					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] procPP) PREUPDATE REQUEST timed out!\n");
							/*����(SOCKET_ERROR || WSAETIMEOUT �� p_try++*/
							p_try++;
							continue;
						}
						printf("\a[ERROR] procPP) PREUPDATE Sendto Error!\n");
					}

					if (!sMode)
						printf("procPP)rpMsg.ID == left.id] i : %d, PREUPDATE �޽��� waiting...\n", i);

					retval = recvfrom(ppSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
					if (retval == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) {
							printf("\a[ERROR] procPP) PREUPDATE Receive timed out!\n");
							/*����(SOCKET_ERROR || WSAETIMEOUT �� p_try++*/
							p_try++;
							continue;
						}
						printf("\a[ERROR] procPP) PREUPDATE Recvfrom Error!\n");
					}

					if (!sMode)
						printf("procPP)rpMsg.ID == left.id] i : %d, PREUPDATE �޽��� ����.\n", i);

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
			

			/*���� stablize*/
			for (int i = 0; i < (int)myNode.chordInfo.FRefInfo.fileNum; i++)
			{
				if (myNode.chordInfo.FRefInfo.fileRef[i].owner.ID == left_node_id)	//RefInfo�� ���� Node�̸�
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
				if (myNode.fileInfo.fileRef[i].refOwner.ID == left_node_id)	/*������ file�� Ref�� ���� Node�̸�*/
				{
					int fingerId = myNode.fileInfo.fileRef[i].Key;

					memset(&rqMsg, 0, sizeof(chordHeaderType));
					rqMsg.msgType = REQUEST;
					rqMsg.msgID = FINDPRECOMMAND;
					rqMsg.nodeInfo = myNode.nodeInfo;
					rqMsg.moreInfo = fingerId;
					rqMsg.bodySize = 0;

					if (!sMode)
						printf("PINGPONG FILE) SUCC���� findpred ��û��.\n");
					/*Succ���� fileKey�� FINDPRE ��û*/
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
						printf("PINGPONG FILE) SUCC���� findpred �������.\n");

					//���� ��尡 �ڽ��� ��û�ѳ���ϰ�� �׸��� �ڽ��� ���ϴ� �������� Ÿ���ϰ�� 
					/*Pre���� SUCCINFO��û*/
					memset(&rqMsg, 0, sizeof(chordHeaderType));
					rqMsg.msgType = REQUEST;
					rqMsg.msgID = SUCCINFOCOMMAND;
					rqMsg.nodeInfo = myNode.nodeInfo;
					rqMsg.moreInfo = 0;
					rqMsg.bodySize = 0;

					if (!sMode)
						printf("FILE ADD) succinfo ��û��.\n");

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
						printf("PINGPONG FILE) succinfo �������.\n");

					if (rpMsg.nodeInfo.ID == myNode.nodeInfo.ID)	//fileSucc�� myNode�� ��
					{
						myNode.fileInfo.fileRef[i].refOwner = myNode.nodeInfo;
						//������ �κ�
						myNode.chordInfo.FRefInfo.fileRef[i].Key = myNode.fileInfo.fileRef[i].Key;
						myNode.chordInfo.FRefInfo.fileRef[i].refOwner = myNode.nodeInfo;
						myNode.chordInfo.FRefInfo.fileRef[i].owner = myNode.nodeInfo;
						strcpy(myNode.chordInfo.FRefInfo.fileRef[i].Name, myNode.fileInfo.fileRef[i].Name);

						//printf("PINGPONG FILE) PINGPONG FILE ������ : %d\n", rpMsg.nodeInfo.ID);
						//	printf("PINGPONG FILE) ���� add �Ϸ��!\n �����̸�: %s ����Ű: %d\n", myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Name, myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Key);

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
							printf("PINGPONG FILE) fileadd ��û!!!\n");
						/*file Succ���� FILEADD ��û*/
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
							printf("PINGPONG FILE) PINGPONG FILE �������!!!\n");

						printf("PINGPONG FILE) PINGPONG FILE ������ : %d\n", rpMsg.nodeInfo.ID);
						printf("PINGPONG FILE) ���� add �Ϸ��!\n �����̸�: %s ����Ű: %d\n", myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Name, myNode.fileInfo.fileRef[myNode.fileInfo.fileNum - 1].Key);
					}

				}
			}
			
		}

		Sleep((rand() % 2001) + 3000);

	}
	if (!sMode)
		printf("procPingPong Thread ���� !!!\n");
	return 0;
}