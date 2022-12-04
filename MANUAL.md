# Manual 

[fig_general]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/general.svg "general" 
[fig_open_image]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/openImage.png "Open image UI" 
[fig_open_image_mesh]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/openImageMesh.png "Open image, mesh opening part UI" 
[fig_open_image_cage]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/openImageCage.png "Open image, cage opening part UI" 
[fig_open_mesh]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/openMesh.png "Open mesh UI" 
[fig_cutting_planes]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/cuttingPlanes.png "Cutting planes UI" 
[fig_display]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/display.png "Display pannel UI" 
[fig_multi_view]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/multiView.png "MultiView button" 
[fig_alpha_low]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/alphaLow.png "general" 
[fig_alpha_high]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/alphaHigh.png "general" 
[fig_unsegmented_mode]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/unsegmentedMode.png "Unsegmented mode" 
[fig_segmented_mode]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/segmentedMode.png "Segmented mode" 
[fig_color_unit]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/colorUnit.png "Color unit" 
[fig_color_unit_advanced]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/colorUnitAdvanced.png "Color unit advanced" 
[fig_color_unit_menu]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/colorUnitMenu.png "Color unit menu" 
[fig_2D_view]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/2Dview.png "2D view menu" 
[fig_all_tools]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/allTools.png "All tools" 
[fig_3D_manipulator]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/3Dmanipulator.png "3D manipulator" 
[fig_none_tool]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/noneTool.png "None tool" 
[fig_move_tool]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/moveTool.png "Move tool" 
[fig_direct_tool]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/directTool.png "Direct tool" 
[fig_ARAP_tool]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/ARAPTool.png "ARAP tool" 
[fig_slice_tool]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/sliceTool.png "Sice tool" 
[fig_marker_tool]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/markerTool.png "Marker tool" 
[fig_export_image]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/exportImage.png "Export image UI" 
[fig_combobox]: https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/combobox.png "Object selection combobox" 

This document explain how to use the software.
In depth explanation of the code is located in the Doxygen documentation, see [README.md](./README.md).
To build the project see [BUILD.md](./BUILD.md).

## General concept

