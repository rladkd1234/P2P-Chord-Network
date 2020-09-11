#include "header.c"

unsigned WINAPI procFileRecv(void *arg)
{
	printf("file down Recv start\n");
	//	int sender_sock;
	FILE* file_num = NULL;
	char buf[1024];
	int read_len;
	//	int file_read_len;
	//	struct sockaddr_in sender_addr;

		//int *exitFlag = (int*)arg;
	int AddrLen = sizeof(struct sockaddr_in);

	if ((frSock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("procFileRecv) Error) socket failed!");
		exit(1);
	}
	if (!sMode)
		printf("procFileRecv) connecting node ID : %d\n", sharing.msg.nodeInfo.ID);
	if (!sMode)
		printf("procFileRecv) connetcing node address : %s\n", inet_ntoa(sharing.msg.nodeInfo.addrInfo.sin_addr));

	if (-1 == connect(frSock, (struct sockaddr*) &sharing.senderSockAddr, AddrLen))
	{
		perror("procFileRecv) ERROR) CONECCT FAILED\n");
		exit(1);
	}
	//Sleep(3000);
	char file_name[1024];
	memset(buf, 0x00, 1024);
	//sender_sock = accept(frSock, (struct sockaddr*)&sharing.senderSockAddr, AddrLen);
	recv(frSock, (buf), 1024, 0);
	if (!sMode)
		printf("procFileRecv) file read finish : %s\n", buf);
	strcpy(file_name, buf);

	file_num = fopen(file_name, "wt");
	if (file_num == NULL)
	{
		perror("procFileRecv) ERROR) procFileRecv) file open failed\n");
		exit(1);
	}

	memset(&sharing.msg, 0, sizeof(chordHeaderType));
	while (recv(frSock, (char*)&sharing.msg, 1024, 0) != 0)
	{
		read_len = sharing.msg.moreInfo;
		if (!sMode)
			printf("procFileRecv) readed File length : %d\n", read_len);
		//printf("%s\n", buf);
		recv(frSock, buf, 1024, 0);
		fwrite(buf, sizeof(char), read_len, file_num);
	}

	printf("procFileRecv) file Receive complete.\n");
	fclose(file_num);

	return 1;
	//file_num = open(file_name, O_WRONLY | O_CREAT | O_EXCL, 0700);
	/*file_read_len = sharing.msg.moreInfo;
	printf("파일 크기 : %d\n", file_read_len);
	while (1)
	{
	memset(buf, 0x00, 1024);
	recv(frSock, buf, 1024,0);
	write(file_num, buf, file_read_len);
	file_read_len -= 1024;
	if (file_read_len <0)
	{
	printf("파일 다운로드를 마쳤습니다.\n");
	break;
	}
	}*/
	/*while ((read_len = read(frSock, buf, 1024)) != 0)
	{
	fwrite((void*)buf, 1, read_len, file_num);
	}*/


	//connect해서 접속했으니 접속이된상태
	//TCP 연결과정이 기억이 안남




}