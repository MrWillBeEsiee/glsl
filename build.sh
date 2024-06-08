#!/bin/bash

# Utilisez les chemins MinGW corrects
INCLUDE_PATH="-I/mingw64/include"
LIB_PATH="-L/mingw64/lib"

# Spécifiez les bibliothèques nécessaires
LIBS="-lglew32 -lglfw3 -lgdi32 -lopengl32"

# Compilez le programme
g++ -o shader_program main.cpp $INCLUDE_PATH $LIB_PATH $LIBS
