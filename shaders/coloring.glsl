#version 400 core

// This shader is only included in other shaders to have
// stable coloring functions !

vec4 voxelIdxToColor_1channel(in vec2 colorBounds, in uvec3 ucolor) {
	// Have the R and G color channels clamped to the min/max of the scale
	// (mimics under or over-exposure)
	float color_r = clamp(float(ucolor.r), colorBounds.x, colorBounds.y);
	float color_g = clamp(float(ucolor.r), colorBounds.x, colorBounds.y);
	// Compute the color as Brian's paper describes it :
	float color_k = 2.5;
	float sc = colorBounds.y - colorBounds.x;
	float eosin = (color_r - colorBounds.x)/(sc);
	float dna = (color_g - colorBounds.x)/(sc); // B is on G channel because OpenGL only allows 2 channels upload to be RG, not RB

	float eosin_r_coef = 0.050;
	float eosin_g_coef = 1.000;
	float eosin_b_coef = 0.544;

	float hematoxylin_r_coef = 0.860;
	float hematoxylin_g_coef = 1.000;
	float hematoxylin_b_coef = 0.300;

	float r_coef = eosin_r_coef;
	float g_coef = eosin_g_coef;
	float b_coef = eosin_b_coef;

	return vec4(
		exp(-hematoxylin_r_coef * dna * color_k) * exp(-eosin_r_coef * eosin * color_k),
		exp(-hematoxylin_g_coef * dna * color_k) * exp(-eosin_g_coef * eosin * color_k),
		exp(-hematoxylin_b_coef * dna * color_k) * exp(-eosin_b_coef * eosin * color_k),
		1.
	);
}

vec4 voxelIdxToColor_2channel(in vec2 colorBounds, in uvec3 ucolor) {
	// Have the R and G color channels clamped to the min/max of the scale
	// (mimics under or over-exposure)
	float color_r = clamp(float(ucolor.r), colorBounds.x, colorBounds.y);
	float color_g = clamp(float(ucolor.g), colorBounds.x, colorBounds.y);
	// Compute the color as Brian's paper describes it :
	float color_k = 2.5;
	float sc = colorBounds.y - colorBounds.x;
	float eosin = (color_r - colorBounds.x)/(sc);
	float dna = (color_g - colorBounds.x)/(sc); // B is on G channel because OpenGL only allows 2 channels upload to be RG, not RB

	float eosin_r_coef = 0.050;
	float eosin_g_coef = 1.000;
	float eosin_b_coef = 0.544;

	float hematoxylin_r_coef = 0.860;
	float hematoxylin_g_coef = 1.000;
	float hematoxylin_b_coef = 0.300;

	float r_coef = eosin_r_coef;
	float g_coef = eosin_g_coef;
	float b_coef = eosin_b_coef;

	return vec4(
		exp(-hematoxylin_r_coef * dna * color_k) * exp(-eosin_r_coef * eosin * color_k),
		exp(-hematoxylin_g_coef * dna * color_k) * exp(-eosin_g_coef * eosin * color_k),
		exp(-hematoxylin_b_coef * dna * color_k) * exp(-eosin_b_coef * eosin * color_k),
		1.
	);
}

vec4 hsv2rgb(in vec2 colorBounds, in uvec3 ucolor) {

	//Retourne une échelle de couleur pour une valeure flottante normalisée entre 0 et 1
	float value = clamp((float(ucolor.r) - colorBounds.x) / ((1.01*colorBounds.y) - colorBounds.x), .0, 1.);
	vec3 hsv = vec3 (value, 1., 1.);
	hsv.x = mod( 100.0 + hsv.x, 1.0 ); // Ensure [0,1[
	float   HueSlice = 6.0 * hsv.x; // In [0,6[
	float   HueSliceInteger = floor( HueSlice );
	float   HueSliceInterpolant = HueSlice - HueSliceInteger; // In [0,1[ for each hue slice
	vec3    TempRGB = vec3(   hsv.z * (1.0 - hsv.y), hsv.z * (1.0 - hsv.y * HueSliceInterpolant), hsv.z * (1.0 - hsv.y * (1.0 - HueSliceInterpolant)) );
	float   IsOddSlice = mod( HueSliceInteger, 2.0 ); // 0 if even (slices 0, 2, 4), 1 if odd (slices 1, 3, 5)
	float   ThreeSliceSelector = 0.5 * (HueSliceInteger - IsOddSlice); // (0, 1, 2) corresponding to slices (0, 2, 4) and (1, 3, 5)
	vec3    ScrollingRGBForEvenSlices = vec3( hsv.z, TempRGB.zx );           // (V, Temp Blue, Temp Red) for even slices (0, 2, 4)
	vec3    ScrollingRGBForOddSlices = vec3( TempRGB.y, hsv.z, TempRGB.x );  // (Temp Green, V, Temp Red) for odd slices (1, 3, 5)
	vec3    ScrollingRGB = mix( ScrollingRGBForEvenSlices, ScrollingRGBForOddSlices, IsOddSlice );
	float   IsNotFirstSlice = clamp( ThreeSliceSelector, 0.0,1.0 );                   // 1 if NOT the first slice (true for slices 1 and 2)
	float   IsNotSecondSlice = clamp( ThreeSliceSelector-1.0, 0.0,1. );              // 1 if NOT the first or second slice (true only for slice 2)
	return  vec4(mix( ScrollingRGB.xyz, mix( ScrollingRGB.zxy, ScrollingRGB.yzx, IsNotSecondSlice ), IsNotFirstSlice ), 1.);    // Make the RGB rotate right depending on final slice index
}

vec4 colorSegmentColoration(in mat3 colorSegment, in vec2 colorBounds, in uvec3 ucolor) {
	float t = clamp((float(ucolor.r) - colorBounds.x) / (colorBounds.y - colorBounds.x), .0, 1.);

	vec4 color0 = vec4(colorSegment[0].xyz, 1.);
	vec4 color1 = vec4(colorSegment[1].xyz, 1.);

	return mix(color0, color1, t);
}

vec4 voxelIdxToColor(in uvec3 colorParams, in mat3 colorSegment, in vec2 colorBounds, in uvec3 ucolor) {
	// Texture and color parameters :
	uint colorFunction = colorParams.x;
	uint colorChannel = colorParams.y;
	uint nbChannels = colorParams.z;
	if (colorFunction == 1u) {
		if (nbChannels == 1u) {
			float alpha = 1.f;
			float val = (float(ucolor.r) - colorBounds.x)/(colorBounds.y-colorBounds.x);
			return vec4(val, val, val, alpha);
		} else {
			uint uval = (colorChannel == 0u) ? ucolor.r : ucolor.g;
			float alpha = 1.f;
			float val = (float(uval) - colorBounds.x)/(colorBounds.y-colorBounds.x);
			return vec4(val, val, val, alpha);
		}
	} else if (colorFunction == 2u) {
		if (nbChannels == 1u) { return voxelIdxToColor_1channel(colorBounds, ucolor); }
		else { return voxelIdxToColor_2channel(colorBounds, ucolor); }
	} else if (colorFunction == 3u) {
		return hsv2rgb(colorBounds, ucolor);
	} else if (colorFunction == 4u) {
		return colorSegmentColoration(colorSegment, colorBounds, ucolor);
	} else {
		return vec4(.0,.0,.0,1.);
	}
}

