#version 330
in vec4 p;
out vec2 u;
void main()
{
u=vec2((p.x+1)/2,1-(p.y+1)/2);
gl_Position=vec4(p.x,p.y,1,1);
}