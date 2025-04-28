#version 330

// src: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/

uniform sampler2D c;
in vec2 u;
out vec4 f;

const float A = 2.51;
const float B = 0.03;
const float C = 2.43;
const float D = 0.59;
const float E = 0.14;

vec3 aces(const vec3 color)   
{
    return (color * (A * color + B)) / (color * (C * color + D) + E); 
}

void main()
{
    vec3 hdrColor       = texture(c, u).rgb;   
    vec3 toneMapped     = aces(hdrColor);   
    vec3 gammaCorrected = sqrt(toneMapped);  
    f = vec4(gammaCorrected, 1.0);    
} 
