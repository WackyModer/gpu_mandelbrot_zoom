#include <cstdio>
#include <cmath>
#include <glm/glm.hpp>
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <thread>

#include "./shared.h"

#define GLFW_DLL

// Crappy multithreaded variable management
int side = 0;

// Both width and height need to be the same and need to be a multiple of 32
int width = 256;
// Both width and height need to be the same and need to be a multiple of 32
int height = 256;


// Keep the same as ints width and height
unsigned char img[256][256][3];


unsigned int framenum = 0;
unsigned int maxframes = 0xffffffff; // ehhhhh

int maxiter = 5000;//5000;



double x_pos = -0.743643887037158704752191506114774*1000000.0; // ok, this whole multiplication thing gives more decimal accuracy
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

    double offset_x = dataa[1].pos.x/1000000.0;
    double offset_y = dataa[1].pos.y/1000000.0;

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




    double zoooom = startzoom;

    for(int i = 0; i < maxframes; i++) {
        if(zoooom > maxzoom) {
            break;
        }

        // To test how a zoom will look at x zoom.
        // zoooom = 400000000.0;

        printf("\n\n%f\n\n",zoooom);

        GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(computeShader, 1, &computeShaderSource, NULL);
        glCompileShader(computeShader);

        GLuint computeProgram = glCreateProgram();
        glAttachShader(computeProgram, computeShader);
        glLinkProgram(computeProgram);




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
        zoooom *= zoomspeed;



        side = 1;
    }
    side = 2;


    glfwTerminate();
}

int main (int argc, char* argv[])
{
    std::thread t1(disk_thread);
    std::thread t2(process_stuff);

    t2.join();
    t1.join();

    printf("Image generated!!");
}