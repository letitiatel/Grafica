#version 330 core
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 texCoord;

uniform mat4 myMatrix;
out vec2 TexCoord;

void main() {
    gl_Position = myMatrix * vec4(vertexPosition, 1.0);
    TexCoord = texCoord;
}
