#version 330 core

in vec2 TexCoord;           // Texture coordinates passed from the vertex shader
out vec4 FragColor;         // Final output color

uniform sampler2D myTexture; // Texture sampler

void main() {
    FragColor = texture(myTexture, TexCoord); // Sample the texture color
}
