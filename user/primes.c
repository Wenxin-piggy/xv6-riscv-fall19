#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"
// #define BUFSZ  (MAXOPBLOCKS+2)*BSIZE
// char buf[BUFSZ];
int getPrimes(int *pipe1,int size){
    int count = 0;
    close(pipe1[1]);
    int first_num;
    // printf("size = %d\n",size);
    if(read(pipe1[0],&first_num,sizeof(first_num)) != 0){
        printf("prime %d\n",first_num);
        count ++;
    }
    if(size == 1){
        return 1;
    }
    int pipe2[2];
    if(pipe(pipe2) != 0){
        printf("pipe failed\n");
    }
    int buf;//除了第一个数之外的其他数
    int count_to_next = 0;
    int buffer[34];
    while((read(pipe1[0],&buf,sizeof(buf)) != 0 )&&(count < size)){
        //从左边读取数据
        count ++;
        if((buf % first_num) != 0){//可以整除，将其传过去
            // printf("buf = %d\n",buf);
            buffer[count_to_next] = buf;
            count_to_next ++;
            // if(write(pipe2[1],&buf,sizeof(buf)) == 0){
            //     printf("write falied\n");
            // }
            // else{
            //      count_to_next ++;
            // }
        }
    }
    close(pipe1[0]);
    if(count == size){
        
        int pid = fork();
        //开一个新线程
        if(pid < 0){
            printf("fork failed!\n");
        }
        else if(pid == 0){
            int done = getPrimes(pipe2,count_to_next);
            if(done == 1){
                exit();
            }
        }
        else{
            close(pipe2[0]);
            for(int i = 0;i < count_to_next;i ++){
                // printf("buffer = %d\n",buffer[i]);
                if(write(pipe2[1],&buffer[i],sizeof(buffer[i])) == 0){
                    printf("write falied\n");
                }
            }
            close(pipe2[1]);
            wait();
            return 1;
        }
        
    }
    return 0;
    
}
int main(int argc, char *argv[]){
    //创建第一个管道
    int pipe1[2];
    int nums[34];
    //初始化nums数组
    for(int i = 0;i < 34;i ++){
        //将数字存进数组里
        nums[i] = i + 2;
    }
     if(pipe(pipe1) != 0){
        printf("pipe(pipe1) failed\n");
        exit();
    }
    // 创建一个进程
    int pid;
    pid = fork();
    if(pid < 0){
        //创建线程失败
        printf("fork failed");
        exit();
    }
    else if(pid == 0){
        //child
        int done = getPrimes(pipe1,34);
        if(done == 1){
            exit();
        }
    }
    else{
        //parent
        //主线程将数据存进去
        // printf("parent\n");
        close(pipe1[0]);
        for(int i = 0;i < 34;i ++){
            if(write(pipe1[1],&nums[i],sizeof(nums[i])) == 0){
                printf("parent write %d to pipe error",nums[i]);
            }
        }
        close(pipe1[1]);
        wait();
        exit();
    }
    exit();
}