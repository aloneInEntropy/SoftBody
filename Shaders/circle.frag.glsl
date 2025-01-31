#version 460

layout(location = 0) out vec4 FragColour;

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 outColour;
layout(location = 2) in vec3 fragPos;

uniform sampler2D texture1;
layout(location = 2) uniform float radius;

void main() {
	// https://www.youtube.com/watch?v=kOkfC5fLfgE - sebastian lague, "rendering fluids", 27:58
	vec2 centre = (uv - 0.5) * 2;
	float sDist = dot(centre, centre);
	if (sDist > radius * radius) discard;
	FragColour = vec4(1);
}
