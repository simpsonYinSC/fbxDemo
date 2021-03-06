#version 430 core

in Vertex 
{
  vec3 normal;
  vec3 color;
  vec3 view;
  vec3 L;
} vertex;

out vec4 color;

// Material properties
uniform vec3 diffuse_albedo = vec3(0.1, 0.23, 0.5);
uniform vec3 specular_albedo = vec3(0.7);
uniform float specular_power = 128.0;

uniform vec3 rim_color;
uniform float rim_power = 16.0;

vec3 calculate_rim(vec3 N, vec3 V)
{
  float f = 1.0 - dot(N, V);
  f = smoothstep(0.0, 1.0, f);
  f = pow(f, rim_power);
  return f * rim_color;
}

void main(void)
{
  vec3 N_fs = normalize(vertex.normal);
  vec3 L_fs = normalize(vertex.L);
  vec3 V_fs = normalize(vertex.view);

  vec3 H = normalize(L_fs + V_fs);

  vec3 diffuse = max(dot(N_fs, L_fs), 0.0) * diffuse_albedo;
  vec3 specular = pow(max(dot(N_fs, H), 0.0), specular_power) * specular_albedo;
  vec3 rim_lighting = calculate_rim(N_fs, V_fs);
  color = vec4(diffuse + specular + rim_lighting, 1.0);
}
