
#ifndef IMAGE_LOAD_H
#define IMAGE_LOAD_H

#include "stb.hpp"

void setTexture(const char* path) {
    

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        stbi_image_free(data);
    }
    else
    {
        
        stbi_image_free(data);
        return;
    }
    
}
#endif // !IMAGE_LOAD_H
