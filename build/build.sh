#!/bin/bash

# Utilisez les chemins MinGW corrects
INCLUDE_PATH="-Iinclude"
LIB_PATH="-L/mingw64/lib"

# Spécifiez les bibliothèques nécessaires
LIBS="-lglew32 -lglfw3 -lgdi32 -lopengl32"

# Compilez le programme en incluant les fichiers sources d'ImGui
g++ -o main_scene ../src/main.cpp ../include/imgui.cpp ../include/imgui_draw.cpp ../include/imgui_tables.cpp ../include/imgui_widgets.cpp ../include/imgui_impl_glfw.cpp ../include/imgui_impl_opengl3.cpp ../include/tiny_obj_loader.cc $INCLUDE_PATH $LIB_PATH $LIBS
