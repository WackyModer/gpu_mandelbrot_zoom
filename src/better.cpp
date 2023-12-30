#include <cstdio>
#include <glm/glm.hpp>
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <thread>
#include <iostream>

#define GLFW_DLL

// Crappy multithreaded variable management
int side = 0;

// Both width and height need to be the same and need to be a multiple of 32
int width = 256;
// Both width and height need to be the same and need to be a multiple of 32
int height = 256;


// Keep the same as ints width and height
unsigned char ***img;//img[256][256][3];


unsigned int framenum = 0;
unsigned int maxframes = 0xffffffff; // ehhhhh

int maxiter = 5000;//5000;



double x_pos = -0.743643887037158704752191506114774*10000000.0; // ok, this whole multiplication thing gives more decimal accuracy
double y_pos = 0.131825904205311970493132056385139*1000000.0; // ok, this whole multiplication thing gives more decimal accuracy

/*
    It will zoom until it hits this number, so don't set it too high.
    How do you get this and try to tween this you ask?

    console python interpreter.

zoom = 0.25 #(Or whatever your zoom level is)
i = 0
while zoom < x: #(x is maxzoom)
    zoom *= y # Zoom speed
    i += 1 # WHY DOES PYTHON NOT HAVE A ++
    print(zoom)
    print(i)
 */
float startzoom = 0.25f;
float zoomspeed = 1.02f; // How fast it zooms, heavily recommend keeping it below 1.05, it gets fast, well, fast
double maxzoom = 400000000.0;

// uhhh lemme explain
// I got nuthin
// Ok, so this is the OpenGL shader code for rending the thing
// It can be SUPPERRRRRRRRRR slow on some GPU's, because it uses...
// doubles. I might hack in two floats mashed together to make it slightly faster.
// but for the most part this is gonna be slow because it uses doubles instead of floats
// which is not supported on most consumer hardware (Built in Intel Iris GPU :sob: :sob:)
// Works just fine on my """"""server"""""" desktop that has a 1070 iirc
const char* computeShaderSource = R"(
#version 430 core
// Your compute shader code here


layout (local_size_x = 32, local_size_y = 32) in;

struct Vertex {
    vec4 pos;
};

layout(std430) buffer;

layout(binding=0) writeonly buffer Pos {
    Vertex verticies[];
};


layout(binding=1) readonly buffer Pose {
    Vertex dataa[];
};

uvec3 localInvocationID = gl_LocalInvocationID;
uvec3 globalInvocationID = gl_GlobalInvocationID;

int maxiter = int(dataa[0].pos.z);



int mandel(double creal, double cimag){
    int iter = 0;

    double zreal = 0.0;
    double zimag = 0.0;

    double xreal = 1.0;
    double ximag = 1.0;


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

void main() {

    int width = int(dataa[0].pos.x);
    int height = int(dataa[0].pos.y);

    double zoom = dataa[0].pos.w;

    double offset_x = dataa[1].pos.x/10000000.0;
    double offset_y = dataa[1].pos.y/10000000.0;

    double real = (globalInvocationID.x - width / 2.0) / zoom / width + offset_x;
    double imag = (globalInvocationID.y - height / 2.0) / zoom / height + offset_y;

    int meit = mandel(real, imag);

    int r;
    int g;
    int b;

    if(meit == maxiter){
        r = 0;
        g = 0;
        b = 0;
    }else{
        r = int((sin(meit * 0.1) * 0.5 + 0.5) * 255);
        g = int((cos(meit * 0.1) * 0.5 + 0.5) * 255);
        b = 255;
    }

    verticies[globalInvocationID.x+globalInvocationID.y*width].pos.x = r;
    verticies[globalInvocationID.x+globalInvocationID.y*width].pos.y = g;
    verticies[globalInvocationID.x+globalInvocationID.y*width].pos.z = b;


    //verticies[0].pos.x = 9;

    return;
}
)";

