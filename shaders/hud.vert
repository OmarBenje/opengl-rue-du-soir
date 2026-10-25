#version 330 core
layout(location=0) in vec2 pos;
uniform vec2 resolution;
void main(){gl_Position=vec4(pos.x/resolution.x*2.0-1.0,1.0-pos.y/resolution.y*2.0,0.0,1.0);}
