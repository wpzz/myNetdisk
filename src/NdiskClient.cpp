#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<sys/socket.h>

int main()
{
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd != -1);

    struct sockaddr_in ser;
    struct sockaddr_in cli;
    memset(&ser, 0, sizeof(ser));

    ser.sin_family = AF_INET;
    ser.sin_port = htons(2111);
    ser.sin_addr.s_addr = inet_addr("127.0.0.1");

    int res = connect(sockfd, (struct sockaddr*)&ser, sizeof(ser));
    assert(res != -1);

    while(1)
    {
        fflush(stdout);
        char file_name[128] = {0};
        printf("please input file name\n");
	fgets(file_name, 127, stdin);
	file_name[strlen(file_name)-1] = '\0';

	send(sockfd, file_name, strlen(file_name), 0);
        
	char file_real_name[128] = "./lll/";
	strcat(file_real_name, file_name);
	int fd = open(file_real_name, O_CREAT | O_WRONLY, 0664);
	if(fd == -1)
	{
	    send(sockfd, "error", 5, 0);
	    continue;
	}

	char buff[128] = {0};
	recv(sockfd, buff, 128, 0);
	if(strcmp(buff, "file open fail\n") == 0)
	{
	    printf("%s", buff);
	}

	int file_size = 0;
	sscanf(buff, "%d", &file_size);
	
	int num = 0;
	int count = 0;
	char tmp[128];
	while(count != file_size)
	{
	    num = recv(sockfd, tmp, 128, 0);
	    count += num;
	    printf("recv count %.2f%\n", count*100.0/file_size);
	    write(fd, tmp, num);
	    memset(tmp, 0, 128);
	}
	close(fd);
    }

    return 0;
}