void process_stuff() {
    //printf("X: %f, Y: %f",x_pos,y_pos); :P

    std::vector<glm::vec4> image;
    image.resize(width * height);

    // Initialize GLFW
    glewInit();
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Now that it is initialized, we create a window
    GLFWwindow* window = glfwCreateWindow(800, 800, "Temp window (This is needed)", NULL, NULL);

    // We set the context to be our window and then initialize GLAD
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE; // I cried over this line

    glewInit();


    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &computeShaderSource, NULL);
    glCompileShader(computeShader);

    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);


    double zoooom = startzoom;

    for(int i = 0; i < maxframes; i++) {

        // To test how a zoom will look at x zoom.
        // zoooom = 400000000.0;

        printf("\n\n%f\n\n",zoooom);




        GLuint computeBufSize = sizeof(glm::vec4) * (width*height);

        GLuint buf;
        glGenBuffers(1, &buf);

        GLuint binding = 0;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, buf);

        glBufferData(GL_SHADER_STORAGE_BUFFER, computeBufSize, image.data(), GL_STATIC_DRAW);


        std::vector<glm::vec4> data = {
                { width, height, maxiter, zoooom },
                { (double)x_pos, (double)y_pos, 0, 0},
        };

        GLuint computeBufSize_two = sizeof(glm::vec4) * data.size();

        GLuint buf_two;
        glGenBuffers(1, &buf_two);

        GLuint binding_two = 1;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_two, buf_two);

        glBufferData(GL_SHADER_STORAGE_BUFFER, computeBufSize_two, data.data(), GL_STATIC_DRAW);


        glUseProgram(computeProgram);


        glDispatchCompute((int)(width/32),(int)(height/32),1);

        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        glGetNamedBufferSubData(buf, 0, computeBufSize, image.data());

        framenum = i;
        printf("Rendered frame %d\n",framenum);

        printf("copying frame to disk buf...\n");


        while(side == 1) {

        }

        for(int x = 0; x<width;x++){
            for(int y = 0; y<height;y++){
                glm::vec4 froggert = image[x*width+y];
                img[x][y][2] = (unsigned char)froggert.x;
                img[x][y][1] = (unsigned char)froggert.y;
                img[x][y][0] = (unsigned char)froggert.z;
            }
        }


        if(zoooom > maxzoom) {
            break;
        }

        zoooom *= zoomspeed;



        side = 1;
    }
    side = 2;


    glfwTerminate();
}




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

            sprintf(img_file_nm,"./mand_%d.bmp", framenum); // Might wanna have it write into a folder...

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


    /*
    for(int x = 0; x<width;x++){
        fwrite(&img[x], BYTES_PER_PIXEL, width, imageFile);
    }
     */
    for(int x = 0; x<width;x++){
        for(int y = 0; y < height; y++){
            fwrite(&img[x][y][0], sizeof(unsigned char), 1, imageFile);
            fwrite(&img[x][y][1], sizeof(unsigned char), 1, imageFile);
            fwrite(&img[x][y][2], sizeof(unsigned char), 1, imageFile);
        }
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

// BAD BAD BAD BAD BADDDDDDD
// I NEED TO GET A BETTER SOLUTIONNNN
int load_config() {

    FILE* cfg_file = fopen("config.txt", "r");

    if(cfg_file == nullptr) {
        return 1;
    }

    char cfg_buffer[256];


    int i = 0;
    while (fgets(cfg_buffer, sizeof(cfg_buffer), cfg_file) != nullptr) {
        // This gets every line.
        //std::printf("%s", cfg_buffer);

        int cfg_int = atoi(cfg_buffer);
        double cfg_dbl = atof(cfg_buffer);

        if(i == 9) {

            if(cfg_int % 32 == 0 && cfg_int != 0) {
                width = cfg_int;
                printf("Loading width of %d\n", cfg_int);
            }else {
                printf("Width is not a multiple of 32 greater than 0 or is not a number, got %s\n", cfg_buffer);
                fclose(cfg_file);
                return 1;
            }

        }else if(i == 10) {

            if(cfg_int % 32 == 0 && cfg_int != 0) {
                height = cfg_int;
                printf("Loading height of %d\n", cfg_int);

            }else {
                printf("Height is not a multiple of 32 greater than 0 or is not a number, got %s\n", cfg_buffer);
                fclose(cfg_file);
                return 1;
            }

        }else if(i == 11) {

            if(cfg_int > 0 && cfg_int != 0) {
                maxiter = cfg_int;
                printf("Loading max iter of %d\n", cfg_int);
            }else {
                printf("Max iter is not a number greater than 0 or is not a number, got %s\n", cfg_buffer);
                fclose(cfg_file);
                return 1;
            }

        }else if(i == 12) {

            if(cfg_dbl != 0) {
                zoomspeed = (float)cfg_dbl;
                printf("Loading zoom speed of %f\n", cfg_dbl);
            }else {
                printf("Zoom speed is not a number greater than 0 or is not a number, got %s\n", cfg_buffer);
                fclose(cfg_file);
                return 1;
            }

        }else if(i == 13) {


            if(cfg_dbl != 0) {
                startzoom = (float)cfg_dbl;
                printf("Loading start zoom of %f\n", cfg_dbl);
            }else {
                printf("Start zoom is not a number greater than 0 or is not a number, got %s\n", cfg_buffer);
                fclose(cfg_file);
                return 1;
            }

        }else if(i == 14) {


            if(cfg_dbl != 0) {
                maxzoom = (float)cfg_dbl;
                printf("Loading max zoom of %f\n", cfg_dbl);
            }else {
                printf("Max zoom is not a number greater than 0 or is not a number, got %s\n", cfg_buffer);
                fclose(cfg_file);
                return 1;
            }

        }else if(i == 15) {


            if(cfg_dbl != 0) {
                x_pos = cfg_dbl*10000000.0;
                printf("Loading X offset of %f\n", cfg_dbl);
            }else {
                printf("X offset is not a number greater than or less than 0 or is not a number, got %s\n", cfg_buffer);
                fclose(cfg_file);
                return 1;
            }

        }else if(i == 16) {


            if(cfg_dbl != 0) {
                y_pos = cfg_dbl*10000000.0;
                printf("Loading y offset of %f\n", cfg_dbl);
            }else {
                printf("Y offset is not a number greater than or less than 0 or is not a number, got %s\n", cfg_buffer);
                fclose(cfg_file);
                return 1;
            }

        }else if(i >= 17){
            printf("\nThe config file is too long? (Has too many lines)\n");
            fclose(cfg_file);
            return 1;
        }

        i++;
    }

    if(i != 17){
        printf("\nThe config file is too short? (Has too few lines)\n");
        fclose(cfg_file);
        return 1;
    }

    // Close the file
    fclose(cfg_file);

    return 0;
}


int main (int argc, char* argv[])
{
    // loading config file.
    int config_code = load_config();
    if(config_code == 1){
        printf("Something went wrong with config loading! \n");

        exit(1);
    }

    img = new unsigned char**[width];
    for (int i = 0; i < width; ++i) {
        img[i] = new unsigned char*[height];
        for (int j = 0; j < height; ++j) {
            img[i][j] = new unsigned char[3];
        }
    }


    std::thread t1(disk_thread);
    std::thread t2(process_stuff);

    t2.join();
    t1.join();


    // Deallocate memory for the 3D array
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            delete[] img[i][j];
        }
        delete[] img[i];
    }
    delete[] img;

    printf("Image generated!!");
}