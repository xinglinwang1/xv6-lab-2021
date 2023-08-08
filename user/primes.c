#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void prime(int fd){
    int factor;
    //读出第一个数设为factor
    if(read(fd, &factor, sizeof(int)) == 0){
        //管道为空，直接exit
        exit(0);
    }

    printf("prime %d\n", factor);
    //管道不为空，创建新的管道
    int p[2];
    pipe(p);
    //子进程递归调用prime
    if(fork() == 0){
        //读端作为参数给新的子进程
        close(p[1]);
        prime(p[0]);
    }
    //父进程
    else{
        close(p[0]);
        int divideNum;
        //读取管道中的数，若不能被factor整除，则发送给子进程
        while(read(fd, &divideNum, sizeof(int))){
            if(divideNum % factor != 0){
                write(p[1], &divideNum, sizeof(int));
            }
        }
        close(p[1]);
    }

    wait(0);
    exit(0);
}

int main(int argc, char const *argv[])
{
    int p[2];
    pipe(p);
    if(fork() == 0){
        close(p[1]);
        prime(p[0]);
    }
    else{
        close(p[0]);
        for(int i = 2; i <=35; i++){
            write(p[1], &i, sizeof(int));
        }
        close(p[1]);
    }
    wait(0);
    exit(0);
}