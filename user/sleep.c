#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

//argc是传入main函数的参数个数，argv是传入main函数的参数列表
int main(int argc, char *argv[])
{
    int sec;
    //先处理错误情况：没有输入参数
    if (argc <= 1)
    {
        printf("Please pass an argument. Input format: sleep [seconds]\n");
        exit(0);
    }
    sec = atoi(argv[1]);
    sleep(sec);
    exit(0);
}
