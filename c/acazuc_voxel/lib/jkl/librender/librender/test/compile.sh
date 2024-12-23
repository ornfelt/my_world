#!/bin/bash
g++ basic_context.cpp -I ../include -I ../lib/glad/include -I ../lib/glfw/include -lglfw -L .. -lrender -L ../lib/glad -lglad -ldl -lm -I /usr/include/freetype2 -lfreetype
