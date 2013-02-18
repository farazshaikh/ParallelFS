#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 

int main(int argc,char **argv) {
  char *filename;
  int   fd;
  unsigned int size;
  char  pattern;
  filename = argv[1];
  pattern  = *argv[2];
  size     = atoi(argv[3]); 

  fd = open(filename,O_CREAT | O_WRONLY);
  while(size--) 
    write(fd,&pattern,1);

  
}
