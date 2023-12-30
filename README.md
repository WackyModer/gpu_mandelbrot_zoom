# gpu_mandelbrot_zoom
A C++ gpu-based mandelbrot zoom generator that uses OpenGL

[Video of it](github.com/WackyModer/gpu_mandelbrot_zoom/blob/main/showcase/480p_mand_zoom.mp4)



## How to get it:

~~First of all, it requires **you** to get all the libraries and put them in the right spot.~~

~~The makefile is set up for mingw64 on windows, so Mac isn't getting one, and I'm working on it for linux.~~

~~Now once you have all the libraries and everything run `make` then run `main` to run the program.~~

The program is now a precompiled binary for 64 bit windows. If you want to compile it you're in charge of getting the libraries, the names and versions are `glew 2.2.0`, `glfw 3.3.9`, and `glm 0.9.9.8`

edit config.txt to change things like iterations, max zoom, starting zoom, position, resolution, and enabling/disabling smoothing.

## If you have a fix or optimisation or fix, submit a PR, if theres an issue open an issue (or fix it yourself).
