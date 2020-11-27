#ifndef FEATURES_HPP_
#define FEATURES_HPP_

/**
This file exists to enable or disable certain features of the program.
This selective 'enabling' will be done mostly through macros, or sometimes constexpr variables, and will affect directly
the compiled code, by enabling or disabling certain parts of it.
**/

/// Allows for the image reader classes to log the file sizes read, or to be read.
// #define IMAGE_READER_LOG_FILE_SIZE

// #define GRID_DETAILS_ENABLE_TRANSFORMATION_DETAILS
// #define GRID_DETAILS_ENABLE_TRANSFORMATION_PICKER
// #define GRID_DETAILS_ENABLE_SIDE_PANEL

// Enable ellipsis if the name of a grid is too long in the list view :
// #define GRID_LIST_ITEM_ENABLE_ELLIPSIS_TEXT

// Features for the discretegrid/bounding box
#define ENABLE_TRANSFORMATIONS
#define ENABLE_BASIC_BB
#define ENABLE_DATA_BB
#define ADJUST_TO_BB
#define ENABLE_BB_TRANSFORM

#define GLM_MAT_BEFORE_VEC
//#define REVERSE_MATRIX_ORDER

#endif // FEATURES_HPP
