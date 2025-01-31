#version 460

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_colour;
layout (location = 2) in vec2 vertex_texture;
layout (location = 3) in mat4 instance_trans;

layout(location = 0) out vec2 TexCoords;
layout(location = 1) out vec3 outColour;
layout(location = 2) out vec3 fragPos;

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;

void main() {
	outColour = vertex_colour;
	TexCoords = vertex_texture.xy;
    vec4 outPos = proj * view * instance_trans * vec4(vertex_position, 1.0);
	fragPos = vec3(instance_trans * vec4(vertex_position, 1.0));
	gl_Position = outPos;
}
