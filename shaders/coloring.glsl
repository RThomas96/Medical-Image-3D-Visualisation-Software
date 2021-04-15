#line 1001
//////////////////////////////////// COLOR FUNCTIONS ////////////////////////////////////

// This shader is only included in other shaders to have
// similar coloring functions ! Should not be compiled alone.
// Note : is included when encoutering "#pragma include_color_shader;"

#ifndef MAIN_SHADER_UNIT
uniform uint rgbMode;
uniform uint r_channelView;
uniform uint r_selectedChannel;
uniform uint r_nbChannels;
uniform uint g_channelView;
uniform uint g_selectedChannel;
uniform uint g_nbChannels;
uniform vec2 colorBounds;
uniform vec2 textureBounds;
uniform vec2 colorBoundsAlternate;
uniform vec2 textureBoundsAlternate;
uniform vec3 color0;
uniform vec3 color1;
uniform vec3 color0Alternate;
uniform vec3 color1Alternate;
#endif

vec4 voxelIdxToColor_1channel(in uvec3 ucolor) {
	// Have the R and G color channels clamped to the min/max of the scale
	// (mimics under or over-exposure)
	float color_r = clamp(float(ucolor.r), colorBounds.x, colorBounds.y);
	float color_g = clamp(float(ucolor.r), colorBounds.x, colorBounds.y);
	// Compute the color as Brian's paper describes it :
	float color_k = 2.5;
	float sc = colorBounds.y - colorBounds.x;
	float scAlt = colorBoundsAlternate.y - colorBoundsAlternate.x;
	float eosin = (color_r - colorBounds.x)/(sc);
	float dna = (color_g - colorBoundsAlternate.x)/(scAlt);

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

vec4 voxelIdxToColor_2channel(in uvec3 ucolor) {
	// Have the R and G color channels clamped to the min/max of the scale
	// (mimics under or over-exposure)
	float color_r = clamp(float(ucolor.r), colorBounds.x, colorBounds.y);
	float color_g = clamp(float(ucolor.g), colorBoundsAlternate.x, colorBoundsAlternate.y);
	// Compute the color as Brian's paper describes it :
	float color_k = 2.5;
	float sc = colorBounds.y - colorBounds.x;
	float scAlt = colorBoundsAlternate.y - colorBoundsAlternate.x;
	float eosin = (color_r - colorBounds.x)/(sc);
	float dna = (color_g - colorBoundsAlternate.x)/(scAlt);

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

vec4 hsv2rgb(in uint color, in vec2 cb) {

	//Retourne une échelle de couleur pour une valeure flottante normalisée entre 0 et 1
	float value = clamp((float(color) - cb.x) / ((1.01*cb.y) - cb.x), .0, 1.);
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

vec4 colorSegmentColoration(in vec2 cbounds, in vec3 color0, in vec3 color1, in uint ucolor) {
	float t = clamp((float(ucolor) - cbounds.x) / (cbounds.y - cbounds.x), .0, 1.);
	return vec4(mix(color0, color1, t), 1.);
}

vec4 color_red(in uvec3 ucolor) {
	if (r_channelView == 1u) {	// Greyscale
		float val = (float(ucolor.r) - colorBounds.x)/(colorBounds.y-colorBounds.x);
		return vec4(val, val, val, 1.);
	}
	// N°2 is HandE coloration, only available if both channels are visible
	if (r_channelView == 3u) {	// HSV-2-RGB
		return hsv2rgb(ucolor.r, colorBounds);
	}
	if (r_channelView == 4u){	// User-defined colors
		return colorSegmentColoration(colorBounds, color0, color1, ucolor.r);
	}
}

vec4 color_green(in uvec3 ucolor) {
	if (g_channelView == 1u) {	// Greyscale
		float alpha = 1.f;
		float val = (float(ucolor.g) - colorBoundsAlternate.x)/(colorBoundsAlternate.y-colorBoundsAlternate.x);
		return vec4(val, val, val, alpha);
	}
	// N°2 is HandE coloration, only available if both channels are visible
	if (g_channelView == 3u) {	// HSV-2-RGB
		return hsv2rgb(ucolor.g, colorBoundsAlternate);
	}
	if (g_channelView == 4u){	// User-defined colors
		return colorSegmentColoration(colorBoundsAlternate, color0Alternate, color1Alternate, ucolor.g);
	}
}

vec4 voxelIdxToColor(in uvec3 ucolor, in vec2 vis) {
	if (rgbMode == 1u) {
		return color_red(ucolor);
	}
	if (rgbMode == 2u) {
		return color_green(ucolor);
	}
	if (rgbMode == 3u) {
		// Only support HandE mode for now :
		// return voxelIdxToColor_2channel(ucolor);
		if (vis.y > .5f) {
			return color_green(ucolor);
		}
		/*if (vis.x > .5f ){
			return color_red(ucolor);
		}*/
		return vec4(.0,.0,.0,1.);
		// Still color it, with green channel :
		vec4 fincolor = color_green(ucolor);
		// set alpha to 0 and return :
		fincolor.a = .0f;
		return fincolor;
	}
	if (rgbMode == 4u) {
		return voxelIdxToColor_2channel(ucolor);
	}
}

//////////////////////////////////// COLOR FUNCTIONS ////////////////////////////////////

