#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    char *parentSend = "a";
    char *childSend = "b";
    char buf[2];
    int p[2];
    pipe(p);
 if(fork() == 0){
     if(read(p[0],buf,1) != 1){
         fprintf(2,"Can't read from parent!\n");
         exit(1);
     }

     printf("child receive: %c\n", buf[0]);
        close(p[0]);
     printf("%d: received ping\n",getpid());
      if( write(p[1],childSend,1) != 1){//从文件偏移量开始写
         fprintf(2,"Can't send to parent!\n");
         exit(1);
     }
     close(p[1]);
     exit(0);

 }else{

     if(write(p[1],parentSend,1) != 1){
          fprintf(2,"Can't send to child!\n");
         exit(1);
     }

     close(p[1]);
    
    wait((int*) 0);
     if(read(p[0],buf,1) != 1){//从文件偏移量开始读，但写入buf中的是直接覆盖
          fprintf(2,"Can't read from child!\n");
         exit(1);
     }
     close(p[0]);
      printf("parent receive: %c\n", buf[0]);
      printf("%d: received pong\n",getpid());    
    exit(0);
 }
}
