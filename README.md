# examples-cpp
<img align="left" src="https://github.com/raysan5/raylib/raw/master/logo/raylib_logo_animation.gif" width="64">
Examples and samples made for raylib using C++

## Building
The examples use premake. Each example is a project on it's own. Premake will setup the build systems for each project for you, and download any needed libraries (raylib and box2d)

### For Visual Studio
Run premake-VisualStudio.bat. When it is done each example folder will have a .sln file in it that you can open and run.

### For MinGW
Run premake-mingw.bat. When it is done each example folder will have a makefile that you can build, just cd into the example folder and type make.

### For GCC On Linux
Run "premake5 gmake2". When it is done each example folder will have a makefile that you can build, just cd into the example folder and type make.

### For GCC On MacOs
Run "premake5.osx gmake2". When it is done each example folder will have a makefile that you can build, just cd into the example folder and type make.

# Pew
A very simple game where you drive a tank around and shoot targets
![pew_game](https://user-images.githubusercontent.com/322174/138608560-47de649e-7316-42f3-a4f5-c8ba59ef8b98.gif)

# Cards
An example of how to build up cards, decks, and hands and drag cards around.
![cards](https://user-images.githubusercontent.com/322174/138608557-5b1dfeb3-33a3-409c-8635-0dac7f7cdf36.gif)

#Platform Example
An example of platfomer style movement, sprite animation, and state management.
![platform_jump](https://user-images.githubusercontent.com/322174/147867102-ce62fbbd-2f2a-4ccd-8d44-9f49450b9df5.gif)

#Unsorted Billboards
An example of using shaders to discard pixels with 0 alpha to allow billboards to be drawn in any order.
![unsorted_billobards](https://user-images.githubusercontent.com/322174/148694937-f7c4b166-b81a-4d48-af8f-eefbc1a3f487.gif)


#Box2d Raylib
An example of using the Box2d physics library with raylib.
![Box2d_spin](https://user-images.githubusercontent.com/322174/155644898-4667f4e6-894c-4ae0-90bd-485c503fe2a6.gif)

#Box2d Raylib Objects
An more advanced example using box2d and objects
![box2d_objects](https://user-images.githubusercontent.com/322174/155657154-b13f5fa7-2f18-43ce-a647-deb2cebff826.gif)

#Voxel Mesher
An example of how to create blockgame (minecraft) style voxel chunk meshes.

