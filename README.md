# gpu_mandelbrot_zoom
A C++ gpu-based mandelbrot zoom generator that uses OpenGL

[Video of it](github.com/WackyModer/gpu_mandelbrot_zoom/blob/main/showcase/480p_mand_zoom.mp4)



## How to get it:

First of all, it requires **you** to get all the libraries and put them in the right spot.

The makefile is set up for mingw64 on windows, so Mac isn't getting one, and I'm working on it for linux.


First of all to change any settings you have to edit the main C++ file and the `shared.h` file. Its not ideal, and I'm working on a fix.

Now once you have all the libraries and everything run `make` then run `main` to run the program.



## If you have a fix or optimisation submit a PR, if theres an issue open an issue.
