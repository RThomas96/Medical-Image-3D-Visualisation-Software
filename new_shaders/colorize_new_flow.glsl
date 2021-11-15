#line 1001
//////////////////////////////////// COLOR FUNCTIONS ////////////////////////////////////

// This shader is only included in other shaders to have
// similar coloring functions ! Should not be compiled alone.
// Note : is included when encoutering "#pragma include_color_shader;"

#ifndef MAIN_SHADER_UNIT
// The structure which defines every attributes for the color channels.
struct colorChannelAttributes {
	uint isVisible;			// /*align : ui64*/ Is this color channel enabled/visible ?
	uint colorScaleIndex;	// /*align : ui64*/ The color channel to choose
	uvec2 visibleBounds;	// /*align : vec4*/ The bounds of the visible values
	uvec2 colorScaleBounds;	// /*align : vec4*/ The value bounds for the color scale
};

uniform uint mainChannelIndex;						// The index of the main channel in the voxel data
uniform sampler1D colorScales[4];					// All the color scales available (all encoded as 1D textures)
layout(std140) uniform ColorBlock {
	colorChannelAttributes attributes[4];	// Color attributes laid out in this way : [ main, R, G, B ]
} colorChannels;
#endif

/// Determines if the color channel is visible or not, given a certain value for the channel.
bool isColorChannelVisible(in uint index, in uint value) {
	return (colorChannels.attributes[index].isVisible > 0u) &&
			(
				value >= uint(colorChannels.attributes[index].visibleBounds.x)
				&& value <= uint(colorChannels.attributes[index].visibleBounds.y)
			);
}

vec4 colorizeFragmentSingleChannel(in uint color_channel_index, in uint value) {
	// Since we first check if the channel is visible, we only need to colorize it.
	// We're guaranteed to not only have the color channel activated, but also that
	// the given value is _always_ within the visible bounds.

	vec2 csB = colorChannels.attributes[color_channel_index].colorScaleBounds;
	float normalized_value = (float(value) - float(csB.x)) / (float(csB.y) - float(csB.x));
	uint colorScaleIndex = colorChannels.attributes[color_channel_index].colorScaleIndex;
	return texture(colorScales[colorScaleIndex], normalized_value);
}

vec4 fragmentEvaluationSingleChannel(in uvec3 color) {
	// Get the main color's channel :
	uint mainValue;
	if (mainChannelIndex == 0u) {
		mainValue = color.r;
	} else if (mainChannelIndex == 1u) {
		mainValue = color.g;
	} else if (mainChannelIndex == 2u) {
		mainValue = color.b;
	}

	// Handy predefined uint for all color channels in uniforms :
	uint main = 0u, red = 1u, green = 2u, blue = 3u;
	// Precompute the visibility of all color channels :
	bool visible_main  = isColorChannelVisible(main, mainValue);
	bool visible_red   = isColorChannelVisible(red, color.r);
	bool visible_green = isColorChannelVisible(green, color.g);
	bool visible_blue  = isColorChannelVisible(blue, color.b);

	// First, check if main is visible :
	if (visible_main) {
		if (mainValue >= color.r && mainValue >= color.g && mainValue >= color.b) {
			return colorizeFragmentSingleChannel(main, mainValue);
		}
	}
	// Here, no 'else' condition since we don't want to end if the second condition
	// for the main channel fails ! It created 'noise' where the values of the main
	// channel are not above other channels.
	if (visible_red) {
		if (visible_green) {
			if (color.r >= color.g) {
				if (visible_blue) {
					if (color.r > color.b) {
						return colorizeFragmentSingleChannel(red, color.r);
					} else {
						return colorizeFragmentSingleChannel(blue, color.b);
					}
				} else {
					return colorizeFragmentSingleChannel(red, color.r);
				}
			} else {
				if (visible_blue) {
					if (color.g > color.b) {
						return colorizeFragmentSingleChannel(green, color.g);
					} else {
						return colorizeFragmentSingleChannel(blue, color.b);
					}
				} else {
					return colorizeFragmentSingleChannel(green, color.g);
				}
			}
		} else {
			if (visible_blue) {
				if (color.r > color.b) {
					return colorizeFragmentSingleChannel(red, color.r);
				} else {
					return colorizeFragmentSingleChannel(blue, color.b);
				}
			} else {
				return colorizeFragmentSingleChannel(red, color.r);
			}
		}
	} else {
		if (visible_green) {
			if (visible_blue) {
				if (color.g > color.b) {
					return colorizeFragmentSingleChannel(green, color.g);
				} else {
					return colorizeFragmentSingleChannel(blue, color.b);
				}
			} else {
				return colorizeFragmentSingleChannel(green, color.g);
			}
		} else {
			if (visible_blue) {
				return colorizeFragmentSingleChannel(blue, color.b);
			} else {
				return vec4(.0, .0, .0, .0);
			}
		}
	}
}
