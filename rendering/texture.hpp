
#ifndef TEXTURE_H
#define TEXTURE_H
#include<string>

struct Texture {
    unsigned int id;
    unsigned w;
    unsigned h;
    const unsigned char* albedo;
    std::string name;

    void swap(Texture&& t) {
        id = t.id;
        albedo = t.albedo;
        name = t.name;

        t.id = 0;
    }
};


#endif // !TEXTURE_H
