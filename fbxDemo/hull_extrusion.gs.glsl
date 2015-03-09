#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 6) out;

in Vertex
{
  vec3 normal;
  vec3 color;
} vertex[];

out vec3 normal_gs;
out vec3 color_gs;

uniform float line_width = 0.01f;

void main(void) 
{
  int i;

  for(i = 0; i < gl_in.length(); i++)
  {
    gl_Position = gl_in[i].gl_Position + line_width * vec4(vertex[i].normal, 0.0);
    color_gs = vec3(0,0,0);
    normal_gs = vertex[i].normal;
    EmitVertex();
  }
  EndPrimitive();
}
