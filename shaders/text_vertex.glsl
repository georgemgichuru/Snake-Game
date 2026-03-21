#version 330 core
// Each glyph quad has 4 floats: screen position (x,y) + UV coord (u,v)
layout (location = 0) in vec4 vertex;

out vec2 TexCoords;

// Orthographic projection matching screen pixel space
uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords   = vertex.zw;
}