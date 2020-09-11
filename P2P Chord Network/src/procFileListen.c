#include "header.c"

unsigned WINAPI procFileListen(void *arg)
{

	int file_name_len;
	char buf[1024];
	FILE* file_source = NULL;
//	int temp;
	int read_len = 1;
//	struct sockaddr_in clientaddr;
	printf("file listen start\n");
	int *exitFlag = (int*)arg;
	int AddrLen = sizeof(struct sockaddr_in);
//	int retval;
	if ((flSock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("procFileListen) Error: socket failed!");
		exit(1);
	}
	/*flSock bind*/
	if (-1 == bind(flSock, (struct sockaddr*)&myNode.nodeInfo.addrInfo, AddrLen))
	{
		perror("procFileListen) flsock binding Error");
		exit(1);
	}
	/*listen(TCP)*/
	if (-1 == listen(flSock, 5))
	{
		perror("procFileListen) flsock Listen Error");
		exit(1);
	}
	//소켓바인딩/리슨
	while (!(*exitFlag)) {
		//커넥트
		//if (!sMode)
			printf("procFileListen) connected node address : %s\n", inet_ntoa(sharing.senderSockAddr.sin_addr));
	
		/*accept*/
		fsSock = accept(flSock, (struct sockaddr*)&sharing.senderSockAddr, &AddrLen);
		
		//printf("접속하려는거 ? : %s\n", inet_ntoa(sharing.senderSockAddr.sin_addr));

		//printf("Detected Who Wants to download your file : %s ", inet_ntoa(sharing.senderSockAddr.sin_addr));
		//sharing.senderSockAddr = clientaddr;
	//	if (!sMode)
			printf("procFileListen) sended file name : %s\n", myNode.fileInfo.fileRef[sharing.msg.moreInfo].Name);

		file_name_len = strlen(myNode.fileInfo.fileRef[sharing.msg.moreInfo].Name);

		send(fsSock, myNode.fileInfo.fileRef[sharing.msg.moreInfo].Name, file_name_len, 0);
		//다운받아서 추가한것도 파일 추가로 보고 코드테이블을 갱신해야 하는가?
		file_source = fopen(myNode.fileInfo.fileRef[sharing.msg.moreInfo].Name, "r");
		if (file_source == NULL)
		{
			perror(" ERROR] procFileListen) file open\n");
			exit(1);
		}
		memset(&sharing.msg, 0, sizeof(chordHeaderType));
		while (1)
		{
			file_name_len = fread(buf, sizeof(char), 1024, file_source);
			sharing.msg.moreInfo = file_name_len;
			//if (!sMode)
				printf("procFileListen) sending file size : %d\n", sharing.msg.moreInfo);
			send(fsSock, (char*)&sharing.msg, sizeof(chordHeaderType), 0);
			//printf("%s\n", buf);
			send(fsSock, buf, file_name_len, 0);
			if (feof(file_source))
				break;
		}
		printf("procFileListen) file transport end\n");
		if (shutdown(fsSock, SD_SEND) == -1)
		{
			perror("procFileListen) ERROR) fsSock shutdown\n");
			exit(1);
		}
		closesocket(fsSock);
		fclose(file_source);
		/*fseek(file_source, 0, SEEK_END);
		int total = ftell(file_source);
		printf("보내려는 파일크기 : %d \n", total);
		send(fsSock, (char*)&total, sizeof(total), 0);

		rewind(file_source);
		while (1) {
		file_name_len = fread(buf, 1, 1024, file_source);
		if (file_name_len > 0) {
		if (-1 == send(fsSock, buf, file_name_len, 0))
		{
		perror("오류\n");
		exit(1);
		}

		read_len += file_name_len;
		}
		else if (file_name_len == 0 && read_len == total) {
		printf("파일 전송 완료 했습니다.\n");
		break;
		}
		else {
		perror("파일 입출력 오류");
		exit(1);
		}
		}*/
		/*while (1)
		{
		memset(buf, 0x00, 1024);
		read_len = _read(file_source, buf, 1024);
		send(fsSock, buf, read_len, 0);
		if (read_len == 0)
		{
		break;
		}
		}
		if (read_len == 0)
		break;*/
		/*while (1)
		{
		read_len = fread((void*)buf, 1, 1024, file_source);
		if (read_len < 1024)
		{
		write(fsSock, buf, read_len);
		break;
		}
		write(fsSock, buf, read_len);
		}*/
	}
	//flsock임
	//recvthread에서 파일 다운로드 요청을 받았을 경우 flsock을 넘김
	//파일을 원하는 유저는 flsock을 받으면 filerecvthread 를 생성하고 frsock으로 flsock에게 요청함
	//flsock에서 요청이 들어오면 fssock으로 파일을 파일 크기만큼 전송함


	return 1;

}