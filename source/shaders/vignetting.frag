#version 330 core

uniform sampler2D c;
in vec2 u;
out vec4 f;

//uniform float vignetteStrength; 
const float vignetteStrength = 0.2; 

void main() 
{
    vec3 color  = texture(c, u).rgb;  
    vec2 center = vec2(0.5, 0.5);
    float dist  = length(u - center); 
    float vignette = smoothstep(vignetteStrength, 0.8, dist); 
    color *= 1.0 - vignette;
    f = vec4(color, 1.0);  
}