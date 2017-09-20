#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<fcntl.h>

void down_deal(int sockfd, char* file_name)
{

	file_name[strlen(file_name)-1] = '\0';

	send(sockfd, file_name, strlen(file_name), 0);
        
	file_name = file_name+5;
	char file_real_name[128] = "./lll/";
	strcat(file_real_name, file_name);
	printf("file_name:%s\n", file_real_name);
	int fd = open(file_real_name, O_CREAT | O_WRONLY, 0664);
	if(fd == -1)
	{
	    send(sockfd, "error", 5, 0);
	    return;
	}

	char buff[128] = {0};
	recv(sockfd, buff, 128, 0);
        if(strcmp(buff, "file open fail\n") == 0)
	{
	    printf("%s", buff);
	}

	int file_size = 0;
	sscanf(buff, "%d", &file_size);
	printf("%d:\n", file_size);
	int num = 0;
	int count = 0;
	char tmp[1024];
	printf("\033[?25l");
	while(count != file_size)
	{
	//printf("\n1:\n");
	    num = recv(sockfd, tmp, 1024, 0);
	//printf("\nnum:%d\n", num);
	    count += num;
	    printf("\rrecv count: %.2f%%  %d", count*100.0/file_size, count);
	    write(fd, tmp, num);
	    memset(tmp, 0, 128);
	}
	printf("\033[?25h");
	printf("\n");
	close(fd);
}
int main(int argc, char *argv)
{
    struct sockaddr_in ser;
    struct sockaddr_in cli;
    int ser_len = sizeof(ser);
    int snd_buff_size = 0;
    socklen_t opt_size = sizeof(snd_buff_size);

    ser.sin_family = AF_INET;
    ser.sin_port = htons(6500);
    ser.sin_addr.s_addr = inet_addr("127.0.0.1");

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd != -1);

    int size = getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &snd_buff_size, &opt_size);
    printf("snd_buff_size:%d\n", snd_buff_size);

    int res = connect(sockfd, (struct sockaddr*)&ser, ser_len);
    assert(res != -1);
    /*
    char enterbuff[1024] = {0};
    memset(enterbuff, 0, 1024);
    recv(sockfd, enterbuff, 1023, 0);
    printf("%s\n", enterbuff);
    */
    char buffer[20] = {"down b.avi\n"};
    down_deal(sockfd, buffer);
    return 0;
    while(1)
    {
        fflush(stdout);
        char buff[1024] = {0};
	printf("please input info:");
	fgets(buff, 1023, stdin);
	if(buff[0] == '\n')
	{
	    continue;
	}
	if(strncmp(buff, "end", 3) == 0)
	{
	    break;
	}
	if(strncmp(buff, "down", 4) == 0)
	{
	    down_deal(sockfd, buff);
	}
	else
	{
            send(sockfd, buff, strlen(buff)-1, 0);
	    memset(buff, 0, 1024);
	    recv(sockfd, buff, 1023, 0);
	    if(strncmp(buff, "ok#", 3) != 0)
	    {
	        printf("ERR:%s\n", buff);
	    }
	    else if(strcmp(buff, "ok#") == 0)
	    {
	        continue;
	    }
	    else
	    {
	        printf("%s", buff+3 );
	    }
        }
    
    }
    close(sockfd);
    return 0;
}
