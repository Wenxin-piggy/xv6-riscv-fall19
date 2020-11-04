#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
# define NULL 0
void
panic(char *s)
{
  fprintf(2, "%s\n", s);
  exit(-1);
}

int
fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

int getcmd(char *buf, int nbuf)
{
  fprintf(2, "@ ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int parsecmdwithpipe(char* buf,char * command1,char * command2){
    int flag = 0;
    int pos = -1;
    for(int x = 0;x < strlen(buf);x ++){
        //扫一次，看看有没有pipe的标识
        if(buf[x] == '|'){
            //有pipe
            pos = x;
            flag = 1;
            break;//纪录|的位置
        }
    }
    if(flag == 0){//其中没有|分割
        int x;
        for(x = 0;x < strlen(buf) - 1;x ++){
            //最后一个是换行号
            command1[x] = buf[x];
        }
        command1[x] = 0;//字符串以\0结尾
        return flag;
    }
    else{//其中有|分割
        int x = 0;
        for(x = 0;x < pos - 1;x ++){
            command1[x] = buf[x];
        }
        command1[x] = 0;//字符串以\0结尾
        int y = -1;
        for(x = pos + 2;x < strlen(buf) - 1;x ++){
            command2[++y] = buf[x];
        }
        command2[++y] = 0;
        return flag;
    }
}
void parsecmd(char* buf){
    char command1[100];
    char command2[100];
    int pipe_flag = parsecmdwithpipe(buf,command1,command2);//按照|解析
    if(pipe_flag == 1){
        //中间有管道
        int p[2];
        if(pipe(p) < 0){
            panic("pipe");
        }
        if(fork1() == 0){
            //左端，将输出截止，传入pipe
            int input_flag = 0;
            int input_pos = -1;
            char * argv_2[MAXARG];
            char argv[MAXARG][25];
            int argv_i = -1;
            for(int i = 0;i < strlen(command1);i ++){
                if(command1[i] == '<'){
                    input_flag = 1;
                    input_pos = i;
                }
            }
            if(input_flag == 1){
                char temp[512];
                int temp_i = 0;
                char input_fileName[100];
                for(int i = 0;i < input_pos;i ++){
                    if(command1[i] == ' '){
                        temp[temp_i] = 0;
                        argv_i ++;
                        memmove(argv[argv_i],temp,strlen(temp));
                        temp_i = 0;
                    }
                    else{
                        temp[temp_i ++] = command1[i];
                    }
                }
                int t = 0;
                for(int i = input_pos + 1;i < strlen(command1);i ++){
                    if(command1[i] == ' ')  continue;
                    input_fileName[t ++] = command1[i]; 
                }
                input_fileName[t] = 0;
                t = 0;
                for(int i = 0;i <= argv_i;i ++){
                    argv_2[t ++] = argv[i];
                }
                argv_2[t] = NULL;
                t = 0;
                while(argv_2[t] != NULL){
                    t ++;
                }
                close(0);
                if(open(input_fileName,O_RDONLY) < 0){
                    fprintf(2, "open %s failed\n", input_fileName);
                    exit(-1);
                }
                close(1);
                dup(p[1]);
                close(p[0]);
                close(p[1]);
                exec(argv_2[0],argv_2);
            }
            else{
                char temp[512];
                int temp_i = 0;
                for(int i = 0;i < strlen(command1);i ++){
                    if(command1[i] == ' '){
                        // printf("get in\n");
                        temp[temp_i] = 0;
                        argv_i ++;
                        memmove(argv[argv_i],temp,strlen(temp));
                        temp_i = 0;
                    }
                    else{
                        temp[temp_i ++] = command1[i];
                    }
                }
                temp[temp_i] = 0;
                argv_i ++;
                memmove(argv[argv_i],temp,strlen(temp));
                temp_i = 0;
                for(int i = 0;i <= argv_i;i ++){
                    // printf("argv=%s/endd\n",argv[i]);
                    argv_2[i] = argv[i];
                }
                close(1);
                dup(p[1]);
                close(p[0]);
                close(p[1]);
                exec(argv[0],argv_2);
            }
            exit(0);
        }
        if(fork1() == 0){
            int output_flag = 0;
            int output_pos = -1;
            char * argv_2[MAXARG];
            char argv[MAXARG][25];
            int argv_i = -1;
            for(int i = 0;i < strlen(command2);i ++){
                if(command2[i] == '>'){
                    output_flag = 1;
                    output_pos = i;
                }
            }
            if(output_flag == 1){
                char temp[512];
                int temp_i = 0;
                char output_fileName[100];
                for(int i = 0;i < output_pos;i ++){
                    if(command2[i] == ' '){
                        temp[temp_i] = 0;
                        argv_i ++;
                        memmove(argv[argv_i],temp,strlen(temp));
                        temp_i = 0;
                    }
                    else{
                        temp[temp_i ++] = command2[i];
                    }
                }
                int t = 0;
                for(int i = output_pos + 1;i < strlen(command2);i ++){
                    if(command2[i] == ' ')  continue;
                        output_fileName[t ++] = command2[i]; 
                }
                output_fileName[t] = 0;
                for(int i = 0;i <= argv_i;i ++){
                    argv_2[i] = argv[i];
                    // printf("2:argv_2 = %s\n",argv_2[i]);
                }
                // printf("OfileName=%s/endd\n",output_fileName);
                close(1);
                if(open(output_fileName,O_WRONLY | O_CREATE) < 0){
                    fprintf(2, "open %s failed\n", output_fileName);
                    exit(-1);
                }else{
                    // fprintf(2,"open %s ok\n",output_fileName);
                }
                
            }
            else{
                char temp[512];
                int temp_i = 0;
                for(int i = 0;i < strlen(command2);i ++){
                    if(command2[i] == ' '){
                        // printf("get in\n");
                        temp[temp_i] = 0;
                        argv_i ++;
                        memmove(argv[argv_i],temp,strlen(temp));
                        temp_i = 0;
                    }
                    else{
                        temp[temp_i ++] = command2[i];
                    }
                }
                temp[temp_i] = 0;
                argv_i ++;
                memmove(argv[argv_i],temp,strlen(temp));
                temp_i = 0;
                for(int i = 0;i <= argv_i;i ++){
                    argv_2[i] = argv[i];
                }
            }
            close(0);
            dup(p[0]);
            close(p[0]);
            close(p[1]);
            exec(argv[0],argv_2);
            exit(0);
        }
        close(p[0]);
        close(p[1]);
        wait(0);
        wait(0);
        exit(0);
    }
    else{
        //中间没有管道
        // printf("pipe_flag = %d\n",pipe_flag);
        // printf("command1 = %s//end\n",command1);
        int input_flag = 0;
        int output_flag = 0;
        int input_pos = -1;
        int output_pos = -1;
        char * argv_2[MAXARG];
        char argv[MAXARG][25];
        int argv_i = -1;
        for(int i = 0;i < strlen(command1);i ++){
            if(command1[i] == '<'){
                input_flag = 1;
                input_pos = i;
            }
            else if(command1[i] == '>'){
                output_flag = 1;
                output_pos = i;
            }
        }
        if((input_flag == 1) && (output_flag == 1)){
            //有输入也有输出
            char temp[512];
            int temp_i = 0;
            char input_fileName[100];
            char output_fileName[100];
            for(int i = 0;i < input_pos;i ++){
                if(command1[i] == ' '){
                    temp[temp_i] = 0;
                    argv_i ++;
                    memmove(argv[argv_i],temp,strlen(temp));
                    temp_i = 0;

                }
                else{
                    temp[temp_i ++] = command1[i];
                }
            }
            // //得到了command
            // argv_i = getrealcommand(argv,command1,input_pos);
            int t = 0;
            for(int i = input_pos + 1;i < output_pos;i ++){
                if(command1[i] == ' ')  continue;
                input_fileName[t ++] = command1[i]; 
            }
            input_fileName[t] = 0;
            t = 0;
            for(int i = output_pos + 1;i < strlen(command1);i ++){
                if(command1[i] == ' ')  continue;
                output_fileName[t ++] = command1[i];
            }
            output_fileName[t] = 0;
            for(int i = 0;i <= argv_i;i ++){
                argv_2[i] = argv[i];
                // printf("argv=%s/endd\n",argv[i]);
            }
            // printf("IfileName=%s/endd\n",input_fileName);
            // printf("OutfileName=%s/endd\n",output_fileName);
            close(0);
            if(open(input_fileName,O_RDONLY) < 0){
                fprintf(2, "10open %s failed\n", input_fileName);
                exit(-1);
            }
            close(1);
            if(open(output_fileName,O_CREATE | O_WRONLY) < 0){
                fprintf(2, "11open %s failed\n", output_fileName);
                exit(-1);
            }
            if(fork() == 0){
                exec(argv[0],argv_2);
            }
            else{
                wait(0);
            }
        }
        else if ((input_flag == 1)&&(output_flag == 0)){
            char temp[512];
            int temp_i = 0;
            char input_fileName[100];
            for(int i = 0;i < input_pos;i ++){
                if(command1[i] == ' '){
                    temp[temp_i] = 0;
                    argv_i ++;
                    memmove(argv[argv_i],temp,strlen(temp));
                    temp_i = 0;

                }
                else{
                    temp[temp_i ++] = command1[i];
                }
            }
            int t = 0;
            for(int i = input_pos + 1;i < strlen(command1);i ++){
                if(command1[i] == ' ')  continue;
                input_fileName[t ++] = command1[i]; 
            }
            input_fileName[t] = 0;
            for(int i = 0;i <= argv_i;i ++){
                // printf("argv=%s/endd\n",argv[i]);
                argv_2[i] = argv[i];
            }
            // printf("IfileName=%s/endd\n",input_fileName);
            close(0);
            if(open(input_fileName,O_RDONLY) < 0){
                fprintf(2, "open %s failed\n", input_fileName);
                exit(-1);
            }
            if(fork() == 0){
                exec(argv[0],argv_2);
            }
            else{
                wait(0);
            }
        }
        else if((input_flag == 0) && (output_flag == 1)){
            char temp[512];
            int temp_i = 0;
            char output_fileName[100];
            for(int i = 0;i < output_pos;i ++){
                if(command1[i] == ' '){
                    temp[temp_i] = 0;
                    argv_i ++;
                    memmove(argv[argv_i],temp,strlen(temp));
                    temp_i = 0;

                }
                else{
                    temp[temp_i ++] = command1[i];
                }
            }
            int t = 0;
            for(int i = output_pos + 1;i < strlen(command1);i ++){
                if(command1[i] == ' ')  continue;
                output_fileName[t ++] = command1[i]; 
            }
            output_fileName[t] = 0;
            for(int i = 0;i <= argv_i;i ++){
                // printf("argv=%s/endd\n",argv[i]);
                argv_2[i] = argv[i];
            }
            // printf("OfileName=%s/endd\n",output_fileName);
            close(1);
            if(open(output_fileName,O_WRONLY | O_CREATE) < 0 ){
                fprintf(2, "open %s failed\n",output_fileName);
                exit(-1);
            }
            if(fork() == 0){
                exec(argv[0],argv_2);
            }
            else{
                wait(0);
            }
        }
        else{
            char temp[512];
            int temp_i = 0;
            for(int i = 0;i < strlen(command1);i ++){
                // printf("1:%c\n",command1[i]);
                if(command1[i] == ' '){
                    // printf("get in\n");
                    temp[temp_i] = 0;
                    argv_i ++;
                    memmove(argv[argv_i],temp,strlen(temp));
                    temp_i = 0;

                }
                else{
                    temp[temp_i ++] = command1[i];
                }
            }
            temp[temp_i] = 0;
            argv_i ++;
            memmove(argv[argv_i],temp,strlen(temp));
            temp_i = 0;
            for(int i = 0;i <= argv_i;i ++){
                // printf("argv=%s/endd\n",argv[i]);
                argv_2[i] = argv[i];
            }
            if(fork() == 0){
                exec(argv[0],argv_2);
            }
            else{
                wait(0);
            }
        }
        exit(0);
    }
    return;

}
int main(void){
    static char buf[100];
    int fd;

    // Ensure that three file descriptors are open.
    while((fd = open("console", O_RDWR)) >= 0){
        if(fd >= 3){  
        close(fd);
        break;
        }
    }
    // Read and run input commands.
    while(getcmd(buf, sizeof(buf)) >= 0){
        if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
        // Chdir must be called by the parent, not the child.
        buf[strlen(buf)-1] = 0;  // chop \n
        if(chdir(buf+3) < 0)
            fprintf(2, "cannot cd %s\n", buf+3);
        continue;
        }
        if(fork1() == 0){
            // printf("get : %s",buf);
            parsecmd(buf); 
            exit(0);
        }
        
        wait(0);
    }
    exit(0);
}