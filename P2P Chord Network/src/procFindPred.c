#include "header.c"

unsigned WINAPI procFindPred(void *arg)
{
	if (!sMode)
		printf("procFindPred 시작!!!\n");
	
	chordHeaderType rpMsg, rqMsg;
	msgInfoType *msgInfo = malloc(sizeof(msgInfoType*));
	msgInfo = (msgInfoType*)arg;
	nodeInfoType recvNode, rpNode;
	struct sockaddr_in receiveAddr;

	srand((unsigned int)time(NULL));

	int retval;
	int AddrLen = sizeof(struct sockaddr_in);
	int targetKey = msgInfo->msg.moreInfo;

	if (!sMode)	
		printf("procFindPred) msg.addr : %s, msg.port : %d, msg.ID : %d\n",
		inet_ntoa(msgInfo->senderSockAddr.sin_addr), ntohs(msgInfo->senderSockAddr.sin_port), msgInfo->msg.nodeInfo.ID);

	memset(&rpMsg, 0, sizeof(rpMsg));
	memset(&recvNode, -1, sizeof(recvNode));
	memset(&rpNode, 0, sizeof(rpNode));

	if (myNode.nodeInfo.ID == myNode.chordInfo.fingerInfo.finger[0].ID) // Initial Node 일 때
	{
		/*응답 메시지 인코딩*/
		rpMsg.msgType = RESPOND; // respond
		rpMsg.msgID = FINDPRECOMMAND;
		rpMsg.nodeInfo = myNode.nodeInfo;
		rpMsg.moreInfo = SUCCESS;
		rpMsg.bodySize = 0;

		if (!sMode)
			printf("procFindPred) [Initial Node]일 때 FINDPRE 메시지 응답.\n");

		/*요청 Node에게 FINDPRE 응답*/
		retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&(msgInfo->senderSockAddr), AddrLen);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("\a[ERROR] procFindPred) [Initial Node]일 때 FINDPRED REQUEST timed out!\n");
			}
			printf("\a[ERROR] procFindPred) [Initial Node]일 때 FINDPRED Sendto Error!\n");

		}

		if (!sMode)
			printf("procFindPred) [Initial Node]일 때 FINDPRED 응답 메시지 정보 rpMsg.nodeInfo.ID : %d\n", rpMsg.nodeInfo.ID);
	}

	else if (modIn(ringSize, targetKey, myNode.nodeInfo.ID, myNode.chordInfo.fingerInfo.finger[0].ID, 0, 1)) // Pre Node 일 때 
	{
		/*응답 메시지 인코딩*/
		rpMsg.msgType = RESPOND; // respond
		rpMsg.msgID = FINDPRECOMMAND;
		rpMsg.nodeInfo = myNode.nodeInfo;
		rpMsg.moreInfo = SUCCESS;
		rpMsg.bodySize = 0;

		if (!sMode)	
		printf("procFindPred) [Predessor Node]일 때 FINDPRE 메시지 응답.\n");

		/*요청 Node에게 FINDPRE 응답*/
		retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&(msgInfo->senderSockAddr), AddrLen);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("\a[ERROR] procFindPred) [Predessor Node]일 때 FINDPRED REQUEST timed out!\n");
				return -1;
			}
			printf("\a[ERROR] prcoFindPred) [Predessor Node]일 때 FINDPRED Sendto Error!\n");
			return -1;
		}

		if (!sMode)
			printf("procFindPred) [Predessor Node]일 때 FINDPRED 응답 메시지 정보 rpMsg.nodeInfo.ID : %d\n", rpMsg.nodeInfo.ID);
	}

	else
	{
		for (int i = baseM - 1; i >= 0; i--) {
			/*-1로 초기화 안할 시 Nodekey 0으로 인식해서 오류 */
			if (myNode.chordInfo.fingerInfo.finger[i].ID == -1) 
			{ 
				continue;
			}
			if (modIn(ringSize, myNode.chordInfo.fingerInfo.finger[i].ID, myNode.nodeInfo.ID, targetKey, 0, 0))
			{
				recvNode = myNode.chordInfo.fingerInfo.finger[i];
				break;
			}
		}
		/*요청 메시지 생성*/

		if (!sMode)
			printf("procFindPred) [else] recvNode : %d\n", recvNode.ID);
		/*아직 FixFinger 안됬을시 대비*/
		if (recvNode.ID == -1)
			recvNode = myNode.chordInfo.fingerInfo.finger[0];

		memset(&rqMsg, 0, sizeof(rqMsg));

		rqMsg.msgType = REQUEST;
		rqMsg.msgID = FINDPRECOMMAND;
		rqMsg.nodeInfo = myNode.nodeInfo;
		rqMsg.moreInfo = targetKey;
		rqMsg.bodySize = 0;

		if (!sMode)	
			printf("procFindPred) [else] FINDPRED일 때 recvNode정보 : addr: %s, port : %d, id : %d\n",
			inet_ntoa(recvNode.addrInfo.sin_addr), ntohs(recvNode.addrInfo.sin_port), recvNode.ID);

		if (!sMode)
			printf("procFindPred) [else] FINDPRED일 때 메시지 요청.\n");
	
		/*recvNode에게 FINDPRE 요청*/
		retval = sendto(rqSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&recvNode.addrInfo, AddrLen);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("\a[ERROR] procFindPred) [else] FINDPRED일 때 REQUEST timed out!\n");
				return -1;
			}
			printf("\a[ERROR] procFindPred) [else] FINDPRED일 때 Sendto Error!\n");
			return -1;
		}

		if (!sMode)
			printf("procFindPred) [else] FINDPRED일 때 waiting...\n");
	
		retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("\a[ERROR] procFindPred) [else] FINDPRED일 때 Receive timed out!\n");
				return -1;
			}
			printf("\a[ERROR] procFindPred) [else] FINDPRED일 때 Recvfrom Error!\n");
			return -1;
		}
		
		if (!sMode)
			printf("procFindPred) [else] FINDPRED일 때 FINDPRED 요청의 응답 메시지 정보 rpMsg.nodeInfo.ID : %d\n", rpMsg.nodeInfo.ID);

		rpNode = rpMsg.nodeInfo;

		//memset(&rpMsg, 0, sizeof(chordHeaderType));
		rpMsg.msgType = RESPOND; // respond
		rpMsg.msgID = FINDPRECOMMAND;
		rpMsg.nodeInfo = rpNode;
		rpMsg.moreInfo = SUCCESS;
		rpMsg.bodySize = 0;
	
		if (!sMode)
			printf("procFindPred) [else] FINDPRED일 때 메시지 응답.\n");

		/*요청 Node에게 FINDPRE 응답*/
		retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&(msgInfo->senderSockAddr), AddrLen);

		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("\a[ERROR] procFindPred) [else] FINDPRED일 때 REQUEST timed out!\n");
				return -1;
			}
			printf("\a[ERROR] procFindPred) [else] FINDPRED일 때 Sendto Error!\n");
			return -1;
		}

		if (!sMode)
			printf("procFindPred) [else] FINDPRED 응답 메시지 정보 rpMsg.nodeInfo.ID : %d\n", rpMsg.nodeInfo.ID);
	}

	if (!sMode)
		printf("procFindPred Thread 종료 !!!\n");
	
		return 0;
}
