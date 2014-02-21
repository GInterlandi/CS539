#version 110

attribute  vec4 vPosition;

uniform mat4 modelview;

void main() 
{ 
  gl_Position = modelview*vPosition;
}