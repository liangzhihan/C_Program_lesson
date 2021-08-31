#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/mman.h>

typedef struct {
    unsigned char type[2];
    unsigned int size;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int offset;
}__attribute__((packed))bmp_file_header;

typedef struct {
    unsigned int size;
    int width;
    int height;
    unsigned short planes;
    unsigned short bpp;
    unsigned int compression;
    unsigned int image_size;
    int x_pels_per_meter;
    int y_pels_per_meter;
    unsigned int clr_used;
    unsigned int clr_omportant;
}__attribute__((packed))bmp_info_header;

static int width;
static int height;
static unsigned short *screen_base = NULL;
static unsigned long line_length;


static int show_bmp_image(const char *path)
{
    bmp_file_header file_h;
    bmp_info_header info_h;
    unsigned short *line_buf = NULL;
    unsigned long line_bytes;
    unsigned int min_h, min_bytes;
    int fd=-1;
    int j;

    if(0>(fd=open(path, O_RDONLY)))
    {
        perror("open error");
        return -1;
    }

    if(sizeof(bmp_file_header) != read(fd, &file_h, sizeof(bmp_file_header)))
    {
        perror("read error");
        close(fd);
        return -1;
    }

    if(0 != memcmp(file_h.type, "BM", 2))
    {
        fprintf(stderr,"it's not a BMP file\n");
        close(fd);
        return -1;
    }

    if(sizeof(bmp_info_header) != read(fd, &info_h, sizeof(bmp_info_header)))
    {
        perror("read error");
        close(fd);
        return -1;
    }

    printf("文件大小:%d\n"
            "位图数据的偏移量:%d\n"
            "位图信息头大小：%d\n"
            "图像分辨率：%d*%d\n"
            "图像深度：%d\n", file_h.size, file_h.offset, info_h.size, info_h.width, info_h.height, info_h.bpp);

    if(-1 == lseek(fd, file_h.offset, SEEK_SET))
    {
        perror("lseek error");
        close(fd);
        return -1;
    }

    line_bytes = info_h.width * info_h.bpp/8;
    line_buf = malloc(line_bytes);
    if(NULL == line_buf){
        fprintf(stderr, "malloc error\n");
        close(fd);
        return -1;
    }

    if(line_length > line_bytes)
    {
        min_bytes = line_bytes;
    }
    else
    {
        min_bytes = line_length;
    }

    if(0<info_h.height)
    {
        if(info_h.height > height)
        {
            min_h = height;
            lseek(fd,(info_h.height - height)*line_bytes, SEEK_CUR);
            screen_base += width*(height -1);
        }
        else
        {
            min_h = info_h.height;
            screen_base += width*(info_h.height - 1);
        }

        for(j=min_h;j>0;screen_base -= width, j--)
        {
            read(fd, line_buf, line_bytes);
            memcpy(screen_base, line_buf, min_bytes);
        }
    }
    else
    {
        int temp = 0-info_h.height;
        if(temp > height)
        {
            min_h = height;
        }
        else
        {
            min_h = temp;
        }

        for(j=0;j<min_h;j++,screen_base += width)
        {
            read(fd,line_buf,line_bytes);
            memcpy(screen_base,line_buf,min_bytes);
        }
    }

    close(fd);
    free(line_buf);
    return 0;
}

int main(int argc, char *argv[])
{
    struct fb_fix_screeninfo fb_fix;
    struct fb_var_screeninfo fb_var;
    unsigned int screen_size;
    int fd;

    if(2 != argc)
    {
        fprintf(stderr, "usage: %s <bmp_file>\n", argv[0]);
        exit(-1);
    }

    if(0>(fd=open("/dev/fb0", O_RDWR)))
    {
        perror("open error");
        exit(EXIT_FAILURE);
    }

    ioctl(fd, FBIOGET_VSCREENINFO, &fb_var);
    ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix);

    screen_size = fb_fix.line_length * fb_var.yres;
    line_length = fb_fix.line_length;
    width = fb_var.xres;
    height = fb_var.yres;


    screen_base = mmap(NULL, screen_size, PROT_WRITE, MAP_SHARED, fd, 0);
    if(MAP_FAILED == (void*)screen_base)
    {
        perror("mmap error");
        close(fd);
        exit(EXIT_FAILURE);
    }

    memset(screen_base, 0xFF, screen_size);
    show_bmp_image(argv[1]);

    munmap(screen_base, screen_size);
    close(fd);
    exit(EXIT_SUCCESS);
}

