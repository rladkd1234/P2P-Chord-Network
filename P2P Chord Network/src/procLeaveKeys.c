#include "header.c"

unsigned WINAPI procLeaveKeys(void *arg)
{
	struct sockaddr_in receiveAddr;
	int AddrLen = sizeof(struct sockaddr_in);
	int retval;
	chordHeaderType rqMsg, rpMsg;
	for (int i = 0; i < (int)myNode.fileInfo.fileNum; i++)
	{
		memset(&rqMsg, 0, sizeof(chordHeaderType));
		rqMsg.msgType = REQUEST;
		rqMsg.msgID = FILEDELETECOMMAND;
		rqMsg.nodeInfo = myNode.nodeInfo;
		rqMsg.moreInfo = 0;
		rqMsg.bodySize = 0;
		rqMsg.fileInfo = myNode.fileInfo.fileRef[i];
		/*소유한 File의 Ref에게 FILEDELE 요청*/
		retval = sendto(rqSock, (char*)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&myNode.fileInfo.fileRef[i].refOwner.addrInfo, AddrLen);

		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("\a[ERROR] ProcLeaveKeys) FILEDELETE request timed out!\n");

				continue;
			}
			printf("\a[ERROR] ProcLeaveKeys) FILEDELETE sendto Error!\n");
			continue;
		}//지우는거 그대로 사용
		//여기선 지우고
	}
	for (int i = 0; i < (int)myNode.chordInfo.FRefInfo.fileNum; i++)
	{
		memset(&rqMsg, 0, sizeof(chordHeaderType));

		rqMsg.msgID = LEAVEKEYSCOMMAND;
		rqMsg.msgType = REQUEST;
		rqMsg.nodeInfo = myNode.nodeInfo;
		rqMsg.moreInfo = 0;
		rqMsg.bodySize = 0;
		rqMsg.fileInfo = myNode.chordInfo.FRefInfo.fileRef[i];
		/*Succ에게 LEAVEKEYS 요청*/
		retval = sendto(rqSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&myNode.chordInfo.fingerInfo.finger[0].addrInfo, AddrLen);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("\a[ERROR]  ProcLeaveKeys) leavekeys REQUEST timed out!\n");

				continue;
			}
			printf("\a[ERROR]  ProcLeaveKeys) leavekeys Sendto Error!\n");
			continue;
		}

		retval = recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr *) &receiveAddr, &AddrLen);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("\a[ERROR]  ProcLeaveKeys)leavekeys RECEIVE timed out!\n");

				continue;
			}
			printf("\a[ERROR]  ProcLeaveKeys) leavekeys Recvfrom Error!\n");
			continue;
		}


	}
	return 1;


}