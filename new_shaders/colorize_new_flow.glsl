#line 1001
//////////////////////////////////// COLOR FUNCTIONS ////////////////////////////////////

// This shader is only included in other shaders to have
// similar coloring functions ! Should not be compiled alone.
// Note : is included when encoutering "#pragma include_color_shader;"

#ifndef MAIN_SHADER_UNIT
layout(std140) struct colorChannelAttributes {
	int isVisible;			// /* Aligned as : ui64 */ Is this color channel enabled/visible ?
	vec2 visibleBounds;		// /* Aligned as : vec4	*/ The bounds of the visible values
	sampler2D colorScale;	// /* Aligned as : ui64 */ The color scale for the values		// NOTE : CAN ONLY BE SET IF BINDLESS TEXTURES ARE AVAILABLE !!!
	vec2 colorScaleBounds;	// /* Aligned as : vec4 */ The value bounds for the color scale
};

/*
The color channel attributes are laid out in this way :
  - main color channel (whatever the user decides it will be)
  - red color channel
  - green color channel
  - blue color channel
*/
uniform colorChannelAttributes colorChannels[4];
#endif

/// Determines if the color channel is visible or not, given a certain value.
bool isColorChannelVisible(in uint index, in uint value) {
	return (colorChannels[index].isVisible > 0) &&
			( uint(colorChannels[index].visibleBounds.x) >= value &&
			uint(colorChannels[index].visibleBounds.y) <= value );
}

vec4 colorizeFragmentSingleChannel(in uint color_channel_index, in uint value) {
	// Since we first check if the channel is visible, we only need to colorize it.
	// We're guaranteed to not only have the color channel activated, but also that
	// the given value is _always_ within the visible bounds.

	vec2 colorScaleBounds = colorChannels[color_channel_index].colorScaleBounds;
	float normalized_value = float(value - colorScaleBounds.x) / float(colorScaleBounds.y - colorScaleBounds.x);
	return texture(colorChannels[color_channel_index].colorScale, normalized_value);
}

vec4 fragmentEvaluationSingleChannel(in uvec3 color, in uint mainValue) {
	// Handy predefined uint for all color channels :
	uint main = 0, red = 1, green = 2, blue = 3;
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
	} else {
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
}
