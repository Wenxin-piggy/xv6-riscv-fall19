#include "kernel/types.h"
#include "kernel/memlayout.h"
#include "user/user.h"
int main(int argc,char *argv[]){
    if(argc == 1){
        printf("maybe you forget to input the argument...\n");
    }
    else{
        char* temp = argv[1];
        int n = atoi(temp);
        printf("sleep %d...\n",n);
        sleep(n);
    }
    exit();
}