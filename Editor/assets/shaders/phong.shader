#shader Vertex
#version 420

#define MAX_DIRECTIONAL_LIGHTS 4

#include shaders/camera.shader
#include shaders/shadows.vert

layout (location = 0) in vec3 vert;
layout (location = 1) in vec3 normal;
//tangents
layout (location = 4) in vec2 uv;
layout (location = 5) in vec4 color;

out vec3 WorldPos;
out vec3 CameraPos;
out vec2 TexCoords;
out vec3 Normal;
out vec4 Color;

uniform mat4 model;

// material parameters

uniform float     mat_hasDisplacementMap;
uniform sampler2D mat_displacementMap;

void main() {
	vec3 displacement = vec3(0);
	if (mat_hasDisplacementMap == 1) {
		displacement = normal * texture(mat_displacementMap, uv).r;
	}

	vec4 worldPos    = model * vec4(vert + displacement, 1);
	mat3 modelVector = transpose(inverse(mat3(model)));

	WorldPos  = worldPos.xyz;
	CameraPos = camPos.xyz;
	TexCoords = uv;
	Normal    = normalize(modelVector * normal);
	Color     = color;

	SetDirectionalLightPos(worldPos);

	gl_Position = viewProj * worldPos;
}

#shader Fragment shaders/phong.frag
