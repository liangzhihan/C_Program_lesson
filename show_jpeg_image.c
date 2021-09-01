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
#include <jpeglib.h>

typedef struct bgr888_color {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
}__attribute__((packed))bgr888_t;

static int width;
static int height;
static unsigned short *screen_base = NULL;
static unsigned long line_length;
static unsigned int bpp;

static int show_jpeg_image(const char *path)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *jpeg_file = NULL;
    bgr888_t *jpeg_line_buf = NULL;
    unsigned short *fb_line_buf = NULL;
    unsigned int min_h, min_w;
    unsigned int valid_bytes;
    int i;

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_file = fopen(path, "r");
    if(NULL == jpeg_file)
    {
        perror("fopen error");
        return -1;
    }

    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, jpeg_file);

    jpeg_read_header(&cinfo, TRUE);

    printf("JPEG 图像大小: %d*%d\n", cinfo.image_width, cinfo.image_height);

    cinfo.out_color_space = JCS_RGB;

    jpeg_start_decompress(&cinfo);

    jpeg_line_buf = malloc(cinfo.output_components * cinfo.output_width);
    fb_line_buf = malloc(line_length);

    if(cinfo.output_width > width)
    {
        min_w = width;
    }
    else
    {
        min_w = cinfo.output_width;
    }

    if(cinfo.output_height > height)
    {
        min_h = height;
    }
    else
    {
        min_h = cinfo.output_height;
    }

    valid_bytes = min_w * bpp/8;
    while(cinfo.output_scanline < min_h)
    {
        jpeg_read_scanlines(&cinfo, (unsigned char **)&jpeg_line_buf, 1);

        for(i=0;i<min_w;i++)
        {
            fb_line_buf[i] = ((jpeg_line_buf[i].red & 0xF8)<<8) | 
                        ((jpeg_line_buf[i].green & 0xFC)<<3) | 
                        ((jpeg_line_buf[i].blue & 0xF8)>>3);
        }
        memcpy(screen_base, fb_line_buf, valid_bytes);
        screen_base += width;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(jpeg_file);
    free(fb_line_buf);
    free(jpeg_line_buf);
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
        fprintf(stderr, "usage:%s<jpeg_file>\n", argv[0]);
        exit(-1);
    }

    if(0>(fd=open("/dev/fb0", O_RDWR)))
    {
        perror("open error");
        exit(EXIT_FAILURE);
    }

    ioctl(fd,FBIOGET_VSCREENINFO, &fb_var);
    ioctl(fd,FBIOGET_FSCREENINFO, &fb_fix);

    line_length = fb_fix.line_length;
    bpp = fb_var.bits_per_pixel;
    screen_size = line_length * fb_var.yres;
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
    show_jpeg_image(argv[1]);

    munmap(screen_base, screen_size);
    close(fd);
    exit(EXIT_SUCCESS);
}