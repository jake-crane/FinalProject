#!/bin/bash
g++ *.cpp \
	-D GLEW_STATIC \
	-D GLM_FORCE_RADIANS \
	-I ../../glew-1.11.0/include/ \
	-I ../../freeglut/include/ \
	-I ../../glfw-3.0.4.bin.WIN32/glfw-3.0.4.bin.WIN32/include/GLFW/ \
	-I ../../glm-0.9.5.4/glm/ \
	-I . \
	-L ../../glew-1.11.0/lib/Release/Win32/ \
	-lglew32s -lglew32 \
	-L ../../freeglut/lib/ -lfreeglut \
	-L ../../glew-1.11.0/lib/Release/Win32/ \
	-L ../../glfw-3.0.4.bin.WIN32/glfw-3.0.4.bin.WIN32/lib-mingw/ \
	-lglfw3 \
	-lopengl32 \
	-lglu32 \
	-lGdi32 \
	-std=c++11 \
	-o FinalProject.exe