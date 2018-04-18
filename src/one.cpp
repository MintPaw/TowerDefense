#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
# include <windows.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"
#include "../lib/json.c"

#include "mathTools.cpp"
#include "memoryTools.cpp"

#include "platform.cpp"
#include "renderer.cpp"

#include "main.cpp"
