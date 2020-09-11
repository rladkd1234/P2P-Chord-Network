#include "header.c"

unsigned WINAPI procStabilizeLeave(void *arg)
{
	chordHeaderType rqMsg, rpMsg;
	struct sockaddr_in receiveAddr;
	printf("Stabilize Leave Start \n");
	int *i = &sharing.msg.moreInfo;
	int AddrLen = sizeof(struct sockaddr_in);
	int j = baseM - 1;
	int try = 0;
	struct sockaddr_in temp_addr;
	printf("%d\n", (*i));
	for (;;)
	{
	/*	if (myNode.chordInfo.fingerInfo.Pre.ID == myNode.chordInfo.fingerInfo.finger[(*i)].ID)
		{
			for (int k = 0; k < baseM; k++)
			{
				//myNode.chordInfo.fingerInfo.Pre
					
			}
		}*/
		memset(&rqMsg, 0, sizeof(chordHeaderType));

		rqMsg.msgType = REQUEST;
		rqMsg.msgID = PREINFOCOMMAND;
		rqMsg.nodeInfo = myNode.nodeInfo;
		rqMsg.moreInfo = myNode.chordInfo.fingerInfo.finger[(*i)].ID;
		rqMsg.bodySize = 0;
		if (try != 0)
		{
			sendto(rqSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&temp_addr, AddrLen);
			recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
			temp_addr = rpMsg.nodeInfo.addrInfo;

		}
		if (try == 0)
		{
			sendto(rqSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&myNode.chordInfo.fingerInfo.finger[j].addrInfo, AddrLen);
			recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
			temp_addr = rpMsg.nodeInfo.addrInfo;
			try++;
		}
		if (rpMsg.nodeInfo.ID == myNode.nodeInfo.ID)
		{
			myNode.chordInfo.fingerInfo.finger[(*i)].ID = myNode.nodeInfo.ID;
			myNode.chordInfo.fingerInfo.finger[(*i)].addrInfo = myNode.nodeInfo.addrInfo;
			return;
		}
		if (rpMsg.nodeInfo.ID == myNode.chordInfo.fingerInfo.finger[(*i)].ID)
		{
		
			memset(&rqMsg, 0, sizeof(chordHeaderType));

			rqMsg.msgType = REQUEST;
			rqMsg.msgID = PREUPDATECOMMAND;
			rqMsg.nodeInfo = myNode.nodeInfo;
			rqMsg.moreInfo = 0;
			rqMsg.bodySize = 0;
			sendto(rqSock, (char *)&rqMsg, sizeof(rqMsg), 0, (struct sockaddr*)&temp_addr, AddrLen);
			recvfrom(rqSock, (char*)&rpMsg, sizeof(rpMsg), 0, (struct sockaddr*)&receiveAddr, &AddrLen);
			if (rpMsg.moreInfo == SUCCESS)
			{
				myNode.chordInfo.fingerInfo.finger[(*i)].addrInfo = rpMsg.nodeInfo.addrInfo;
				myNode.chordInfo.fingerInfo.finger[(*i)].ID = rpMsg.nodeInfo.ID;

				printf("ÀßµÆ¶ì~\n");
				break;
			}
			else
			{
				perror("¿¡·¯¶ì~\n");
				exit(1);
			}
		}
	}

	return 0;
}