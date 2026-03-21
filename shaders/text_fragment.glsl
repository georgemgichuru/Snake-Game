#version 330 core
in  vec2 TexCoords;
out vec4 FragColor;

// FreeType glyphs are single-channel (RED only) bitmaps
uniform sampler2D text;
uniform vec4      textColor;

void main() {
    // Sample just the red channel — it holds the glyph's alpha mask
    float alpha = texture(text, TexCoords).r;
    FragColor   = vec4(textColor.rgb, textColor.a * alpha);
}