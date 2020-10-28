#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

char *argv[MAXARG];
char *op;


int 
main (int argc, char *argv[]){
    char *argv2[MAXARG];
    int argv2_i = 0;
    int start = 0;
    if(argc == 1){
        op = "echo";
        argv2[0] = op;
        argv2_i = 1;
    }
    else{
        op = argv[1];
        for(int i = 1;i < argc;i ++){
            argv2[argv2_i] = argv[i];
            argv2_i ++;
        }
    }
    char buf;
    char temp[512];
    int temp_i = 0;
    start = argv2_i;
    while(read(0,&buf,sizeof(buf))){
        if(buf == '\n'){
            temp[temp_i ++] = '\0';  
            argv2[argv2_i] = malloc(sizeof(char)*(strlen(temp) + 1));      
            memmove(argv2[argv2_i],temp,strlen(temp));
            argv2_i ++;
            temp_i = 0;
            // printf("op:%s\n",*op);
            // for(int i = 0;i < argv2_i;i ++){
            //     printf("argv[%d] = %s\n",i,argv2[i]);
            // }
            argv2_i = start;
            if(fork() == 0){
                exec(op,argv2);
            }
            else{
                wait();
            }
            
        }
        else if(buf == ' '){
            // printf("kongge,i = %d temp = %s\n",argv2_i,temp);
            temp[temp_i ++] = '\0';    
            argv2[argv2_i] = malloc(sizeof(char)*(strlen(temp) + 1));      
            memmove(argv2[argv2_i],temp,strlen(temp));
            argv2_i ++;
            // for(int i = 0;i < argv2_i;i ++){
            //     printf("kongge argv[%d] = %s\n",i,argv2[i]);
            // }
            temp_i = 0;
        }
        else{
            temp[temp_i] = buf; 
            temp_i ++;
        }
    }
    exit();
}