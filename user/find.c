#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char *fmtname(char * path){
    // static char buf[DIRSIZ+1];
    char *p;
    //找到/之后的第一个单词
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
    p++;
    return p;
    // // Return blank-padded name.
    // if(strlen(p) >= DIRSIZ)
    //     return p;
    // memmove(buf, p, strlen(p));
    // memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
    // return buf;
}

void getFilePath(char * stPath,char * name){
    char buf[512],*p;
    int fd;
    struct stat st;
    struct dirent de;
    // printf("path = %s,name = %s\n",stPath,name);
    if((fd = open(stPath,0)) < 0){
        //打开当前路径的文件夹
        fprintf(2,"open %s failed\n",stPath);
        return ;
    }
    if(fstat(fd,&st) < 0){
        fprintf(2,"state %s failed!\n",stPath);
        close(fd);
        return ;
    }
    
    switch(st.type){
        case T_FILE:
            // printf("is a file\n");
            if(strcmp(fmtname(stPath),name) == 0){
                //两个文件名相等
                printf("%s\n",stPath);
            }else{
                // printf("not equal,%s,%s",fmtname(stPath),name);
            }
            break;
        case T_DIR:
            if(strlen(stPath) + 1 + DIRSIZ + 1 > sizeof buf ){
                printf("path is too long\n");
                break;
            }
            strcpy(buf,stPath);
            p = buf+strlen(buf);
            *p++ = '/';
            while(read(fd,&de,sizeof(de)) == sizeof(de)){
                
                if(de.inum == 0)
                    continue;
                
                if(strcmp(de.name,".") == 0||strcmp(de.name,"..") == 0){
                    // printf("equal name = %s\n",de.name);
                    continue;
                }
                // printf("de.name = %s\n",de.name);
                memmove(p,de.name,DIRSIZ);
                p[DIRSIZ] = 0;
                // printf("DIR_p = %s\n",buf);
                getFilePath(buf,name);
            }
            break;
    }
    close(fd);
    return ;
}

int main(int argc, char *argv[]){
    
    if(argc < 3){
        // printf("argc = %d\n",argc);
    }
    else{
        getFilePath(argv[1],argv[2]);
    }
    exit();
}
