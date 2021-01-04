#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <time.h>
#include "jpeglib.h"
#include "jerror.h"

#ifndef u_char
#define u_char unsigned char
#endif

char *fbp = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
int xres = 0;
int yres = 0;
int bits_per_pixel = 0;

/******************************************************************************
 *
 ******************************************************************************/
void jpeg_error_exit (j_common_ptr cinfo)
{
	cinfo->err->output_message (cinfo);
	exit (EXIT_FAILURE);
}


/*This returns an array for a 24 bit image.*/
u_char *decode_jpeg (char *filename, short *widthPtr, short *heightPtr)
{
	register JSAMPARRAY lineBuf;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr err_mgr;
	int bytesPerPix;
	FILE *inFile;
	u_char *retBuf;

	inFile = fopen (filename, "rb");
	if (NULL == inFile)
	{
		printf ("Open file error %s\n",filename);
		return NULL;
	}

	cinfo.err = jpeg_std_error (&err_mgr);
	err_mgr.error_exit = jpeg_error_exit;

	jpeg_create_decompress (&cinfo);
	jpeg_stdio_src (&cinfo, inFile);
	jpeg_read_header (&cinfo, 1);
	cinfo.do_fancy_upsampling = 0;
	cinfo.do_block_smoothing = 0;
	jpeg_start_decompress (&cinfo);

	*widthPtr = cinfo.output_width;
	*heightPtr = cinfo.output_height;
	bytesPerPix = cinfo.output_components;

	lineBuf = cinfo.mem->alloc_sarray ((j_common_ptr) &cinfo, JPOOL_IMAGE, (*widthPtr * bytesPerPix), 1);
	retBuf = (u_char *) malloc (3 * (*widthPtr * *heightPtr));

	if (NULL == retBuf)
	{
		perror (NULL);
		return NULL;
	}

	if (3 == bytesPerPix)
	{
		int x;
		int y;
		int i;

		for (y = 0; y < cinfo.output_height; ++y)
		{
			jpeg_read_scanlines (&cinfo, lineBuf, 1);
			memcpy ((retBuf + y * *widthPtr * 3),lineBuf[0],3 * *widthPtr);
		}
	}
	else if (1 == bytesPerPix)
	{
		unsigned int col;
		int lineOffset = (*widthPtr * 3);
		int lineBufIndex;
		int x ;
		int y;

		for (y = 0; y < cinfo.output_height; ++y)
		{
			jpeg_read_scanlines (&cinfo, lineBuf, 1);

			lineBufIndex = 0;
			for (x = 0; x < lineOffset; ++x)
			{
				col = lineBuf[0][lineBufIndex];

				retBuf[(lineOffset * y) + x] = col;
				++x;
				retBuf[(lineOffset * y) + x] = col;
				++x;
				retBuf[(lineOffset * y) + x] = col;
				lineBufIndex;
			}
		}
	}
	else
	{
		fprintf (stderr, "Error: the number of color channels is %d.  This program only handles 1 or 3\n", bytesPerPix);
		return NULL;
	}
	jpeg_finish_decompress (&cinfo);
	jpeg_destroy_decompress (&cinfo);
	fclose (inFile);

	return retBuf;
}

int output_jpg(int start_x,int start_y,char *filename)
{
	int i, j;
	short int width = 0, height = 0;
	unsigned char *jpeg = NULL;
	long location = 0, pos = 0;

	jpeg = decode_jpeg(filename, &width, &height);
	if (jpeg == NULL)
	{
		return -1;
	}

	for (i=0; i<height; i++)
	{
		for (j=0; j<width; j++)
		{
			//location = ( (i + start_y) * xres * bits_per_pixel / 8) + ( (start_x + j) * bits_per_pixel / 8);
			location = ( (start_x + j) * (vinfo.bits_per_pixel / 8)) + ( ( i + start_y)  *  finfo.line_length);
			pos = i * (width * 3) + (j * 3);
			*(fbp + location + 0) = *(jpeg + pos + 2);
			*(fbp + location + 1) = *(jpeg + pos + 1);
			*(fbp + location + 2) = *(jpeg + pos + 0);
			*(fbp + location + 3) = 0x00;
		}
	}

	free(jpeg);
	return 0;
}

int showjpeg(int start_x,int start_y,char *filename)
{
	int fbfd = 0;
	int fbfd1 = 0;
	long int screensize = 0;

	// Open the file for reading and writing
	fbfd = open("/dev/fb0", O_RDWR);
	if (!fbfd)
	{
		printf("Error: cannot open framebuffer device.\n");
		exit(1);
	}

	// Get fixed screen information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo))
	{
		printf("Error reading fixed information.\n");
		exit(2);
	}

	// Get variable screen information
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo))
	{
		printf("Error reading variable information.\n");
		exit(3);
	}

	//DEBUGMSG("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );
	xres = vinfo.xres_virtual;
	yres = vinfo.yres_virtual;
	bits_per_pixel = vinfo.bits_per_pixel;

	// Figure out the size of the screen in bytes
	screensize = vinfo.xres_virtual * vinfo.yres_virtual * vinfo.bits_per_pixel / 8;

	// Map the device to memory
	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,fbfd, 0);
	if ((int)fbp == -1)
	{
		printf("Error: failed to map framebuffer device to memory.\n");
		exit(4);
	}

	output_jpg(start_x,start_y,filename);

	munmap(fbp, screensize);
	close(fbfd);

	//DK 20160308
	// Open the file for reading and writing for fb1
	screensize = 0;
	fbfd1 = 0;
	fbfd1 = open("/dev/fb1", O_RDWR);
	if (!fbfd1)
	{
		printf("Error: cannot open framebuffer device.\n");
		exit(1);
	}
	// Get fixed screen information
	if (ioctl(fbfd1, FBIOGET_FSCREENINFO, &finfo))
	{
		//printf("Error reading fixed information.\n");
		//exit(2);
		return 1;
	}
	
	// Get variable screen information
	if (ioctl(fbfd1, FBIOGET_VSCREENINFO, &vinfo))
	{
		printf("Error reading variable information.\n");
		exit(3);
	}
	
	//DEBUGMSG("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );
	xres = vinfo.xres_virtual;
	yres = vinfo.yres_virtual;
	bits_per_pixel = vinfo.bits_per_pixel;
	
	// Figure out the size of the screen in bytes
	screensize = vinfo.xres_virtual * vinfo.yres_virtual * vinfo.bits_per_pixel / 8;
	
	// Map the device to memory
	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,fbfd1, 0);
	if ((int)fbp == -1)
	{
		printf("Error: failed to map framebuffer device to memory.\n");
		exit(4);
	}
	
	output_jpg(start_x,start_y,filename);
	
	munmap(fbp, screensize);
	close(fbfd1);

	return 0;
}

