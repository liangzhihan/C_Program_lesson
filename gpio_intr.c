#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

static char gpio_path[100];

static int gpio_config(const char *attr, const char *val)
{
    char file_path[100];
    int len;
    int fd;




    sprintf(file_path, "%s/%s",gpio_path,attr);
    if(0>(fd=open(file_path,O_WRONLY)))
    {
        perror("open error");
        return fd;
    }

    len = strlen(val);
    if(len != write(fd,val,len))
    {
        perror("write error");
        return -1;
    }

    close(fd);
    return 0;
}

void main(int argc, char *argv[])
{
    struct pollfd pfd;
    char file_path[100];
    int ret;
    char val;

    if(2!= argc)
    {
        fprintf(stderr, "usage: %s<gpio>\n",argv[0]);
        exit(-1);
    }

    sprintf(gpio_path, "/sys/class/gpio/gpio%s", argv[1]);

    if(access(gpio_path, F_OK))
    {
        int len;
        int fd;

        if(0>(fd=open("/sys/class/gpio/export", O_WRONLY)))
        {
            perror("open error");
            exit(-1);
        }

        len = strlen(argv[1]);
        if(len != write(fd,argv[1],len))
        {
            perror("write error");
            exit(-1);
        }

        close(fd);
    }

    if(gpio_config("direction","in"))
    {
        exit(-1);
    }

    if(gpio_config("active_low","0"))
    {
        exit(-1);
    }

    if(gpio_config("edge","both"))
    {
        exit(-1);
    }

    sprintf(file_path, "%s/%s",gpio_path, "value");

    if(0>(pfd.fd=open(file_path, O_RDONLY)))
    {
        perror("open error");
        exit(-1);
    }

    pfd.events = POLLPRI;

    read(pfd.fd, &val, 1);

    for(;;)
    {
        ret = poll(&pfd,1,-1);
        if(0>ret)
        {
            perror("poll error");
            exit(-1);
        }
        else if(0==ret)
        {
            fprintf(stderr, "poll timeout.\n");
            continue;
        }

        printf("ret = %d\n", ret);

        if(pfd.revents & POLLPRI)
        {
            if(0>lseek(pfd.fd,0,SEEK_SET))
            {
                perror("lseek error");
                exit(-1);
            }

            if(0>read(pfd.fd, &val, 1))
            {
                perror("read error");
                exit(-1);
            }

            printf("GPIO 中断触发<value=%c>\n",val);
        }
    }

    exit(0);

}