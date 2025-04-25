#version 330 core

uniform sampler2D c; 
in vec2 u;
out vec4 f; 

//uniform float aberrationStrength;
const float aberrationStrength = 0.006; 

void main() 
{
    vec2 offset = vec2(aberrationStrength, 0.0); 
    float r = texture(c, u + offset).r; 
    float g = texture(c, u).g;         
    float b = texture(c, u - offset).b; 
    f = vec4(r, g, b, 1.0); 
}