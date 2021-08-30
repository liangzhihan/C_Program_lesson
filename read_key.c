#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

int main(int argc, char *argv[])
{
    struct input_event in_ev = {0};
    int fd = -1;
    int value = -1;

    if(2 != argc)
    {
        fprintf(stderr, "usage: %s<input-dev>\n", argv[0]);
        exit(-1);
    }
    if(0>(fd=open(argv[1],O_RDONLY)))
    {
        perror("open error");
        exit(-1);
    }

    for(;;)
    {
        if(sizeof(struct input_event)!=read(fd,&in_ev,sizeof(struct input_event)))
        {
            perror("read error");
            exit(-1);
        }

        if(EV_KEY == in_ev.type)
        {
            switch(in_ev.value)
            {
                case 0:
                    printf("code<%d>:松开\n",in_ev.code);
                    break;
                case 1:
                    printf("code<%d>:按下\n",in_ev.code);
                    break;
                case 2:
                    printf("code<%d>:长按\n",in_ev.code);
                    break;

            }
        }



    }
}