#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

int main(int argc, char *argv[])
{
    struct input_event in_ev;
    int x,y;
    int down;
    int valid;
    int fd=-1;

    if(2 != argc)
    {
        fprintf(stderr, "usage: %s <input-dev>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(0>(fd=open(argv[1],O_RDONLY)))
    {
        perror("open error");
        exit(EXIT_FAILURE);
    }

    x = y = 0;
    down = -1;
    valid = 0;

    for(;;)
    {
        if(sizeof(struct input_event) != read(fd, &in_ev, sizeof(struct input_event)))
        {
            perror("read error");
            exit(EXIT_FAILURE);
        }

        switch(in_ev.type)
        {
            case EV_KEY:
                if(BTN_TOUCH == in_ev.code)
                {
                    down = in_ev.value;
                    valid = 1;
                }
            break;

            case EV_ABS:
                switch (in_ev.code)
                {
                    case ABS_X:
                        x= in_ev.value;
                        valid = 1;
                    break;

                    case ABS_Y:
                        y= in_ev.value;
                        valid = 1;
                    break;
                }
            break;

            case EV_SYN:
                if(SYN_REPORT == in_ev.code)
                {
                    if(valid)
                    {
                        switch (down)
                        {
                            case 1:
                                printf("按下(%d, %d)\n", x,y);
                                break;
                            case 0:
                                printf("松开\n");
                                break;
                            case -1:
                                printf("移动(%d, %d)\n", x,y);
                                break;

                        }

                        valid = 0;
                        down = -1;
                
                    }
                }
            break;

        }
    }
}