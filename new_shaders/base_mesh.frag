#version 150
#extension GL_ARB_explicit_attrib_location : enable

in vec4 position;
in vec4 normal;
in vec2 texture;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform vec4 camera_pos;

uniform vec4 bb_min;
uniform vec4 bb_max;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 worldPosition;

/// @brief The position of the light, hardcoded simply in order to provide some sort of Phong shading
/// @note The default light position in blender !
vec4 lightPos_model = vec4(4.0762, 1.0055, 5.9039, 0.f);

/// @brief Shading paramters for the Phong rendering.
vec4 shadingParams = vec4(
	.5f,	// ka = k-ambient
	.7f,	// kd - k-diffuse
	.1f,	// ks - k-specular
	25.f	// \alpha - specular exponent
);

/// @brief Base color of the bone (RGB : #e3dac9)
vec3 bone_base_color = vec3(0.89f, 0.855f, 0.788f);
vec3 ambient_color = vec3(0.95f, 0.95f, 0.95f);
vec3 specular_color = vec3(0.502f, 0.502f, 0.502f);

void phongComputation(in vec4 normal, in vec4 position, out vec4 color);

void main() {
	vec3 base_normal = abs(normal.xyz / 1.5f);
	//color = vec4(base_normal, 1.f);
	phongComputation(normal, position, color);
	worldPosition = position;
}

void phongComputation(in vec4 normal, in vec4 position, out vec4 color_out) {
	// first, compute the position of the light (lightPos_model is direction of displacement, and
	// length of it will be the size of the BB * 2.f from the BB's min position :
	float factor = length(bb_max - bb_min) * 2.f;
	vec4 lightPos = bb_min + factor * lightPos_model;

	vec3 l = normalize(lightPos.xyz - position.xyz);
	vec3 n = normalize(normal.xyz);
	float ln = max(.0f, dot(l, n));

	vec3 r = normalize(reflect(-l, n));	// reflect() takes incident, not direction !
	vec3 v = normalize(camera_pos.xyz - position.xyz);
	float rv = max(.0f, dot(r, v));

	vec3 a = shadingParams.x * ambient_color;								// Ambient param
	vec3 d = shadingParams.y * ln * bone_base_color;						// Diffuse param
	vec3 s = shadingParams.z * pow(rv, shadingParams.w) * specular_color;	// Specular param

	color_out = vec4(a + d + s, 1.f);
}
