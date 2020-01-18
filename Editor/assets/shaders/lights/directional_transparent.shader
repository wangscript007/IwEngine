#shader Vertex
#version 430 core

layout(std140, column_major) uniform Camera {
	mat4 viewProj;
};

uniform mat4 model;

layout(location = 0) in vec3 vert;
layout(location = 2) in vec2 uv;

out vec2 UV;

void main() {
	UV = uv;
	gl_Position = viewProj * model * vec4(vert, 1);
}

#shader Fragment
#version 430 core

uniform sampler2D alphaMask;
uniform float alphaThreshold;

in vec2 UV;

void main() {
	if (alphaThreshold > texture(alphaMask, UV).r) {
		discard;
	}

	float depth = gl_FragCoord.z;

	float dx = dFdx(depth);
	float dy = dFdy(depth);
	float bias = depth * depth + 0.25 * (dx * dx + dy * dy);

	gl_FragColor = vec4(depth, bias, 0, 1);
}