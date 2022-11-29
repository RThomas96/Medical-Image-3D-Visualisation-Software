# Manual 
This document is a manual aim for for both developpers and users.
For the user this manual explain how to use the software.
For developper it explain where the code is located for each feature, the in depth explanations of the code are directly in it, in the Doxygen documentation.
To compile the code see [TODO].
For the FAQ see [TODO].

## General

Explain why this is important to select the CAGE.

## Glossary

image slice:

### User interface

## Opening

### Open image
To open a 3D image use the [Open] button in the tool bar.
Some presets are available to automatically fill this form and open a Mouse Brain Atlas or a mouse head MRI.
As these files as been tested this is a good way to check if the software behave correctly on your machine.
You can also set a name for the image that will be used throughout the software.

#### Select the image file
To select the image file to load, click on [Select image file].
Supported format are .tif and .ome.tiff.
Available options are:
- Subsample: allow to load a subsampled version of the image. The only subsample option is nearest neighbors. For a subsample value of 2, only one pixel out of two are loaded.
- Segmented: check if the image is segmented. In this case a random color map is automatically computed, see section [Segmented mode](#Segmented mode)
- Position: place the minimum point of the loading images's bounding box at the given position in the scene. The position do not take into account the voxel size or the subsample. If you want to place the image at the position of the [2, 2, 2] pixel, you need to manually multiply this value per the voxel size.
- Voxel size: the real size of an image voxel. Can be in any unit (mm, cm,...) as long as it is the same for all opened images. The voxel size is internally multiplied by the subsample to keep a consistant image size.

Once an image has been selected you can optionnally load a Tetrahedral Mesh for the visualisation, and/or a cage for the deformation, see [concept](#General).

#### Select the Tetrahedral Mesh
To load a Tetrahedral Mesh, also called TM, check the [Mesh] checkbox section, and click on [Select mesh file].
The Tetrahedral is used for better visualisation and efficient deformation, see [concept](#General), 
The supported format is [.mesh](#Mesh).

If no TM is selected, a generic regular Tetrahedral Mesh grid is generated to allow data visualisation, see [concept](#General).

#### Select the Cage
To load a Cage directly linked to the previously loaded TM, check the [Cage] checkbox section, and click on [Select cage file].
A cage is a closed surfacique mesh that can transfer its deformation to the TM using cage coordinates allowing As Rigid As Possible deformations, see [concept](#General).
The supported format is [.off](### Off).
Available options are:
- Cage type: the cage can use different types of cage coordinates to deform the Tetrahedral Mesh. The MVC<sup>1</sup> allow to deform the TM whatever the cage. The Green<sup>2</sup> coordinates can deform the TM only if the cage englobe the TM.

Note that this cage opening operation can also be performed afterward through the [Open Mesh](### Open Mesh) menu.

<sup>1</sup>FLOATER M. S.: Mean value coordinates. Computer aided geometric design 20, 1 (2003), 19-27.

<sup>2</sup>U T., SCHAEFER S., WARREN J.: Mean value coordinates for closed triangular meshes. In ACM Siggraph 2005 Papers. 2005, pp. 561-566.

### Open Mesh
To open a mesh go to the [File] menu, and click on [Open Mesh].
To select the mesh file to load, click on [Choose].
The supported format is [.off](### Off).

It is also possible to load a Mesh as a cage, a Cage can deform any type of Mesh, a regular Surfacique Mesh, a Tetrahedral Mesh, or even another Cage.
To load a Mesh a a cage, check the [Cage] checkbox section, choose the cage type, and choose in the combo box which mesh the cage will deform.

### Open graph
To open a graph go to the [File] menu, and click on [Open Graph].
The supported format is [.graph](### Graph), which is a custom format.

## Visualisation

### 3D view
The software provide real-time visualisation of the deformed image.

#### Naviguation
The 3D view naviguation shortkeys are:
- Left click + move: rotate the camera
- Right click + move: translate the camera
- Wheel: zoom in/out
- A: display the x/y/z axis 

To center the Camera on an Object, select it on the large combo box under the Tool Bar.

#### Cutting planes
Cutting planes are three axis aligned planes that allows to hide the scene part that reside on one side of the plane, very usefull to explore inner parts of the data.
To change the cutting planes position, go to the [Cutting planes] section on the left pannel.
Available options from left to right are:
- x, y and z sliders: to adjust the position of each plane
- Invert button: to invert the cutting plane orientation.
- Checkbox: to activate/deactivate the cutting plane

Cutting planes are computed from the scene bounding box.
[BUG?] An object edition that change the scene bounding box, like a translation, will change the cutting planes positions without changing the actual sliders values.

#### Display toggle
To activate/deactivate the display of either the Grids, Meshes or the Tetrahedral Meshes, go to the [Display] section on the left pannel.

[LIMITATION] The display toggle affect all the opened objects, there is no interface for per object display management. It is important to note that this is an interface issue only, all the code is ready to welcome this feature.

####Color control: unsegmented mode
For visualisation the software map each pixel value of the image with a color.
This color map can be controlled using two different modes, the segmented and unsegmented mode, that can be acceded throught the bottom pannel.
The bottom pannel can be show/hide from the Windows menu on the top of the software.

Unsegmented mode is designed to customize the colormap for images that are unsegmented, like MRI or lightsheet acquisitions.
Available options are:
- Double slider: change the values to display.
- Color buttons at each side of the slider: change the color for the min and max values of the image.
- Button "Change min/max": set the min and max values for the colormap. As an example, if an image has a 0 min value with a red color and a 10 000 max value with a blue color, all values between 0 and 10 will have a color very close to red. However if you set the colormap min and max to 0 and 10, all values in this range will have well spread colors.
- Combobox right to the slider: predefined colormap. The "User colors" apply the color map defined by the two color buttons, "GreyScale" apply a shade of grey and HSV to RGB change the color representation.

####Color control: segmented mode
Segmented mode is designed to customize the colormap for images that are segmented, like atlases.
This mode allow to control the display of each value.
The main section contain what we call a color unit, each of them allow to control the display of the associated value.
Available options are:
- Spinbox at the bottom: the value associated with the unit
- Central color button: assign a color for the value
- Checkbox left to the color button: choose to apply this color or not. If it is unchecked, the color from the Unsegmented mode is applied.
- Eye button right to the color button: choose to hide/show this value.
It is possible to add a unit using the "+" button at the most right of the main section.
Note, if you pressed "Segmented" during the [.image loading screen](### Open image), this mode is selected by default and ranges are already filled.
By default all units are associated with an unique value, however, for greater flexibility it is also possible to assign a range with the advanced mode.
The advanced mode display new options:
- A second spinbox: set the maximum value of the range.
- Cross button: delete the unit
- Left and right arrow button: move the unit to the left and right. Indeed the ranges are applied from left to right, this allow complexe interactions. For example it is possible to assign all values from 1-100 with a color to the very left, and then easily add multiple ranges from 10-20, 30-40 and 70-80 for example. Without this feature the user must add ranges like 1-10 10-20 20-30 30-40 40-70 70-80 80-100. WARNING: if the 1-100 unit is to the very right, all values will be blue.

The menu section at the left contain various options.
Available options are from top to bottom:
- Load a colormap file, which is a json file, see [.colormap format](### Colormap).
- Save the current colormap.
- Delete all color units and add the default unit. The default unit associate all image values with a random color, but it is unchecked, which means that the segmented mode is deactivate because the unsegmented colormap is applied to all values.
- Show/hide all the units.
- Check/uncheck all the units.
- Automatically generates a color map which associate a random color for each value of the image. IMPORTANT: the maximum number of units if fixed to 500. If the loaded image has more than 500 values, this feature will not work.
- Show/hide advanced mode.

[BUG?] Selecting one or the other mode do not change from one mode to the other which is not intuitive. Instead we always the Segmented mode, and we apply the Unsegmented mode for all units that are unchecked. However the display range of the unsegmented is still applied. This strange interaction is due to the legacy feature, originaly only the unsegmented mode was available, but this mode is not enought to precisely handle segmented data. The segmented mode alone is not enought either.

### 2D view
The software allows to visualise the deformed image in a 2D view.
It can be activated using the "Dual View" button in the tool bar.
The 2D view will be automatically updated each time an object is edited and the mouse is released.
The 2D view parameters can be changed using the pannel at the right of the bottom section.
Available options are from top to bottom:
- Button with link icon: choose to link the image slice visible in the 2D view with the cutting plane.
- Slider: select which image slice to display.
- Combobox back: choose which object to display on back.
- Combobox front: choose which object to display on front.
- Checkbox: choose to display or hide front/hide image.
- Spinbox: choose the transparency of the front/back image. The value range is [0, 255].
- Subsample: choose a subsample to apply to the final image. It is very important to limit the computation time of each image.
- Side: chaneg which axis to visualise. If the link button is toggled, it also indicate which cutting plane is linked.
- Mirror: allow to mirror the image on X or Y.
- Resolution: Set the image resolution to fit the front or the back object. For example if we have a brain atlas on top of a MRI acquisition, the brain atlas has a much larger precision. To ensure a real time visualisation we can set the resolution to back to keep a low resolution. To inspect the result at full resolution we can set the resolution to Front.

[IMPROVEMENT] The real time 2D view update and not only when the mouse is released has been deactivated to avoid slowing down the software. The option is not exposed to the user.
[LIMITATION] Problem with the mixed mode, images has been cached in order to quickly naviguate from slide to slide, therefore they are stored into a 3D array. Changing the resolution change the size of the 3D array and discard all the previous data and this is not how the viewer is designed.

## Edition
The software allow to deform the image using various tools.
It is important to note that these works on cage only.
Due to the generic nature of the code they can also be used directly on the TM, however this option has been deactivate to the user to avoid editing the TM by error.

### 3D manipulation

Each tool that will be presented often present a 3D manipulation to apply deformation.
It can be manipulated using the right or left click while overring the axis to apply.
It is also possible to change the manipulation primitive with the wheel.
Then instead of a 3D guizmo, the operation can be applied using a 3D ball that move on the plane of the current view.

### Tools

Each tool is applied on the currently selected object.
It has a help on the tool section on the left.
Some tools has also some options in the form of buttons.

[IMPROVEMENT] Actually the tools are independant objects that are deleted and re-created, therefore when a tool is exited, all its states are deleted. To "remember" the state of a closed tool it can be usefull to keep the previous tools. This is easy to do in the code.

#### None tool

This tool is used no edion is needed and when the goal is visualisation only.

#### Move tool

This tool is a classic 3D manipulator used to translate, rotate and scale the selected object.
Available options are:
- Even: to apply an even scale on both X, Y and Z axis. If this option is toggled and a scaled is applied on the X axis, the same scale will be applied on the 2 other axis.
- Link: allow to link or unlink the cage to its TM. If this option is toggled the cage will move the TM which is the default behavior. However if the user untoggle this option he is able to move the cage without moving the TM. This is usefull to adjust the placement of the cage according to the TM.
- Reset: reset the view and the size of the 3D manipulator, this is usefull when applying large scaling operation and the object get off the screen.

[IMPROVEMENT] This tool cannot be used directly on the TM while this could be logical.
[BUG] The even, link or reset button can be desynchronized

#### Direct tool

This tool allow to move each vertew independantly.
While activated, all vertices become red, indicating that they can be grabbed.
Each vertice can then be grabbed and moved arround freely.
This is the only tool than can be used on the TM directly.

#### ARAP tool

#### Slice tool

#### Marker tool

### History

## Registration

## Export

## How to add a tool

## Formats

### TIFF

### Mesh

### Off

### Colormap

## TODO

# Quick fix
- Lock all buttons except the select file button
- Automatic load of dx/dy/dz
- Automatic load of position
- WARNING: add a warning for voxel size
- Allow to delete a single object instead of clear the whole scene
- Add name checks
- Interface to change cage, and TetMesh
- Add option to save at specific format like uint16 or whatever
- Ajouter compilation Windows Yan
- Add manual to convert to NIFTI with FIJI
- Generate cage at openning
- Allow to apply global manipulator to TetMesh
- Toggle dual rendering

# Bug
- Can't deactivate rendering of a Grid because of the dual pass
- An object edition that change the scene bounding box, like a translation, will change the cutting planes positions without changing the actual sliders values.

# Idea
- Better display toggle capabilities
- Tool states are not kept
- Better subsample for high resolution images
- NIFTI reader
- The 2D view do not implement HSV to RGB and GrayScale
- The 2D view do not implement slide mode value selection

## Cleaned



./core/geometry/grid.hpp
./core/geometry/grid.cpp
./core/algorithm/ICP.hpp
./core/deformation/AsRigidAsPossible.cpp
./core/deformation/cage_surface_mesh.cpp
./core/deformation/cage_surface_mesh.hpp
./core/drawable/drawable.cpp
./core/drawable/drawable.hpp
./core/drawable/drawable_grid.cpp
./core/drawable/drawable_grid.hpp
./core/drawable/drawable_selection.cpp
./core/drawable/drawable_selection.hpp
./core/drawable/drawable_surface_mesh.cpp
./core/drawable/drawable_surface_mesh.hpp
./core/geometry/base_mesh.cpp
./core/geometry/base_mesh.hpp
./core/geometry/graph_mesh.cpp
./core/geometry/graph_mesh.hpp
./core/geometry/surface_mesh.cpp
./core/geometry/surface_mesh.hpp
./core/geometry/tetrahedral_mesh.cpp
./core/geometry/tetrahedral_mesh.hpp
./core/images/cache.cpp
./core/images/cache.hpp
./core/images/image.cpp
./core/images/image.hpp
./core/interaction/manipulator.cpp
./core/interaction/manipulator.hpp
./core/interaction/mesh_manipulator.cpp
./core/interaction/mesh_manipulator.hpp
./core/utils/GLUtilityMethods.cpp
./core/utils/apss.cpp
./core/utils/apss.hpp
./legacy/image/utils/include/bounding_box.hpp
./legacy/image/utils/include/image_api_common.hpp
./legacy/image/utils/include/local_cache.hpp
./legacy/image/utils/include/read_cache.hpp
./legacy/image/utils/include/threaded_task.hpp
./legacy/image/utils/src/threaded_task.cpp
./legacy/meshes/drawable/shaders.cpp
./legacy/meshes/drawable/shaders.hpp
./qt/3D_viewer.cpp
./qt/3D_viewer.hpp
./qt/UI/chooser.cpp
./qt/UI/chooser.hpp
./qt/UI/color_button.cpp
./qt/UI/color_button.hpp
./qt/UI/color_control.cpp
./qt/UI/color_control.hpp
./qt/UI/deformation_form.cpp
./qt/UI/deformation_form.hpp
./qt/UI/display_pannel.hpp
./qt/UI/form.cpp
./qt/UI/form.hpp
./qt/UI/info_pannel.cpp
./qt/UI/info_pannel.hpp
./qt/UI/open_image_form.cpp
./qt/UI/open_image_form.hpp
./qt/UI/quicksave_mesh.cpp
./qt/UI/quicksave_mesh.hpp
./qt/UI/save_image_form.cpp
./qt/UI/save_image_form.hpp
./qt/UI/tool_pannel.hpp
./qt/cutplane_groupbox.hpp
./qt/double_slider.cpp
./qt/double_slider.hpp
./qt/helper/QActionManager.hpp
./qt/image3D_viewer.cpp
./qt/image3D_viewer.hpp
./qt/legacy/applyCageWidget.cpp
./qt/legacy/applyCageWidget.hpp
./qt/legacy/openMeshWidget.cpp
./qt/legacy/openMeshWidget.hpp
./qt/legacy/viewer_structs.cpp
./qt/legacy/viewer_structs.hpp
./qt/main_widget.cpp
./qt/main_widget.hpp
./qt/planar_viewer.cpp
./qt/planar_viewer.hpp
./qt/range_slider.cpp
./qt/range_slider.hpp
./qt/scene.cpp
./qt/scene.hpp
./qt/scene_control.cpp
./qt/scene_control.hpp
