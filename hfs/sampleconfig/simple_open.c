#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

int dup(int oldfd);
int dup2(int oldfd, int newfd);

int
main(int argc,char **argv)
{
    char *filename;
    int   fd,fd2;
    unsigned int size;
    char  pattern;

    filename = argv[1];
    pattern  = *argv[2];
    size     = atoi(argv[3]);

    fd  = open(filename,O_CREAT | O_WRONLY);
    fd2= open(filename,O_CREAT | O_WRONLY);
    close(fd);
    close(fd2);
}
