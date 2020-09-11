#include "header.c"

unsigned WINAPI procFixFinger(void *arg)
{
	srand((unsigned int)(time(NULL)));
	Sleep((rand() % 2001) + 3000);
	if (!sMode)
		printf("procFF) procFixFinger 시작!!!\n");

	int *exitFlag = (int*)arg;
	int AddrLen = sizeof(struct sockaddr_in);
	int retval;
	nodeInfoType rpNode;
	struct sockaddr_in receiveAddr;
	int p_try = 0;
	chordHeaderType rqMsg, rpMsg;

	//	msgInfoType msgInfo;
	memset((char*)&rqMsg, 0, sizeof(chordHeaderType));
	memset((char*)&rpMsg, 0, sizeof(chordHeaderType));

	int optVal = 10000;
	setsockopt(ffSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&optVal, sizeof(optVal));

	while (!(*exitFlag))
	{
		for (int i = 1; i < baseM; i++)
		{
			int fingerId = modPlus(ringSize, myNode.nodeInfo.ID, twoPow(i));

			if (fingerId == myNode.nodeInfo.ID) {
				WaitForSingleObject(hMutex, INFINITE);
				myNode.chordInfo.fingerInfo.finger[i] = myNode.nodeInfo;
				ReleaseMutex(hMutex);
				continue;
			}

			/*FindPredcessor 요청(ㅡmoreInfo에 MyNodeID + 2^i*/
			/*요청 메시지 생성*/
			memset(&rqMsg, 0, sizeof(rqMsg));

			rqMsg.msgType = REQUEST;
			rqMsg.msgID = FINDPRECOMMAND;
			rqMsg.nodeInfo = myNode.nodeInfo;
			rqMsg.moreInfo = fingerId; // myNodeID + 2^i
			rqMsg.bodySize = 0;

			if (!sMode)
				printf("procFF) i : %d, %d(SUCC)에게 FINDPRE 요청 메시지 보냄.\n", i, myNode.chordInfo.fingerInfo.finger[0].ID);
			
			retval = sendto(ffSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&myNode.chordInfo.fingerInfo.finger[0].addrInfo, AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] procFF) FINDPRE REQUEST timed out!\n");

					continue;
				}
				printf("\a[ERROR] procFF) FINDPRE Sendto Error!\n");
				continue;
			}

			if (!sMode)
				printf("procFF) i : %d, FINDPRE waiting...\n", i);
		//	WaitForSingleObject(hMutex, INFINITE);
			retval = recvfrom(ffSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] procFF) FINDPRE Receive timed out!\n");

					continue;
				}
				printf("\a[ERROR] procFF) FINDPRE Recvfrom Error!\n");
				continue;
			}

			if (!sMode)	
				printf("procFF)i : %d, FINDPRE에서 PRE정보 : addr: %s, port : %d, id : %d \n",
				i, inet_ntoa(rpMsg.nodeInfo.addrInfo.sin_addr), ntohs(rpMsg.nodeInfo.addrInfo.sin_port), rpMsg.nodeInfo.ID);

			/*pre에게 SUCCINFO 요청*/
			rpNode = rpMsg.nodeInfo;
			/*SUCCSESSORINFO 전송*/
		//	memset(&rpMsg, 0, sizeof(chordHeaderType));
			memset(&rqMsg, 0, sizeof(chordHeaderType));
			rqMsg.msgType = REQUEST;
			rqMsg.msgID = SUCCINFOCOMMAND;
			rqMsg.nodeInfo = myNode.nodeInfo;
			rqMsg.moreInfo = 0;
			rqMsg.bodySize = 0;

			if (!sMode)
				printf("procFF)i : %d, SUCCINFO 요청 메시지 전송\n", i);
			
			retval = sendto(ffSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&rpNode.addrInfo, AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] procFF) SUCCINFO REQUEST timed out!\n");

					continue;
				}
				printf("\a[ERROR] procFF) SUCCINFO Sendto Error!\n");
				continue;
			}

			if (!sMode)
				printf("procFF)i : %d, SUCCINFO waiting...\n", i);
		
			retval = recvfrom(ffSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("\a[ERROR] procFF) SUCCINFO Receive timed out!\n");

					continue;
				}
				printf("\a[ERROR] procFF) SUCCINFO Recvfrom Error!\n");
				continue;
			}

			rpNode = rpMsg.nodeInfo;

			if (!sMode) 
				printf("procFF)i : %d, SUCCINFO에서 SUCC정보 : addr: %s, port : %d, id : %d\n",
					i, inet_ntoa(rpMsg.nodeInfo.addrInfo.sin_addr), ntohs(rpNode.addrInfo.sin_port), rpNode.ID);

			WaitForSingleObject(hMutex, INFINITE);
			myNode.chordInfo.fingerInfo.finger[i] = rpNode;
			ReleaseMutex(hMutex);

			if (!sMode)
				printf("procFF) proFixFinger : finger[%d] has been updated!!!\n", i);
		}

		Sleep((rand() % 2001) + 3000);

	}

	if (!sMode)
		printf("procFixFinger Thread 종료 !!!\n");
	return 0;
}