#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <pthread.h>
#include <signal.h>
#include <time.h>

#define MSG_SIZE 80
#define BUF_SIZE 4096
#define FILENAMESIZE 255
#define null NULL
#define THREADPERWORK 3
key_t key_id, key_id2;  // , key_id3
int clientCnt = 0;
void* filerecv(void* arg);
pthread_t thread[3];


/*typedef struct tTHREAD
{
	char ddmsg[MSG_SIZE];
	int cCnt;
}THREAD;
*/
struct threadArg {
	char fifoFileName[FILENAMESIZE];
	long long fileSize;
	int number;
};

void error_handler(char* linkFileName) {
	//  unlink(linkFileName);
	perror("handler err\n");
	exit(1);
}

void decoding(char code[], int len)
{
	for (int i = 0; i < len; i++)
	{
		code[i] = code[i] - 1;
	}
}
void requestPasing(char* request[3], char buf[])
{
	int cnt = 0;
	request[0] = &buf[0];
	for (int i = 0; i < BUF_SIZE; i++) {
		if (cnt > 2)
			break;
		if (buf[i] == ' ') {
			buf[i] = '\0';
			cnt++;
			request[cnt] = &buf[i + 1];
		}
	}
}
//THREAD recvbuf[3];
//THREAD sendbuf[3];
//int sender_id;
//sem_t threadrock;

/*void make_Fifo() {
	if (mkfifo("./managefifo", 0666) == -1) { //fifo init
		printf("fail to make fifo manage()\n");
		exit(1);
	}
}*/
/*
void open_Fifo() {
	if ((filedesmf = open("./managefifo", O_WRONLY | O_TRUNC)) < 0) { //waiting in client
		printf("fail to call open manage()\n");
		exit(1);
	}
}
*/
/*void snd_Data() {
	if (msgsnd(key_id, dmsg, MSG_SIZE) == -1) { //fifo rw
		printf("fail to call msgsnd()\n");
		exit(1);
	}
}*/
/*
void rcv_Data() {
	if (msgrcv(key_id2, arriveMsg, MSG_SIZE, getpid(), 0) == -1) {
		perror("msgrcv error");
		exit(1);
	}
	printf("receive success\n");
}*/
//server
int main()
{
	char buf[BUF_SIZE];
	//char buf2[BUF_SIZE];
	int protocol;
	char* request[3];
	//int readlen=0;
	struct threadArg* argument;

	key_id = msgget((key_t)60041, IPC_CREAT | 0666); //send queue
	key_id2 = msgget((key_t)60042, IPC_CREAT | 0666); //recv queue
	//key_id3= msgget((key_t)60043, IPC_CREAT|0666); //result queue

	printf("시작 전\n");

	requestPasing(request, buf);
	// requestPasing(request,buf2);
	for (int i = 0; i < THREADPERWORK; i++) {
		argument = (struct threadArg*)malloc(sizeof(struct threadArg));
		sprintf(argument->fifoFileName, "./%sFIFO%d", request[1], i + 1);
		argument->number = i + 1;
		argument->fileSize = atoll(request[2]);
		pthread_create(&thread[i], NULL, (void*)filerecv, (void*)argument/*, (void*)& recvbuf[i]*/);
	}
	memset(buf, 0x00, BUF_SIZE);
	// memset(buf2,0x00,BUF_SIZE);

	pthread_exit(0);
	/*for (int i = 0; i <= THREADPERWORK; i++)
	{
		pthread_join(thread[i], NULL);
	}*/
	//sem_destroy(&threadrock);

	//unlink("./managefifo");

	if (msgctl(key_id2, IPC_RMID, NULL) == -1) {
		printf("msgctl failed\n");
		exit(1);
	}

	return 0;
}

void* filerecv(void* arg) {

	int fifo2Ser;
	int fifo2Cli;
	//sender_id = getpid();
	int fd;
	char buf[BUF_SIZE];
	//char buf2[BUF_SIZE];
	char tempFileName[FILENAMESIZE];
	char fifo2SerFileName[FILENAMESIZE];
	char fifo2CliFileName[FILENAMESIZE];
	int tempfd;
	int readlen = 0;
	/*매개변수 저장*/
	struct threadArg* argument = (struct threadArg*)arg;
	sprintf(fifo2SerFileName, "%s2ser", argument->fifoFileName);
	sprintf(fifo2CliFileName, "%s2cli", argument->fifoFileName);
	sprintf(tempFileName, "./%stemp.txt", argument->fifoFileName);

	if ((fifo2Ser = open(fifo2SerFileName, O_RDONLY | O_NONBLOCK)) < 0) {
		//printf("ser fail to call open manage()\n");
		printf(" ");
		//error_handler("./managefifo",fifo2SerFileName,null,null);
	}

	if ((fifo2Cli = open(fifo2CliFileName, O_WRONLY)) < 0) {
		printf(" ");
		//printf("cli fail to call open manage()\n");
		//error_handler("./managefifo", fifo2SerFileName, fifo2CliFileName,null);
	}

	if ((tempfd = open(tempFileName, O_RDWR | O_CREAT)) < 0) {
		printf(" ");
		//printf("tempfd fail to call open manage()\n");
	//error_handler("./managefifo",fifo2SerFileName,fifo2CliFileName,tempFileName);
	}

	if (msgrcv(key_id2, (void*)& buf, BUF_SIZE, 0, 0) == -1) {
		perror("msgrcv error");
		exit(1);
	}
	printf("\nthread%d receive success\n", argument->number);
	printf("%s\n", buf);
	write(fifo2Ser, buf, BUF_SIZE);
	read(fifo2Ser, buf, BUF_SIZE);
	decoding(buf, BUF_SIZE);
	write(tempfd, buf, BUF_SIZE);
	//readflag=1;
//}
//decoding(buf,readlen);
//write(tempfd,buf,readlen);

//else {

	//strcpy(buf2, buf);
	lseek(tempfd, 0, SEEK_SET);
	while ((readlen = read(tempfd, buf, BUF_SIZE)) > 0) {
		if (msgsnd(key_id, (void*)& buf, BUF_SIZE, 0) == -1) {
			perror("msgsnd error");
			exit(1);
		}
		printf("\nthread%d send success\n", argument->number);
		printf("%s\n", buf);
		//   }
		   /*sendbuf[i].cCnt = recvbuf[i].cCnt;
		   //sendbuf[n].ddmsg = recvbuf[n].ddmsg
		   strcpy(sendbuf[i].ddmsg, recvbuf[i].ddmsg);
		   //sendbuf[i].ddmsg = sendbuf[i].ddmsg - 32;
		   if (msgsnd(key_id, (void*)& sendbuf[i], sizeof(struct tTHREAD), 0) == -1) { //fifo rw
			   printf("fail to call msgsnd()\n");
			   exit(1);
		   }
		   printf("send success\n");
		   printf("%d. %s\n", sendbuf[i].cCnt, sendbuf[i].ddmsg);
		   */
		   //}
	}

}
