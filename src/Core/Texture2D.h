#pragma once
#include "glad/glad.h"

struct Texture2D {
    unsigned int ID     = 0;
    int          width  = 0;
    int          height = 0;

    void bind(unsigned int unit = 0) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, ID);
    }
};
