#include <stdio.h>
#include <stdlib.h>
#include <tslib.h>

int main(int argc, char *argv[])
{
    struct tsdev *ts=NULL;
    struct ts_sample samp;
    int pressure=0;

    ts = ts_setup(NULL, 0);
    if(NULL == ts)
    {
        fprintf(stderr, "ts_setup error");
        exit(EXIT_FAILURE);
    }

    for(;;)
    {
        if(0>ts_read(ts, &samp, 1))
        {
            fprintf(stderr, "ts_read error");
            ts_close(ts);
            exit(EXIT_FAILURE);
        }

        if(samp.pressure)
        {
            if(pressure)
                printf("移动(%d,%d)\n",samp.x,samp.y);
            else
            {
                printf("按下(%d,%d)\n",samp.x,samp.y);
            }
        }
        else
            printf("松开\n");

        pressure = samp.pressure;
    }

    ts_close(ts);
    exit(EXIT_SUCCESS);
}