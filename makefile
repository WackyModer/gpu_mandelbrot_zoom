SOURCE = ./src/main_bmp.cpp ./src/threadw.cpp
GLEW_INC = -I.\openGL_libraries\glew-2.2.0-win32\glew-2.2.0\include
GLFW_INC = -I.\openGL_libraries\glfw-3.3.9.bin.WIN64\glfw-3.3.9.bin.WIN64\include
GLM_INC = -I.\openGL_libraries\glm.0.9.9.800\build\native\include
INCLUDES = $(GLEW_INC) $(GLFW_INC) $(GLM_INC)

GLFW_LNK = -L.\openGL_libraries\glfw-3.3.9.bin.WIN64\glfw-3.3.9.bin.WIN64\lib-mingw-w64
GLEW_LNK = -L.\openGL_libraries\glew-2.2.0-win32\glew-2.2.0\lib\Release\x64
LINKS = $(GLEW_LNK) $(GLFW_LNK)
ELS = -luser32 -lkernel32 -lgdi32 -lglew32s -lglew32 -lglfw3dll -lglfw3

all:
	g++ -c -o ./main_bmp.obj ./src/main_bmp.cpp $(INCLUDES)
	g++ -c -o ./threadw.obj ./src/threadw.cpp $(INCLUDES)
	g++ ./threadw.obj ./main_bmp.obj $(LINKS) $(ELS) -o main.exe

bep:
	g++ ./src/main_bmp_noGPU.cpp -o ea.exe


aee:
	main.exe 100 50 test.bmp