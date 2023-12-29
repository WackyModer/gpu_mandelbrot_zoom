/*
 Not even I understand this file.

 I mean, I made it, but I have no damn clue.
 */


#include <stdio.h>

#include "./shared.h"

const int BYTES_PER_PIXEL = 3; /// red, green, & blue
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;

void write_start(char** imageFileName, FILE* imageFile, int w, int h);
void write_img(unsigned char* image, FILE* imageFile);
unsigned char* createBitmapFileHeader(int h, int stride);
unsigned char* createBitmapInfoHeader(int h, int w);

void disk_thread() {
    while(side != 2) {
        if(side == 1) {
            printf("Starting write\n");
            char img_file_nm[100] = "";

            sprintf(img_file_nm,"./frames/mand_%d.bmp", framenum); // Might wanna have it write into a folder...

            FILE* imageFile = fopen((char*)img_file_nm, "wb");

            if(imageFile == NULL){
                printf("err\n");
            }

            write_start((char**)img_file_nm, imageFile, width, height);

            write_img((unsigned char*) img, imageFile);

            fclose(imageFile);
            printf("Finished write...\n");
            if(side != 2){
                side = 0;
            }
        }
    }
}

void write_start(char** imageFileName, FILE* imageFile, int w, int h){
    int widthInBytes = w * BYTES_PER_PIXEL;

    unsigned char padding[3] = {0, 0, 0};
    int paddingSize = (4 - (widthInBytes) % 4) % 4;

    int stride = (widthInBytes) + paddingSize;

    unsigned char* fileHeader = createBitmapFileHeader(h, stride);
    fwrite(fileHeader, 1, FILE_HEADER_SIZE, imageFile);

    unsigned char* infoHeader = createBitmapInfoHeader(h, w);
    fwrite(infoHeader, 1, INFO_HEADER_SIZE, imageFile);
}

void write_img(unsigned char* image, FILE* imageFile) {

    int widthInBytes = width * BYTES_PER_PIXEL;

    unsigned char padding[3] = {0, 0, 0};
    int paddingSize = (4 - (widthInBytes) % 4) % 4;

    int stride = (widthInBytes) + paddingSize;


    for(int x = 0; x<width;x++){
        fwrite(&img[x], BYTES_PER_PIXEL, width, imageFile);
    }

    fwrite(padding, 1, paddingSize, imageFile);

}


unsigned char* createBitmapFileHeader (int h, int stride)
{
    int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * h);

    static unsigned char fileHeader[] = {
            0,0,     /// signature
            0,0,0,0, /// image file size in bytes
            0,0,0,0, /// reserved
            0,0,0,0, /// start of pixel array
    };

    fileHeader[ 0] = (unsigned char)('B');
    fileHeader[ 1] = (unsigned char)('M');
    fileHeader[ 2] = (unsigned char)(fileSize      );
    fileHeader[ 3] = (unsigned char)(fileSize >>  8);
    fileHeader[ 4] = (unsigned char)(fileSize >> 16);
    fileHeader[ 5] = (unsigned char)(fileSize >> 24);
    fileHeader[10] = (unsigned char)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

    return fileHeader;
}

unsigned char* createBitmapInfoHeader (int h, int w)
{
    static unsigned char infoHeader[] = {
            0,0,0,0, /// header size
            0,0,0,0, /// image width
            0,0,0,0, /// image height
            0,0,     /// number of color planes
            0,0,     /// bits per pixel
            0,0,0,0, /// compression
            0,0,0,0, /// image size
            0,0,0,0, /// horizontal resolution
            0,0,0,0, /// vertical resolution
            0,0,0,0, /// colors in color table
            0,0,0,0, /// important color count
    };

    infoHeader[ 0] = (unsigned char)(INFO_HEADER_SIZE);
    infoHeader[ 4] = (unsigned char)(w      );
    infoHeader[ 5] = (unsigned char)(w >>  8);
    infoHeader[ 6] = (unsigned char)(w >> 16);
    infoHeader[ 7] = (unsigned char)(w >> 24);
    infoHeader[ 8] = (unsigned char)(h      );
    infoHeader[ 9] = (unsigned char)(h >>  8);
    infoHeader[10] = (unsigned char)(h >> 16);
    infoHeader[11] = (unsigned char)(h >> 24);
    infoHeader[12] = (unsigned char)(1);
    infoHeader[14] = (unsigned char)(BYTES_PER_PIXEL*8);

    return infoHeader;
}