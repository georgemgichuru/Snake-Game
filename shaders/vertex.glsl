#version 330 core
// ^ Specifies we are using OpenGL version 3.3 in "Core" mode

// --- INPUT ATTRIBUTES ---
// Location 0 corresponds to position data we uploaded via C++ (x, y)
layout (location = 0) in vec2 aPos;
// Location 1 corresponds to texture coordinates (u, v) (if we wanted to paste images)
layout (location = 1) in vec2 aTexCoord;

// --- OUTPUT TO FRAGMENT SHADER ---
// "out" pushes this variable safely down the pipeline into the next shader over.
out vec2 TexCoord;

// --- UNIFORMS (GLOBAL CPU VARIABLES) ---
// These are values we set directly from our Game loop via `Renderer.cpp` calls like ->setMat4()
// "model" typically shifts the position, scales, or rotates out raw coordinates.
uniform mat4 model;
// "projection" converts the abstract maths into actual pixel positions fitting your Window Width/Height
uniform mat4 projection;

void main() {
    // Math logic: Multiply the raw geometry point against our scaling and our camera viewport.
    // We add 0.0 and 1.0 to forcibly convert from a 2D format to the required graphical 4D Vector.
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    
    // Pass the texture coordinate straight through without altering it.
    TexCoord = aTexCoord;
}