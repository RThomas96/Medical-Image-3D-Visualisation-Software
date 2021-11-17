#ifndef FEATURES_HPP_
#define FEATURES_HPP_

/**
This file exists to enable or disable certain features of the program.
This selective 'enabling' will be done mostly through macros, or sometimes constexpr variables, and will affect directly
the compiled code, by enabling or disabling certain parts of it.
**/

// #define GRID_DETAILS_ENABLE_TRANSFORMATION_DETAILS
// #define GRID_DETAILS_ENABLE_TRANSFORMATION_PICKER
// #define GRID_DETAILS_ENABLE_SIDE_PANEL

// Enable ellipsis if the name of a grid is too long in the list view :
// #define GRID_LIST_ITEM_ENABLE_ELLIPSIS_TEXT

/// @brief If defined, asks the user for images to load. Otherwise, loads the first 50 images of the 'Blue' dataset.
#define USER_DEFINED_IMAGE_LOADING

//#define LOAD_RED_AND_BLUE_IMAGE_STACKS

/*
#include <renderdoc_app.h>
extern RENDERDOC_API_1_4_1* rdocAPI;
*/

#endif	  // FEATURES_HPP
