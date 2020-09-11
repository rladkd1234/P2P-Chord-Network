#include "header.c"

unsigned WINAPI procJoin(void *arg)
{
	//	struct sockaddr_in *senderAddr = malloc(sizeof(struct sockaddr_in));
	//	senderAddr = (struct sockaddr_in*)arg;
	
	//if (!sMode)
		printf("procJoin 시작!!!\n");
	
	chordHeaderType rpMsg, rqMsg;

	//chordHeaderType *Msg = malloc(sizeof(chordHeaderType*));
	//Msg = (chordHeaderType*)arg;

	msgInfoType *msgInfo = malloc(sizeof(msgInfoType*));
	msgInfo = (msgInfoType*)arg;

	struct sockaddr_in recvAddr;
	nodeInfoType recvNode, rpNode;

	int retval;
	int AddrLen = sizeof(struct sockaddr_in);
	//printf("senderAddr : %s\nsenderAddr port : %d\n", inet_ntoa(senderAddr->sin_addr), ntohs(senderAddr->sin_port));

	//if (!sMode)	
		printf("procJoin) msg.addr : %s, msg.port : %d, JoinNode.ID: %d\n",
		inet_ntoa(msgInfo->senderSockAddr.sin_addr), ntohs(msgInfo->senderSockAddr.sin_port), msgInfo->msg.nodeInfo.ID);

	int targetKey = modPlus(ringSize, msgInfo->msg.nodeInfo.ID, 1);

	memset(&rqMsg, 0, sizeof(rqMsg));
	memset(&recvNode, 0, sizeof(recvNode));
	memset(&rpNode, 0, sizeof(rpNode));

	if (myNode.nodeInfo.ID == myNode.chordInfo.fingerInfo.finger[0].ID) // Initial Node 일 때
	{
		/*응답 메시지 인코딩*/
		rpMsg.msgType = RESPOND; // respond
		rpMsg.msgID = JOINCOMMAND;   // JoinInfo
		rpMsg.nodeInfo = myNode.nodeInfo;
		rpMsg.moreInfo = SUCCESS;
		rpMsg.bodySize = 0;
		
		/*요청 Node에게 JOINCOMMAND 응답*/
		
		//if (!sMode)	
			printf("procJoin) [Initial Node] JoinInfo 메시지 응답.\n");
		
		retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&(msgInfo->senderSockAddr), AddrLen);
		
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("\a[ERROR] procJoin) [Initial Node] JoinInfo REQUEST timed out!\n");
				return -1;
			}
			printf("\a[ERROR] procJoin) [Initial Node] JoinInfo Sendto Error!\n");
			return -1;
		}

	}

	else if (modIn(ringSize, targetKey, myNode.nodeInfo.ID, myNode.chordInfo.fingerInfo.finger[0].ID, 0, 1)) //내가 PRE Node 일 때
	{
		/*요청 Node에게 JOINCOMMAND 응답(SUCC정보)*/
		/*응답 메시지 인코딩*/
		rpMsg.msgType = RESPOND; // respond
		rpMsg.msgID = JOINCOMMAND;   // JoinInfo
		rpMsg.nodeInfo = myNode.chordInfo.fingerInfo.finger[0];
		rpMsg.moreInfo = SUCCESS;
		rpMsg.bodySize = 0;
		
		//if (!sMode)	
			printf("procJoin) [predessor Node]일 때 Joinfo 메시지 응답.\n");
		
		retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&(msgInfo->senderSockAddr), AddrLen);

		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("\a[ERROR] procJoin) [predessor Node]일 때 REQUEST timed out!\n");
				return -1;
			}
			printf("\a[ERROR] procJoin) [predessor Node]일 때 Sendto Error!\n");
			return -1;
		}
	}

	else 
	{
		/*요청 메시지 생성*/
		memset(&rqMsg, 0, sizeof(rqMsg));

		/*find_closet_predecessor*/
		for (int i = baseM - 1; i >= 0; i--) {
			if (modIn(ringSize, myNode.chordInfo.fingerInfo.finger[i].ID, myNode.nodeInfo.ID, targetKey, 0, 0))
			{
				recvNode = myNode.chordInfo.fingerInfo.finger[i];
				break;
			}
		}

		
		rqMsg.msgType = REQUEST;
		rqMsg.msgID = FINDPRECOMMAND;
		rqMsg.nodeInfo = myNode.nodeInfo;
		rqMsg.moreInfo = targetKey;
		rqMsg.bodySize = 0;

		if (!sMode)	
			printf("procJoin) [else] FINDPREDESSOR시 recvNode정보 : addr: %s, port : %d, id : %d\n",
			inet_ntoa(recvNode.addrInfo.sin_addr), ntohs(recvNode.addrInfo.sin_port), recvNode.ID);
		
		//if (!sMode)
			printf("procJoin) [else] FINDPREDESSOR 메시지 요청.\n");
	
		/*find_closet_predecessor에게 FINDPRE 요청*/
		retval = sendto(rqSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&recvNode.addrInfo, AddrLen);

		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("\a[ERROR] procJoin) [else] FINDPRED REQUEST timed out!\n");
				return -1;
			}
			printf("\a[ERROR] procJoin) [else] FINDPRED Sendto Error!\n");
			return -1;
		}

		if (!sMode)
			printf(" procJoin) [else] FINDPRED  waiting...\n");
		retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&recvAddr, &AddrLen);

		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("\a[ERROR] procJoin) [else] FINDPRED Receive timed out!\n");
				return -1;
			}
			printf("\a[ERROR] procJoin) [else] FINDPRED Recvfrom Error!\n");
			return -1;
		}

		rpNode = rpMsg.nodeInfo;

		/*Pre에게 SUCCINFO 요청 */
		memset(&rqMsg, 0, sizeof(chordHeaderType));
		rqMsg.msgType = REQUEST;
		rqMsg.msgID = SUCCINFOCOMMAND;
		rqMsg.nodeInfo = myNode.nodeInfo;
		rqMsg.moreInfo = 0;
		rqMsg.bodySize = 0;

		//if (!sMode)	
			printf("procJoin) [else]  SUCCINFO 요청메시지 전송\n");
			
		retval = sendto(rqSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&rpNode.addrInfo, AddrLen);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("\a[ERROR] procJoin) [else] SUCCINFO REQUEST timed out!\n");
				return -1;
			}
			printf("\a[ERROR] procJoin) [else] SUCCINFO Sendto Error!\n");
			return -1;
		}

		if (!sMode)
			printf("procJoin) [else] SUCCINFO waiting...\n");
		
		retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&recvAddr, &AddrLen);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("\a[ERROR] procJoin) [else] SUCCINFO Receive timed out!\n");
				return -1;
			}
			printf("\a[ERROR] procJoin) [else] SUCCINFO  Recvfrom Error!\n");
			return -1;
		}

		rpNode = rpMsg.nodeInfo;

		//if (!sMode)
			printf("procJoin) [else] SUCCINFOCOMMAND시 SUCC정보 : addr: %s, port : %d, id : %d\n",
			inet_ntoa(rpMsg.nodeInfo.addrInfo.sin_addr), ntohs(rpMsg.nodeInfo.addrInfo.sin_port), rpMsg.nodeInfo.ID);

		/*JOININFO REQUEST 전송*/
	//	memset(&rpMsg, 0, sizeof(chordHeaderType));

		rpMsg.msgType = RESPOND; // respond
		rpMsg.msgID = JOINCOMMAND;   // JoinInfo
		rpMsg.nodeInfo = rpNode;
		rpMsg.moreInfo = SUCCESS;
		rpMsg.bodySize = 0;
	
		/*요청노드에게 JOIN 응답(SUCC정보)*/
		//if (!sMode)
			printf("procJoin) [else] Node일 때 JOININFO 메시지 응답.\n");
		retval = sendto(rpSock, (char *)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&(msgInfo->senderSockAddr), AddrLen);

		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("\a[ERROR] procJoin) [else] Node일 때 JOININFO REQUEST timed out!\n");
				return -1;
			}
			printf("\a[ERROR] procJoin) [else] Node일 때 JOININFO Sendto Error!\n");
			return -1;
		}
	}

	return 1;
}