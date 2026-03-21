#version 330 core
// ^ Specifies we are using OpenGL version 3.3 in "Core" mode

// --- INPUTS ---
// The texture coordinate map that was forwarded to us from `vertex.glsl`
in vec2 TexCoord;

// --- OUTPUTS ---
// The absolute final RGBA color we are definitively drawing to the user's screen at this pixel
out vec4 FragColor;

// --- UNIFORMS (GLOBAL CPU VARIABLES) ---
// This is pushed down from C++ whenever we do `m_renderer->drawRect(..., ..., COLOR)`
uniform vec4      color;
uniform sampler2D image;
uniform int       useTexture;

void main() {
    if (useTexture == 1) {
        // Multiply texture sample by tint; tint defaults to white (no change)
        FragColor = texture(image, TexCoord) * color;
    } else {
        FragColor = color;
    }
}