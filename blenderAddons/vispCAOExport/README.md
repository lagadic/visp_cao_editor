# Blender Export Addon for ViSP CAD file (.cao)

This Addon was tested with:
- Blender 2.78 on Ubuntu 16.04 (manual installation)
- Blender 2.76 on Ubuntu 16.04 and 14.04 (using apt-get install blender)
- Blender 2.78 on OSX.

## Installation:

  1. Run `python build.py` in project directory. `visp_cao_export.zip` file will be created.
  2. Launch Blender and open `File > User Preferences`.
    - (**Ctrl+Atl+U**) under Linux
    - (**Cmd+,**) under OSX
  3. Click on `Install from File...` and navigate to the above project directory and select the zip file.
  4. Search for "*visp*" in Add-ons section and enable the plugin called "*Export: ViSP CAO* ".
  5. Click on `Save User Settings` and close `Blender User Preferences` pannel.
  6. On the left side of Blender, there should be a new tab named `Misc` located under the `Tools`, `Create`,... tabs.

## Usage:

- The Addon consists of a Property panel and a treeview panel. The Property panel, named ViSP CAD Properties Panel, is where
the user will fill in the primitve details while the treeview panel is where the user will manage(enable, disable, delete) the different primitives(3D Face, 3D Line, 3D Cylinder, 3D Circle).

### Property panel

* To assign a primitive to a model:
    * Select the model in the blender scene view;
    * Click on `+ New` in the ViSP Property Panel;
    * Choose `Primitive Type` from dropdown list.

* If `Type == "3D Faces" || "3D Lines"`, for already simplified models i.e every vertex and edge is to be included for export, click on `Add Properties`. The added primitive will show up in the respective treeview.

* If `Type == "3D Cylinders"`:
    * Switch to `Edit Mode` to fill in three details, two axis of revolution points and the radius.
    * To get axis of revolution coordinate from the model:
        * Either choose the vertex from the model and click on `Update` or;
        * Choose three points on the circumference of one face of the cylinder and click on `Update`. The axis of revolution of this face and the radius will be calculated and filled in the respective fields.
    * To get radius of a face:
        * Choose two diametrically opposite points on the circumference and click on `Update`.
    * Finally click on `Add Properties`.

* If `Type == "3D Circles"`:
    * Switch to `Edit Mode` to fill in four details, center of circle, radius and two points on the surface.
    * To get center:
        * Either choose the vertex that represents the center of the circle and hit `Update` or;
        * Choose three points on the circumference and hit `Update`. The center and radius will be calculated and the other two points will be filled in from the selected points.
    * To get only radius:
        * Choose two diametrically opposite points on the circumference and click on `Update`.
  [The below gif contains a demo]

* When choosing `3D Faces` or `3D Lines` for complex models:
  * First simplify the model such that only object contours are present and poly count is reduced. Then follow the above steps. [tutorial link]
  * Or choose the vertices from the complex model needed:
    * Switch to `Edit Mode` and select the necessary vertices for a face from the model and hit `Get Vertices`.
      This will create a new mesh containing the selected vertices and assign a face to it. If polycount is more than one, then `Limited Dissolve` is applied to simplify further.
    * The above step is automated, thus after clicking on `Get vertices`, hit `Add Properties` to assign it to either `3D faces` or `3D Lines`.

### TreeView Management

![treeview](https://user-images.githubusercontent.com/11690674/27358411-2ed79f88-561f-11e7-8011-f0406b564477.png)


### Export

* Before exporting, make sure the necessary primitives are enabled and the remaining disabled (only enabled primitives are exported).
* Go to `File > Export > ViSP .cao` to export to .cao format.

## Uninstall:

  1. Open `File > User Preferences` and search for "*visp*"" in Add-ons search section. 
  2. Click on the triangle next to "*Export: ViSP CAO*" to expand the section and then click on `Remove`.