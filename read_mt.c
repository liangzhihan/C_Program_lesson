#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <linux/input.h>

struct ts_mt{
    int x;
    int y;
    int id;
    int valid;
};

struct tp_xy{
    int x;
    int y;
};

static int ts_read(const int fd, const int max_slots, struct ts_mt *mt)
{
    struct input_event in_ev;
    static int slot=0;
    static struct tp_xy xy[12] = {0};
    int i;

    memset(mt,0x00,max_slots * sizeof(struct ts_mt));
    for(i=0;i<max_slots;i++)
    {
        mt[i].id = -2;
    }

    for(;;)
    {
        if(sizeof(struct input_event) != read(fd,&in_ev,sizeof(struct input_event)))
        {
            perror("read error");
            return -1;
        }

        switch(in_ev.type)
        {
            case EV_ABS:
                switch(in_ev.code)
                {
                    case ABS_MT_SLOT:
                        slot = in_ev.value;
                    break;

                    case ABS_MT_POSITION_X:
                        xy[slot].x = in_ev.value;
                        mt[slot].valid = 1;
                    break;

                    case ABS_MT_POSITION_Y:
                        xy[slot].y = in_ev.value;
                        mt[slot].valid = 1;
                    break;

                    case ABS_MT_TRACKING_ID:
                        mt[slot].id = in_ev.value;
                        mt[slot].valid = 1;
                    break;
                }
            break;

            case EV_SYN:
                if(SYN_REPORT == in_ev.code){
                    for(i=0;i<max_slots;i++)
                    {
                        mt[i].x = xy[i].x;
                        mt[i].y = xy[i].y;
                    }
                }
            return 0;
        }
    }
}

int main(int argc, char *argv[])
{
    struct input_absinfo slot;
    struct ts_mt *mt = NULL;
    int max_slots;
    int fd;
    int i;

    if(2 != argc)
    {
        fprintf(stderr, "usage: %s <input-dev>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fd = open(argv[1], O_RDONLY);
    if(0>fd)
    {
        perror("open error");
        exit(EXIT_FAILURE);
    }

    if(0>ioctl(fd,EVIOCGABS(ABS_MT_SLOT), &slot))
    {
        perror("ioctl error");
        close(fd);
        exit(EXIT_FAILURE);
    }

    max_slots = slot.maximum + 1 - slot.minimum;
    printf("max_slots: %d\n", max_slots);

    mt = calloc(max_slots, sizeof(struct ts_mt));

    for(;;)
    {
        if(0>ts_read(fd,max_slots,mt))
        {
            break;
        }

        for(i=0;i<max_slots;i++)
        {
            if(mt[i].valid)
            {
                if(0<=mt[i].id)
                {
                    printf("slot<%d>, 按下(%d,%d)\n", i, mt[i].x, mt[i].y);
                }
                else if(-1==mt[i].id)
                {
                    printf("slot<%d>, 松开\n", i);
                }
                else{
                    printf("slot<%d>, 移动(%d,%d)\n", i, mt[i].x, mt[i].y);
                }
            }
        }
    }

    close(fd);
    free(mt);
    exit(EXIT_FAILURE);
}
