# CS636-Advanced-Rendering-Techniques

Artifacts from CS636 Advanced Rendering Techniques taken Spring 2020 at Drexel University on-line for Graduate Degree credit

#Repository Contents:

3rd-party: contains external libraries used by the c++ homework code
assets: holds any meshes, images, textures, etc used by the raytracer
class-site: a ruby powered static site displayed on this repository's associated gitlab pages, don't bother with this if you are looking into the raytracer
src: the c++ raytracer you are probably looking for.

#Setting up

`git clone --recurse-submodules https://gitlab.com/TaylorEllington/cs636-advanced-rendering-techniques.git`

or if you messed this up and cloned already without the submodules run

`git submodule update --init --recurse`

Then set up an out of source build with CMake (you need to install CMake)

`mkdir build`
`cd build`
`cmake ..`

That should get you going by generating a project targeted towards whatever is the default build system in your environment. (makefiles, xcode, Visual Studio Solution etc). If you end up with the wrong thing, refer to the CMake docs to get the right generator set up. Once thats done, you should be able to get the raytracer open in your IDE of choice and run it from there. 


#Features
##Homework 1
Camera -> image plane
Generating rays with image plane
Loading smf files
Generating primitives
Creating a scene
Writing to image
Ray/triangle intersection
Ray/sphere intersection
Basic architecture
Constant coloring