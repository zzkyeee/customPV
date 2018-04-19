#include <stdio.h>
#include<pthread.h>
#include <stdlib.h>
#include <unistd.h>　
#include<semaphore.h>
#include<string.h>
#define True 1
#define False 0
#define READER_NUM 5
#define WRITER_NUM 2
#define MAX_TIME 12
#define MIN_TIME 4
//进程控制快
typedef struct process_control_block{
    char processName[100];
    struct process_control_block *next; //下一个pcb
}*pcbList,pcb;
sem_t *rName,*wName;
typedef struct{
    int value;  //资源数目
    //sem_t *mutex;
    char *name;
    pcbList volatile list;
}semaphore;  //信号量结构体

//信号量
semaphore *rmutex,*wmutex;
int readcount=0,readName=0,writeName=0;
void block(semaphore *S,pcbList current){
    pcb *temp=S->list;
    //插入到链表末尾
    while(temp->next!=NULL){
        temp=temp->next;
    }
    temp->next=current;
    while(True){
        if(S->list==current){
            break;
        }
    }
}
void wakeup(semaphore *S){
    if(S->list->next!=NULL){
        S->list=S->list->next;
    }
}
//P操作
void pWait(semaphore* S,pcbList current) {
    if(--S->value<0){
        printf("%s 正在等待...\n",current->processName);
        block(S,current);
    }
}
//V操作
void pSignal(semaphore* S,pcbList current){
    if(++S->value<=0)
        wakeup(S);
}
void *Reader(){
    pcb *current;
    current=(pcb *) malloc(sizeof(pcb));
    sem_wait(rName);
    char *name="读者";
    readName++;
    char buffer[100];
    sprintf(buffer,"%s%d",name,readName);
    sem_post(rName);

    while(True){
        current->next=NULL;
        strcpy(current->processName,buffer);
        sleep((rand()%(MAX_TIME-MIN_TIME))+MIN_TIME);
        pWait(rmutex,current);
        if(readcount==0)
            pWait(wmutex,current);
        printf("%s正在读...\n",current->processName);
        readcount++;
        printf("当前阅读人数%d\n",readcount);
        pSignal(rmutex,current);
        sleep((rand()%(MAX_TIME-MIN_TIME))+MIN_TIME);
        printf("%s准备离去...\n",current->processName);
        pWait(rmutex,current);
        readcount--;
        printf("当前阅读人数%d\n",readcount);
        printf("%s阅读完毕...\n",current->processName);
        if(readcount==0){
            pSignal(wmutex,current);
        }
        pSignal(rmutex,current);
        sleep(MAX_TIME);
    }
}
void *Writer(){
    sem_wait(wName);
    char *name="写者";
    writeName++;
    char buffer[100];
    sprintf(buffer,"%s%d",name,writeName);
    sem_post(wName);

    while(True){
        pcb *current;
        current=(pcb *) malloc(sizeof(pcb));
        strcpy(current->processName,buffer);
        current->next=NULL;
        pWait(wmutex,current);
        printf("%s正在写...\n",current->processName);
        sleep((rand()%(MAX_TIME-MIN_TIME))+MIN_TIME);
        printf("%s写完毕...\n",current->processName);
        pSignal(wmutex,current);
        sleep(MIN_TIME);
    }
}
//信号量初始化
void initMutex(){
    sem_unlink("wName");
    sem_unlink("rName");
    wName=sem_open("wName", O_CREAT, 0, 1);;
    rName=sem_open("rName", O_CREAT, 0, 1);;
    rmutex=(semaphore *)malloc(sizeof(semaphore));
    wmutex=(semaphore *)malloc(sizeof(semaphore));
    pcbList rList=(pcbList) malloc(sizeof(pcb));
    strcpy(rList->processName,"rList Source");
    rList->next=NULL;
    pcbList wList=(pcbList) malloc(sizeof(pcb));
    strcpy(wList->processName,"wList Source");
    wList->next=NULL;
    rmutex->list=rList;
    wmutex->list=wList;
    rmutex->value=1;
    rmutex->name="rmutex";
    wmutex->value=1;
    wmutex->name="wmutex";
}
int main() {
    int i,err,reader_num=READER_NUM,writer_num=WRITER_NUM;
    pthread_t writer[writer_num];
    pthread_t reader[reader_num];
    initMutex();
    //创建读者进程
    printf("创建了%d个读者\n",reader_num);
    for(int i=0;i<reader_num;i++){
        err = pthread_create(&reader[i], NULL, (void *)Reader, NULL);
        //sleep(1);
    }

    //创建写者线程
    printf("创建了%d个写者\n",writer_num);
    for(int i=0;i<writer_num;i++)
        err=pthread_create(&writer[i],NULL,(void *)Writer,NULL);

    //线程阻塞
    for(i=0;i<writer_num;i++)
        pthread_join(writer[i],NULL);
    for(i=0;i<reader_num;i++)
        pthread_join(reader[i],NULL);
}