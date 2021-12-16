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
void phongComputation_diffuse_and_specular(in vec4 position, in vec4 normal, in vec4 lightPos, out vec4 diffuse, out vec4 specular);

void main() {
	phongComputation(normal, position, color);
	worldPosition = position;
}

void phongComputation(in vec4 normal, in vec4 position, out vec4 color_out) {
	// ambient term is always constant :
	vec3 a = shadingParams.x * ambient_color;

	// first, compute the position of the light (lightPos_model is direction of displacement, and
	// length of it will be the size of the BB * 2.f from the BB's min position :
	float factor = length(bb_max - bb_min) * 2.f;
	vec4 lightPos = bb_min + factor * lightPos_model;

	vec4 d0 = vec4(.0f, .0f, .0f, .0f);
	vec4 d1 = vec4(.0f, .0f, .0f, .0f);
	vec4 s0 = vec4(.0f, .0f, .0f, .0f);
	vec4 s1 = vec4(.0f, .0f, .0f, .0f);

	// Compute diffuse & specular for the BB light first, then the camera :
	phongComputation_diffuse_and_specular(position, normal, lightPos, d0, s0);
	phongComputation_diffuse_and_specular(position, normal, camera_pos, d1, s1);

	color_out = vec4(a + ((d0 + s0).xyz + (d1 + s1).xyz)/2.f, 1.f);
}

void phongComputation_diffuse_and_specular(in vec4 position, in vec4 normal, in vec4 lightPos, out vec4 diffuse, out vec4 specular) {
	vec3 l = normalize(lightPos.xyz - position.xyz);
	vec3 n = normalize(normal.xyz);
	float ln = max(.0f, dot(l, n));

	vec3 r = normalize(reflect(-l, n));	// reflect() takes incident, not direction !
	vec3 v = normalize(camera_pos.xyz - position.xyz);
	float rv = max(.0f, dot(r, v));

	vec3 d = shadingParams.y * ln * bone_base_color;						// Diffuse param
	vec3 s = shadingParams.z * pow(rv, shadingParams.w) * specular_color;	// Specular param

	diffuse = vec4(d, 1.f);
	specular = vec4(s, 1.f);
}
