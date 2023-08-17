#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAXLEN 32

int main(int argc, char *argv[]) {
    // 执行命令
    char *path = "echo";
    // 存放参数的数组和指针
    char buf[MAXLEN * MAXARG] = {0},*p;
    // 参数的指针数组
    char *params[MAXARG];
    // xargs后命令所带的参数个数
    int oriParamCnt = 0;
    // 参数序号
    int paramIdx;
    // 参数长度
    int paramLen;
    int i;
    // 临时记录读取字符
    char ch;
    // read读取长度, 用于判断是否输入结束
    int res;

    // 参数数量大于1
    if (argc > 1) {
        // 提取指令
        path = argv[1];
        // 设置参数, 注意也需要带上指令的参数
        for (i = 1; i < argc; ++i) {
            params[oriParamCnt++] = argv[i];
        }
    } else {    //参数唯一,即只有xargs
        // 即指令为echo
        params[oriParamCnt++] = path;
    }
    
    // 后续参数起始序号
    paramIdx = oriParamCnt;
    p = buf;
    paramLen = 0;
    while (1) {
        res = read(0, p, 1);
        ch = *p;
        
        // 若读取的为一般字符
        if (res != 0 && ch != ' ' && ch != '\n') {
            ++paramLen;
            ++p;
            continue;
        }
        // 未读取成功, 或者读取的是空格或回车
        // 计算参数起始位置
        params[paramIdx++] = p - paramLen;
        // 参数长度置0
        paramLen = 0;
        // 设置字符结束符
        *p = 0;
        ++p;
        // 若读取的参数超过上限
        if (paramIdx == MAXARG && ch == ' ') {
            // 一直读到回车即下个命令为止
            while ((res = read(0, &ch, 1)) != 0) {
                if (ch == '\n') {
                    break;
                }
            }
        }
        // 若读取的不为空格, 即 res==0||ch=='\n'
        if (ch != ' ') {
            // 创建子进程执行命令
            if (fork() == 0) {
                exec(path, params);
                exit(0);
            } else {
                // 父进程等待子进程
                wait((int *) 0);
                // 重置参数序号和指针
                paramIdx = oriParamCnt;
                p = buf;
            }
        }
        // 若输入读取完毕则退出
        if (res == 0) {
            break;
        }
    }
    exit(0);
}
