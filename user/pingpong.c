#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"
#define BUFSZ  (MAXOPBLOCKS+2)*BSIZE
char buf[BUFSZ];
int main(int argc, char *argv[]){
    int parent_fd[2],pid;
    int child_fd[2];
   
    //创建一个管道
    if(pipe(parent_fd) != 0){
        printf("pipe(parent_fd) failed\n");
        exit();
    }
    if(pipe(child_fd) != 0){
        printf("pipe(child_fd) failed!\n");
        exit();
    }
    //创建一个线程
    pid = fork();
    if(pid < 0){
        printf("fork failed");
        exit();
    }
    else if(pid == 0){
        //child
        close(parent_fd[1]);
        if(read(parent_fd[0],buf,sizeof(buf)) == 4){
            //到了传过来的东西
            if(strcmp("ping",buf) == 0){
                int pid_child = getpid();
                printf("%d: received %s\n",pid_child,buf);
                //往里面写回应的东西
                close(child_fd[0]);
                if(write(child_fd[1],"pong",4) != 4)
                    printf("children write error");
                close(child_fd[1]);
                close(parent_fd[0]);
                exit();
            }
        }
        close(parent_fd[0]);
    }
    else{
        //parent
        close(parent_fd[0]);
        if(write(parent_fd[1],"ping",4) != 4)
            printf("parent write error");
        close(parent_fd[1]);
        close(child_fd[1]);
        if(read(child_fd[0],buf,sizeof(buf)) == 4){
            if(strcmp("pong",buf) == 0){
                int pid_parent = getpid();
                printf("%d: received %s\n",pid_parent,buf);
                close(child_fd[0]);
                exit();
            }
        }
        close(child_fd[0]);
    }
    exit();
}