#version 430

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in Vertex
{
  vec3 normal;
  vec3 color;
  vec3 view;
  vec3 L;
} vertex[];

out Vertex 
{
  vec3 normal;
  vec3 color;
  vec3 view;
  vec3 L;
} Out;

uniform float line_width = 0.00001;

void main(void) 
{
  int i;

  for(i = 0; i < gl_in.length(); i++)
  {
    gl_Position = gl_in[i].gl_Position + line_width * vec4(vertex[i].normal, 0.0);
    Out.color = vec3(0,0,0);
    Out.normal = vertex[i].normal;
    // try going back to using surface normals, might look better.
    
    float dot_product = dot(-vec3(vertex[i].view), Out.normal);
    if(dot_product >= 0.0) {
      EmitVertex();
    }
  }
  EndPrimitive();
}
