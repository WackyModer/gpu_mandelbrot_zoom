#include <stdio.h>
#include <math.h>
#include <iostream>

const int BYTES_PER_PIXEL = 3; /// red, green, & blue
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;

void write_start(char** imageFileName, FILE* imageFile, int width, int height);
void writecur_img(unsigned char* image, FILE* imageFile, int y);
unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);

const int width = 4000;
int height = 4000;

int maxiter = 500;

float zoom = 0.5;


int mandelcalcd(double creal, double cimag) {
    double zreal = 0.0;
    double zimag = 0.0;
    
    double xreal = 1.0;
    double ximag = 1.0;
    int iter = 0;

    while ((zreal*zreal+zimag*zimag) < 4.0 && iter < maxiter) {
        xreal = zreal;
        ximag = zimag;

        zreal = (xreal*xreal - ximag*ximag) + creal;
        zimag = (xreal*ximag + ximag*xreal) + cimag;

        iter++;

        if (zreal == xreal && zimag == ximag) {
            return maxiter;
        }
        //return maxiter;
    }
    return iter;
}

int mandelcalcf(float creal, float cimag) {
    float zreal = 0.0;
    float zimag = 0.0;

    float xreal = 1.0;
    float ximag = 1.0;
    int iter = 0;

    while ((zreal*zreal+zimag*zimag) < 4.0 && iter < maxiter) {
        xreal = zreal;
        ximag = zimag;

        zreal = (xreal*xreal - ximag*ximag) + creal;
        zimag = (xreal*ximag + ximag*xreal) + cimag;

        iter++;

        if (zreal == xreal && zimag == ximag) {
            return maxiter;
        }
        //return maxiter;
    }
    return iter;
}

int main (int argc, char* argv[])
{
    bool double_or_float = true; // double is true.

    char** imageFileName = (char**)"The_program_broke";
    if(argc == 3 || argc == 4 || argc == 5){
        if(argc == 4){
            imageFileName = (char**) argv[3];
        }else{
            imageFileName = (char**)"out.bmp";
        }
    }else {
        printf("\nIncorrect usage! Expected xxx.exe <square res> <max iter> [Optional file name including .bmp] [-D for double, -F for float. Optional. Floats are faster. You need to have image name for this to work]\n");
        return 0;
    }

    if(argc == 5){
    if(argv[4][1] == 'D'){
            double_or_float = true;
            printf("Using doubles!");
        }else if(argv[4][1] == 'F'){
            printf("Using floats!");
            double_or_float = false;
        }else{
            printf("The fourth argument needs to be either -D or -F.");
            exit(0);
        }
    }

    height = atoi(argv[1]);
    width = atoi(argv[1]);
    maxiter = atoi(argv[2]);

    if(height < 1 || width < 1 || maxiter < 1){
        printf("One of the aguments is a negative number!\n");
    }
    int tmp = (int)(((width*height*3)/1000000));

    printf("\nEstimated file size is %d mb.\n", tmp);
    printf("Running at %dx%d px and %d max iterations with the output file name \"%s\".\n", width, height, maxiter, (char*)imageFileName);
    printf("Continue? [Y/N]: ");
    char yn = '.';
    scanf("%c", &yn);
    if(yn == 'y' || yn == 'Y'){

    }else{
        printf("Exiting...\n");
        return 0;
    }

    printf("\n\n");


    
    FILE* imageFile = fopen((char*)imageFileName, "wb");

    write_start(imageFileName, imageFile, width, height);
    unsigned char image[width][BYTES_PER_PIXEL];

    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){


            int iterations;

            if(double_or_float) {
                double real = (x - width / 2.0) * zoom / width*6 - 0.75;
                double imag = (y - height / 2.0) * zoom / height*6;
                iterations = mandelcalcd(real, imag);
            }else{
                float real = (x - width / 2.0) * zoom / width*6 - 0.75;
                float imag = (y - height / 2.0) * zoom / height*6;
                iterations = mandelcalcf(real, imag);
            }

            if(iterations != maxiter){
                image[x][2] = (unsigned char) (int) ((sin(iterations * 0.1) * 0.5 + 0.5)*255); ///red
                image[x][1] = (unsigned char) (int) ((cos(iterations * 0.1) * 0.5 + 0.5)*255); ///green
                image[x][0] = (unsigned char) (255); ///blue
            }else{
                image[x][2] = (unsigned char) (0); ///red
                image[x][1] = (unsigned char) (0); ///green
                image[x][0] = (unsigned char) (0); ///blue
            }
        }
        try{
        writecur_img((unsigned char*) image, imageFile, y);
        }catch(...){
            printf("Error occured writing file! Maybe you have the size *WAY* too high?\n");
        }
        if(y%10==0){
            printf("Line (Y axis) %d/%d written!\n", y, height);
        }
    }
    fclose(imageFile);
    //generateBitmapImage((unsigned char*) image, height, width, imageFileName);
    printf("Image generated!!");
}

void write_start(char** imageFileName, FILE* imageFile, int width, int height){
    int widthInBytes = width * BYTES_PER_PIXEL;

    unsigned char padding[3] = {0, 0, 0};
    int paddingSize = (4 - (widthInBytes) % 4) % 4;

    int stride = (widthInBytes) + paddingSize;

    unsigned char* fileHeader = createBitmapFileHeader(height, stride);
    fwrite(fileHeader, 1, FILE_HEADER_SIZE, imageFile);

    unsigned char* infoHeader = createBitmapInfoHeader(height, width);
    fwrite(infoHeader, 1, INFO_HEADER_SIZE, imageFile);
}

void writecur_img(unsigned char* image, FILE* imageFile, int y) {

    int widthInBytes = width * BYTES_PER_PIXEL;

    unsigned char padding[3] = {0, 0, 0};
    int paddingSize = (4 - (widthInBytes) % 4) % 4;

    int stride = (widthInBytes) + paddingSize;

    fwrite(image, BYTES_PER_PIXEL, width, imageFile);
    fwrite(padding, 1, paddingSize, imageFile);
}


unsigned char* createBitmapFileHeader (int height, int stride)
{
    int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

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

unsigned char* createBitmapInfoHeader (int height, int width)
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
    infoHeader[ 4] = (unsigned char)(width      );
    infoHeader[ 5] = (unsigned char)(width >>  8);
    infoHeader[ 6] = (unsigned char)(width >> 16);
    infoHeader[ 7] = (unsigned char)(width >> 24);
    infoHeader[ 8] = (unsigned char)(height      );
    infoHeader[ 9] = (unsigned char)(height >>  8);
    infoHeader[10] = (unsigned char)(height >> 16);
    infoHeader[11] = (unsigned char)(height >> 24);
    infoHeader[12] = (unsigned char)(1);
    infoHeader[14] = (unsigned char)(BYTES_PER_PIXEL*8);

    return infoHeader;
}