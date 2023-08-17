#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

char buf[512];

int main(int argc, char *argv[])
{
    //创建双向管道，用于父子进程间通信
    int p1[2], p2[2];
    pipe(p1);
    pipe(p2);
    //创建子进程
    if(fork() == 0){
        //printf("child process\n");
        //不使用的文件描述符随时关闭。
        close(p1[0]);
        close(p2[1]);
        //子进程成功接收到父进程发送的字节
        if(read(p2[0], buf, 1) == 1){
            printf("%d: received ping\n", getpid());
            //将buf上的一个字节写入父进程
            write(p1[1], buf, 1);
        }
    }
    else{
        //printf("parent process\n");
        close(p1[1]);
        close(p2[0]);
        //将buf上的一个字节写入子进程（即父母向孩子发送（buf中的）一个字节）
        write(p2[1], buf, 1);
        //父进程成功接收到子进程发送的字节
        if(read(p1[0], buf, 1) == 1){
            printf("%d: received pong\n", getpid());
        }
    }
    
    exit(0);
}