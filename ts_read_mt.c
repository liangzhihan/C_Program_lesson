#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <tslib.h>

int main(int argc, char *argv[])
{
    struct tsdev *ts = NULL;
    struct ts_sample_mt *mt_ptr = NULL;
    struct input_absinfo slot;
    int max_slots;
    unsigned int pressure[12] = {0};
    int i;

    ts = ts_setup(NULL, 0);
    if(NULL == ts)
    {
        fprintf(stderr, "ts_setup error");
        exit(EXIT_FAILURE);
    }

    if(0>ioctl(ts_fd(ts),EVIOCGABS(ABS_MT_SLOT), &slot))
    {
        perror("ioctl error");
        ts_close(ts);
        exit(EXIT_FAILURE);
    }

    max_slots = slot.maximum + 1 - slot.minimum;
    printf("max_slots: %d\n", max_slots);

    mt_ptr = calloc(max_slots, sizeof(struct ts_sample_mt));

    for(;;)
    {
        if(0>ts_read_mt(ts, &mt_ptr, max_slots, 1))
        {
            perror("ts_read_mt error");
            ts_close(ts);
            free(mt_ptr);
            exit(EXIT_FAILURE);
        }

        for(i=0;i<max_slots;i++)
        {

            printf("第<%d>个点的valid=%d\n", i, mt_ptr[i].valid);

            if(mt_ptr[i].valid)
            {
                if(mt_ptr[i].pressure)
                {
                    if(pressure[mt_ptr[i].slot])
                    {
                        printf("slot<%d>, 移动 (%d,%d)\n", mt_ptr[i].slot, mt_ptr[i].x, mt_ptr[i].y);
                    }
                    else
                    {
                        printf("slot<%d>, 按下 (%d,%d)\n", mt_ptr[i].slot, mt_ptr[i].x, mt_ptr[i].y);
                    }
                }
                else
                {
                    printf("slot<%d>, 松开\n", mt_ptr[i].slot);
                }

                pressure[mt_ptr[i].slot] = mt_ptr[i].pressure;

                printf("处理第<%d>个点\n", i);
            }
        }
    }

    ts_close(ts);
    free(mt_ptr);
    exit(EXIT_SUCCESS);
}