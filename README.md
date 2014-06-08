leapfrop_verlet_opengl
======================

Leapfrop-Verlet showing in OpenGL

Requirements
------------

  -  g++
  -  OpenMP
  -  OpenGL/GLUT
  -  SDL2/SDL_image
  -  CMake

Compiling
---------

$ make release

Running
--------

$ ./build/simulate < test.in

Description
---------

The star catalog is read from the standard input. All files in 
"./textures" are loaded as texture images (the directory must exist, 
although it may be empty). The simulation is done by using Verlet algorithm [1].


[1] http://en.wikipedia.org/wiki/Verlet_integration


