#pragma once // Tells the C++ compiler to only include this file once per compilation unit.

#include "glad/glad.h" // Includes OpenGL functions (like glActiveTexture and glBindTexture).

// A simple container to hold an OpenGL Texture's ID and its physical dimensions.
// We use a 'struct' instead of a 'class' because all members are public data 
// and it doesn't need complex getters/setters or encapsulation.
struct Texture2D {
    unsigned int ID     = 0; // The hardware ID given to us by OpenGL when the texture is created.
    int          width  = 0; // The width of the image in pixels.
    int          height = 0; // The height of the image in pixels.

    // Activates this specific texture on the GPU so that the next draw call uses it.
    // 'unit' defaults to 0 if you don't pass an argument.
    void bind(unsigned int unit = 0) const {
        
        // Selects the texture "slot" or "unit" we want to place this image into.
        // We add our requested unit number to the base GL_TEXTURE0 memory address.
        glActiveTexture(GL_TEXTURE0 + unit); 
        
        // Binds our specific texture ID to the 2D target of the currently active slot.
        // Now, any shader looking at this texture unit will see our image!
        glBindTexture(GL_TEXTURE_2D, ID); 
    }
};