Before diving into the manual itself, it is important to understand the core concepts behind this application to better grasp some design choices.
This section is a quick summary of the __scientific publication__ resulting from this work, that can be found [here](https://diglib.eg.org/bitstream/handle/10.2312/vcbm20221191/093-097.pdf?sequence=1&isAllowed=y).

This software interactively visualise and deform 3D images.
The visualisation is based on a GPU raycasting process guided by a *Tetrahedral Mesh* extracted from the image, called *TM* for the rest of this document.
By deforming the *TM* it is possible to deform the image visualisation in real-time.
However, applying complex deformation methods to a *Tetrahedral Mesh* is very costly, so we use another deformation structure: a *cage*.
A *cage* is a 3D surface mesh that can propagate its deformations to the *TM* using *cage coordinates*.
By manipulating this surface mesh instead of the original *TM*, it is now possible to apply more complexe but very intuitive deformations in real-time, like As Rigid As Possible (*ARAP*) deformations.

To summaryze this software interacts with three main concepts, the 3D image itself that contain the data, the *TM* used for visualisation, and the *cage* used to deform the later.
All __editions__ performed by the user are applied __on the *cage*__.

> __IMPORTANT:__ The software do not curently embed a way to generate *cages* or *TM*, we recommend the package `3D Mesh generation` from the `CGAL library`, available [here](https://doc.cgal.org/latest/Mesh_3/index.html).

> __Note__: A *Cage* can deform any mesh, a regular *Surface Mesh*, a *Tetrahedral Mesh*, or even another *Cage*.

#### Definitions

- *TM*: Tetrahedral mesh used for the image visualisation.
- *Cage*: Surface mesh used to deform the *TM*. This is the only structure the user can interact with.
- *Slice*: A 3D image is a stack of multiple 2D images. We call *slice* one of these 2D images.
- *Grid*: We call *Voxel Grid* or *Grid* the object composed of a 3D image with its attached *TM* and *Cage*. 

### User interface

<img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/general.svg " width="768">

Here is the name of each global sections of the UI:
- ![#FF6347](https://placehold.co/15x15/B3B3B3/B3B3B3.png) `Menu`
- ![#1589F0](https://placehold.co/15x15/FFFF4C/FFFF4C.png) `Toolbar`
- ![#f03c15](https://placehold.co/15x15/007880/007880.png) `Left section`
- ![#1589F0](https://placehold.co/15x15/CD9B5F/CD9B5F.png) `Viewport`
- ![#c5f015](https://placehold.co/15x15/54FF00/54FF00.png) `Bottom section`

## Opening

### Open image

To open a 3D image use the <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/resources/icons/open.svg" width="16"> button in the [tool bar](#user-interface) .
Use the `Presets` buttons to automatically fill this form to open either a Mouse Brain Atlas or a mouse head MRI acquisition, this is a good way to check if the software works correctly on your machine.
Use the `Name` line edit to change the image's name that will be used throughout the software.

> __IMPROVEMENT:__ If an image is loaded with a name that aleady exist the software will not work properly. Their is no verification.

#### Select the image file

<img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/openImage.png " width="512">

Use the `Select image file` button to select the image file to load.
Supported format are .tif and .ome.tiff.
Available options are:
- `Subsample`: load a subsampled version of the image. The only interpolation algorithm available is nearest neighbors. For a subsample value of 2, only one pixel out of two are loaded.
- `Segmented`: set to `True` if the image is segmented. In this case a random color map is automatically computed, see section [Segmented mode](#color-control-segmented-mode)
- `Position`: set the location of the minimum point of the loading images's bounding box at. __WARNING__: The position do not take into account the voxel size. If you want to place the image at the position of the [2, 2, 2] pixel, you need to manually multiply this value per the (voxel_size * subsample).
- `Voxel size`: set the real size of a voxel. Can be in any unit (mm, cm,...) as long as it is the same for all opened images. The voxel size is internally multiplied by the subsample to keep a consistant image size.

Once an image has been selected you can optionnally load a *Tetrahedral Mesh* for the visualisation, and/or a *cage* for the deformation, see [concept](#general-concept).

#### Select the Tetrahedral Mesh

<img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/openImageMesh.png" width="512">

To load a *Tetrahedral Mesh*, also called *TM*, tick the `Mesh` checkbox section, and click on `Select mesh file` button.
The *Tetrahedral Mesh* is used for better visualisation and efficient deformation, see [concept](#general-concept), 
The supported format is [.mesh](#mesh).

> __Note__: If their is no *TM* selected, a generic regular *Tetrahedral Mesh* grid is generated to allow data visualisation, see [concept](#general-concept). However, if there is no *cage* available the __edition is not possible__.

#### Select the Cage

<img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/openImageCage.png" width="512">

To load a *Cage* directly linked to the previously loaded *TM*, check the `Cage` checkbox section, and click on `Select cage file`.
A *cage* is a closed surfacique mesh that can transfer its deformation to the *TM* using *cage coordinates*, see [concept](#general-concept).
The supported format is [.off](#off).
Available options are:
- `Cage type` radio buttons: the *cage* can use different types of *cage coordinates* to deform the *Tetrahedral Mesh*. The MVC<sup>1</sup> deform the *TM* whatever the *cage*. The Green<sup>2</sup> coordinates can deform the *TM* only if the *cage* entirely englobes the *TM*, but yields better deformations.

> __Note__: It is also possible to open a *cage* after the image loading process through the [Menu](#user-interface) and `File>Open Mesh`.

> __IMPROVEMENT__: Automatically generate a regular *cage* grid if no mesh is seleted to allow edition.

<sup>1</sup>FLOATER M. S.: Mean value coordinates. Computer aided geometric design 20, 1 (2003), 19-27.

<sup>2</sup>U T., SCHAEFER S., WARREN J.: Mean value coordinates for closed triangular meshes. In ACM Siggraph 2005 Papers. 2005, pp. 561-566.

### Open Mesh/Cage

![alt text][fig_open_mesh]

To open a mesh go to the [Menu](#user-interface) and `File>Open Mesh`.
Click on `Choose` button to select the mesh file to load.
The supported format is [.off](#off).

It is also possible to load a mesh as a *Cage*, a *Cage* can deform any Mesh, a regular *Surfacique Mesh*, a *Tetrahedral Mesh*, or even another *Cage*.
To load a mesh as a *Cage*, tick the `Cage` checkbox section, choose the *Cage* type *MVC* or *Green*, see [Select the Cage](#select-the-cage), and choose in the combo box which mesh the *Cage* will deform.

### Open graph

To open a graph go to the [Menu](#user-interface) `File>Open Graph`.
The supported format is [.graph](#graph), which is a custom format very similar to off format.

## Visualisation

### 3D view

The software provides real-time 3D visualisation of the deformed image.

#### Naviguation

The 3D view naviguation shortkeys are:
- <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/left-click.png" width="16"> + move: rotate the camera
- <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/right-click.png" width="16"> + move: translate the camera
- Wheel: zoom in/out
- A: display the x/y/z world axis 

<img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/combobox.png " width="512">

To center the Camera on an Object, select it on the large combo box under the [tool bar](#user-interface).

#### Cutting planes

![alt text][fig_cutting_planes]

<img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/cutting_planes.gif" width="256">

Cutting planes are three axis aligned planes that hide the scene part that reside on one side of the plane, very usefull to explore inner parts of the data.
Cutting planes are computed from the scene bounding box.
To change the cutting planes position, go to the `Cutting planes` section on the [left pannel](#user-interface).
Available options from left to right are:
- `x, y and z` sliders: to adjust the position of each plane
- `Invert` button: to invert the cutting plane orientation
- `Checkbox`: to activate/deactivate the cutting plane

> __Bug?__: An object edition that change the scene bounding box, like a translation, will change the cutting planes positions without changing the actual sliders value.

#### Display management

![alt text][fig_display]

To activate/deactivate the display of either the *Grids*, *Cages* or *Tetrahedral Meshes*, go to the `Display` section on the [left pannel](#user-interface).

> __LIMITATION__: The display toggle affects all the opened objects, there is no interface for *per object* display management. This is an interface issue only, the code is ready for this feature.

#### Alpha blending

![alt text][fig_multi_view]

If you have multiple images opened, you can enable the *MultiView* (MView on the button) option in the `Display` section on the [left pannel](#user-interface), this enable the use of the alpha slider.
By changing the slider value the user can change the transparency, allowing the visualisation of two images simulaneously.

<img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/alphaComp.png " width="512">

> __LIMITATION__: There is a big issue with this feature, see ISSUES.

#### Color control: unsegmented mode

![alt text][fig_unsegmented_mode]

The software internally map each pixel value of the image with a color for visualisation.
To control this color map there is two modes available, the *segmented* and *unsegmented* modes, that can be acceded throught the [bottom pannel](#user-interface).
To show/hide the bottom pannel go to the [Menu](#user-interface) and tick `Windows>Show/hide color control window`.

To customize the colormap of unsegmented images, like MRI or lightsheet acquisitions, use the *unsegmented mode*.
Available options are:
- Double slider: change the value range to display.
- Color buttons at each side of the slider: change the color for the min and max values of the image.
- `Change min/max` button : set the min and max values for the colormap. As an example, if an image has a 0 min value with a red color and a 10 000 max value with a blue color, all values between 0 and 10 will have a color very close to red. However if you set the colormap min and max to 0 and 10, all values in this range will have well spread colors.
- Combobox at right of the slider: predefined colormap. The "User colors" apply the color map defined by the two color buttons, "GreyScale" apply a shade of grey and HSV to RGB change the color representation.

#### Color control: segmented mode

![alt text][fig_segmented_mode]

To customize colormap of segmented images like atlases, use the *segmented mode*.
This mode allows to control display of each image value.

![alt text][fig_color_unit]

The bottom section contains *color units*, illustrated above, each of them allows to control display of its associated value.
Available options are:
- Spinbox at the bottom: the value associated with the unit.
- Central color button: assign a color for the value.
- Checkbox left to the color button: choose to apply this color or not. If it is unchecked, the color from *unsegmented* mode is applied.
- Eye button right to the color button: choose to hide/show this value.
To add a unit use the "+" button at the most right of the color units.
Note, if you pressed "Segmented" during the [image loading screen](#open-image), this mode is selected by default and ranges are automaticaly computed.

![alt text][fig_color_unit_advanced]

By default all units are associated with an unique value, however, for greater flexibility it is also possible to assign a range with the advanced mode, illustrated above.
The advanced mode display new options:
- A second spinbox: set the maximum value of the range.
- Cross button: delete the unit.
- Left and right arrow button: move the unit to the left and right. Ranges are applied from left to right, allowing complexe interactions. For example it is possible to assign all values from 1-100 with a color to the very left, and then easily add multiple ranges from 10-20, 30-40 and 70-80 for example. Without this feature the user must add ranges 1-10 10-20 20-30 30-40 40-70 70-80 80-100 to achieve the same result. However if the 1-100 unit is located to the very __right__, all values will have the same color because this will be the last range applied.

![alt text][fig_color_unit_menu]

The menu section at the left, illustrated above, contains various options.
Available options are from top to bottom:
- Load a colormap file, which is a json file, see [colormap format](#colormap).
- Save the current colormap.
- Delete all color units and add the default unit. The default unit associates all values with a color, but it is unchecked, which means that the segmented mode is deactivate because the unsegmented colormap is applied to all values.
- Show/hide all the units.
- Check/uncheck all the units.
- Automatically generates a color map which associate a random color for each value of the image. IMPORTANT: the maximum number of units if fixed to 500. If the loaded image has more than 500 values, if the image is not segmented for example, this feature will simply add the default unit.
- Show/hide advanced mode.

> __BUG?__: The *segmented* mode is always used, and the *unsegmented* mode color is applied for all units with the color mode unticked. However the display range of the *unsegmented* mode is still applied. Originaly, the unsegmented mode only was available which cause this strange interaction. However this mode is not enought to precisely handle segmented data, and the segmented mode alone is not enought either to handle unsegmented data.

### 2D view

To visualise the deformed image in a 2D view click on the "Dual View" <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/resources/icons/dualScreen.svg" width="16"> button in the [tool bar](#user-interface).
Is it possible to visualise two objects on top of each other, which is very usefull for registration problems.
The 2D view will be automatically updated each time the mouse is released during an image edition.

![alt text][fig_2D_view]

Visualisation parameters can be customized using the pannel at the left of the [bottom section](#user-interface).
Available options are from top to bottom:
- Button <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/resources/icons/link.svg" width="16">: link the visible image in the 2D view with the cutting plane. This way the 2D and 3D views are synchronized.
- Slider: select which image slice to display.
- Combobox `Back`: choose which image to display on back.
- Combobox `Front`: choose which image to display on front.
- Checkbox: choose to display or hide front/hide image.
- Spinbox: choose the transparency of the front/back image. The value range is [0, 255].
- `Subsample` spinbox: choose a subsample to apply to the final image. It is very important to limit the computation time of each image.
- `Side` buttons: change which axis to visualise. If the link button is toggled, it also indicate which cutting plane is linked.
- `Mirror` buttons: mirror the image on X or Y.
- `Resolution` spinbox: Set the image resolution to fit the front or back object. For example if we have a brain atlas on top of an MRI acquisition, the brain atlas has a much larger precision. To ensure a real time visualisation we can set the resolution to back to keep a low resolution. To inspect the result at full resolution we can set the resolution to Front.

> __IMPROVEMENT__: Currently the 2D view update is triggered by the mouse right button release during edition only. A "real time" mode could be usefull, where the 2D view is updated when the mouse is moving. The latter is not available because the image computation algorithm is currently too long to be real time.
> __BUG__: The 2D view do not works with a subsampled image.

## Edition

The software allows to deform images using various tools.
These tools works __on *cages* only__, while they could be directly used on the *TM* thanks to the code genericity, this option is deactivated to avoid editing the *TM* by mistake.

### 3D manipulator

<img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/3Dmanipulator.png " width="256">

Tools often use a 3D manipulator to apply deformation.
Available shortcuts are from top to bottom:
- <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/left-click.png" width="16"> or <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/right-click.png" width="16"> + move on axis: translate
- <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/left-click.png" width="16"> or <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/right-click.png" width="16"> + move on circles: rotate
- <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/left-click.png" width="16"> or <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/right-click.png" width="16"> + move on small spheres: scale
- Wheel: changes the manipulator to a 3D sphere that moves along the plane of the current view.

### Tools

![alt text][fig_all_tools]

Each tool edit the currently selected object using the combobox behind the [tool bar](#user-interface).
Each tool has a short help that describes all the shortcuts on the [left section](#user-interface).
In the same section, some tools also comes with options available via buttons.

__IMPROVEMENT__: Tools are independant objects in the code that are deleted and re-created when exited, so their states are deleted. To "remember" the state of a closed tool it can be usefull to keep the previous tools. This is easy to do in the code.

#### None tool

![alt text][fig_none_tool]

Use this tool for visualisation only, when no edition is needed.

#### Move tool

![alt text][fig_move_tool]

To translate, rotate or scale an image use the `move tool`.
Once activated, this tool shows a standard 3D manipulator to deform the image.
Available options are:
- Even <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/resources/icons/even.svg" width="16">: to apply an even scale on both X, Y and Z axis. If this option is toggled and a scaled is applied on one axis, the same scale will be applied on the 2 other axis.
- Link <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/resources/icons/link.svg" width="16">: allow to link or unlink the *cage* to its *TM*. If this option is toggled the *cage* will deform the *TM* which is the default behavior. However if the user untoggle this option he is able to move the *cage* without moving the *TM*. This is usefull to adjust the placement of the *cage* according to the *TM*.
- Reset <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/resources/icons/reset.svg" width="16">: reset the view and the size of the 3D manipulator, this is usefull when applying large scaling operation and the 3D manipulator get off the screen.

__IMPROVEMENT__: This tool cannot be used directly on the *TM*, therefore it is impossible de translate, rotate or scale an image if no *cage* is loaded.
__BUG__: The even, link or reset button are desynchronized sometime.

#### Direct tool

![alt text][fig_direct_tool]

To move independantly each vertex of a mesh, use the `direct tool`.
Once activated, each vertex can be grabbed and moved arround freely.

This is the only tool than can be used on the *TM* directly.

#### ARAP tool

![alt text][fig_ARAP_tool]

To deform entire portion of a mesh surface, use the `ARAP tool`.
Once activated, vertices can be marked as fixed, and any vertices subset can be moved while keeping the fixed vertices unchanged.
All vertices that are not marked move automatically to achieve a very smooth and intuitive deformation.
This tool is extensively used in a registration context when some parts of the images are already correctly aligned, but some other parts need adjustements for a better alignement.
Available shortcuts are:
- Maj+alt + <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/left-click.png" width="16"> + move: mark vertices as fixed.
- Maj + <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/left-click.png" width="16"> + move: mark vertices as moving.
- Maj+ctrl + <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/left-click.png" width="16"> + move: unmark vertices.
- Maj + <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/doc/images/right-click.png" width="16">: show 3D manipulator to move the vertices marked as moving.

#### Slice tool

![alt text][fig_slice_tool]

This tool allows to apply *ARAP* deformation on mesh slices only.
Use the X, Y and Y buttons to choose which axis to move.
To move an axis, use the cutting plane section.
A 3D manipulator is automatically displayed to move the slice.
All the green vertices will be moved, all the grey vertices will remain fixed and all the red vertices will move automatically.
Available shortcuts are:
- "+ or -": adjust the automatically moving part (grey vertices).
- Ctrl+"+ or -": adjust the moving portion (green points).
- P: project the green points on another mesh using APSS projection.__WARNING__: green points are projected on the second mesh opened. For this feature to works, the edited mesh should be the first mesh opened, and a second mesh should be opened too.

__IMPROVEMENT__: not really usable for now because fixed points arent keeped when changing axis. This is an experimental tool.

#### Marker tool

![alt text][fig_marker_tool]

This tool move a selected vertex to match the placed 3D point, and deform the mesh accordingly.
Multiple vertices can be matched, and the tool will compute the deformation to best match these constraints.
First select a vertice, and then select a point in the 3D viewport by pointing it with the mouse and press the key Q.
The point will be placed at the surface of the pointed grid.

### History

The software include an history to undo <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/resources/icons/undo.svg" width="16">/redo<img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/resources/icons/redo.svg" width="16"> editions, these options are available on [tool bar](#user-interface).
To go back to the initial state of the mesh use the `reset`<img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/resources/icons/reset.svg" width="16"> button.

## Save

### Save cage

To save the selected cage, go to the [Menu](#user-interface) and `File>Save...`, then choose a location.

### Re-open cage

Once a *cage* has been edited and saved, it can be reopened again.
__First the original *cage* should be opened__, because the edited *cage* do not contain its resting position which is required to compute defomations. 
So open the original *cage* using the <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/resources/icons/open.svg" width="16"> button in the [tool bar](#user-interface) or the open mesh function on the [Menu](#user-interface) and `File>Open Mesh`.
Then go to the [Menu](#user-interface) `Edit>Apply cage`.
Click on the `Choose` button and select the edited *cage* file.
Use the combobox to select the original *cage*.

__IMPROVEMENT__: the cage saving process could includes the resting cage position.

### Export image

![alt text][fig_export_image]

To export the resulting image use the `Export` <img src="https://gitlab.com/thibaulltt/visualisation/raw/develop/resources/icons/save.svg" width="16"> button in the [tool bar](#user-interface).
Available options from top to bottom are:
- Combobox: choose whith image to export.
- `Reset` button: reset all paramameters of this window.
- `Use colormap` checkbox : use the currently loaded colormap, otherwise the resulting image will be a shade of grey.
- `Export at original resolution` checkbox:  prefill voxel and image size to export with the same voxel size as the original image. This is particularly usefull when an image has been loaded with a subsample.
- `BBox min` triple spinbox: set the image portion you want to export. A visualisation is available on the viewport.
- `Voxel size`: set the voxel size of the image to be exported. If this value is changed the image size will be automatically updated.
- `Image size`: set the image size of the image to be exported. If this value is changed the voxel size will be automatically updated.

__IMPROVEMENT__: The voxel size and the position are not writed into the TIFF file.

## File formats

Here is a list of the file fomats interacting with the software.

### TIFF

This is the main format handled by the software.
All images to be loaded should be in this format.

### Mesh

This format is used for *Tetrahedral Meshes*.

### Off

This format is used for Surface Meshes.
> __IMPORTANT__: it only handles simple format with vertices then faces.
Other cases like faces with 4 vertices or materials are not handled.
For example off format directly from Blender need to be edited manually for a proper opening.

> __IMPROVEMENT__: Handle off files from Blender.

### Colormap

This is a custom format to save and load color map.
You have an example in: resources/data/colorMapPaper.json.
