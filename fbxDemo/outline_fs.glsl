#version 430 core

in Vertex {
  vec3 normal;
  vec3 color;
} vertex;

out vec4 color;

void main(void)
{
  color = vec4(vertex.color, 1.0);
}