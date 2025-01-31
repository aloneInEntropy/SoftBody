#version 460 core
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_texture;
layout(location = 3) in ivec4 bone_ids;
layout(location = 4) in vec4 bone_weights;
layout(location = 5) in mat4 instance_trans; // transform of mesh instance
layout(location = 9) in float texture_depth;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out float tDepth;
out int toggleNormal;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 bones[200];
uniform int showNormal;

void main() {
  vec4 totalPos = vec4(0.0);
  vec3 totalNormal = vec3(0.0);
  int cnt = 0; // number of bones
  int pp[300];
  for (int i = 0; i < 4; i++) {
    if (bone_ids[i] == -1) // ignore unbound bones
      continue;
    cnt++;
    mat4 bone = bones[bone_ids[i]];
    totalPos += bone_weights[i] * bone * vec4(vertex_position, 1.0);

    vec3 worldNormal = mat3(transpose(inverse(bone))) * vertex_normal;
    totalNormal += worldNormal * bone_weights[i];
  }

  if (cnt == 0) {
    // if no bones, animate as if it's static
    totalPos = vec4(vertex_position, 1.0);
    FragPos = vec3(instance_trans * vec4(vertex_position, 1.0));
    Normal = mat3(transpose(inverse(instance_trans))) * vertex_normal;
  } else {
    FragPos = vec3(instance_trans * totalPos);
    Normal = vec3(normalize(instance_trans * vec4(totalNormal, 0.0)));
  }

  TexCoords = vertex_texture;
  tDepth = texture_depth;
  toggleNormal = showNormal;
  gl_Position = proj * view * instance_trans * totalPos;
}
