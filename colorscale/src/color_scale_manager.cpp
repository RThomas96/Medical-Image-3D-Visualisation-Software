#include "../include/color_scale_manager.hpp"

namespace Color {

ColorScaleManager::ColorScaleManager() {
	this->color_scale_user.clear();
	this->greyscale = nullptr;
	this->hsv2rgb = nullptr;
	this->init_default_color_scales();
}

ColorScaleManager::~ColorScaleManager() {
	this->color_scale_user.clear();
	this->greyscale = nullptr;
	this->hsv2rgb = nullptr;
}

void ColorScaleManager::init_default_color_scales() {
	this->greyscale = std::make_shared<SimpleGradientColorScale>(glm::vec3(.0f,.0f,.0f), glm::vec3(1.f,1.f,1.f));
	this->hsv2rgb = std::make_shared<ColorScaleFunctor>([](float f) -> glm::vec3 {
		glm::vec3 hsv = glm::vec3 (f, 1., 1.);
		hsv.x = glm::mod( 100.0f + hsv.x, 1.0f ); // Ensure [0,1[
		float   HueSlice = 6.0f * hsv.x; // In [0,6[
		float   HueSliceInteger = floor( HueSlice );
		float   HueSliceInterpolant = HueSlice - HueSliceInteger; // In [0,1[ for each hue slice
		glm::vec3 TempRGB = glm::vec3(   hsv.z * (1.0 - hsv.y), hsv.z * (1.0 - hsv.y * HueSliceInterpolant), hsv.z * (1.0f - hsv.y * (1.0f - HueSliceInterpolant)) );
		float   IsOddSlice = glm::mod( HueSliceInteger, 2.0f ); // 0 if even (slices 0, 2, 4), 1 if odd (slices 1, 3, 5)
		float   ThreeSliceSelector = 0.5 * (HueSliceInteger - IsOddSlice); // (0, 1, 2) corresponding to slices (0, 2, 4) and (1, 3, 5)
		glm::vec3    ScrollingRGBForEvenSlices = glm::vec3( hsv.z, TempRGB.z, TempRGB.x );           // (V, Temp Blue, Temp Red) for even slices (0, 2, 4)
		glm::vec3    ScrollingRGBForOddSlices = glm::vec3( TempRGB.y, hsv.z, TempRGB.x );  // (Temp Green, V, Temp Red) for odd slices (1, 3, 5)
		glm::vec3    ScrollingRGB = mix( ScrollingRGBForEvenSlices, ScrollingRGBForOddSlices, IsOddSlice );
		float   IsNotFirstSlice = glm::clamp( ThreeSliceSelector, 0.0f, 1.0f );                   // 1 if NOT the first slice (true for slices 1 and 2)
		float   IsNotSecondSlice = glm::clamp( ThreeSliceSelector-1.0f, 0.0f,1.f );              // 1 if NOT the first or second slice (true only for slice 2)
		return glm::vec3(glm::mix(glm::vec3(ScrollingRGB), glm::mix(
				glm::vec3(ScrollingRGB.z, ScrollingRGB.x, ScrollingRGB.y),
				glm::vec3(ScrollingRGB.y, ScrollingRGB.z, ScrollingRGB.x),
				IsNotSecondSlice),IsNotFirstSlice));    // Make the RGB rotate right depending on final slice index
	});

	this->greyscale->setName("Greyscale");
	this->hsv2rgb->setName("HSV to RGB");
}

}
