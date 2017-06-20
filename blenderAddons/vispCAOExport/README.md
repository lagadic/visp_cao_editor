# Blender Export Addon for ViSP CAD file (.cao)

This Addon was tested with:
- Blender 2.78 on Ubuntu 16.04 (manual installation)
- Blender 2.76 on Ubuntu 16.04 and 14.04 (using apt-get install blender)
- Blender 2.78 on OSX.

## Installation:

  1. Run `python build.py` in project directory. `visp_cao_export.zip` file will be created.
  2. Launch Blender and open `File > User Preferences`
    - (**Ctrl+Atl+U**) under Linux
    - (**Cmd+,**) under OSX
  3. Click on `Install from File...` and navigate to the above project directory and select the zip file.
  4. Search for `visp` in Add-ons section and enable the plugin called "Export: ViSP CAO".
  5. Click on `Save User Settings` and close `Blender User Preferences` pannel
  6. On the left side of Blender, there should be a new tab named `Misc` located under the `Tools`, `Create`,... tabs 

**Usage**

  1. Select `Misc` table
  2. In the `ViSP CAD Properties Panel`, the user can assign model properties. To do this select the model(s) in the scene:
  	- Set the model(s) export type (*default is 3D Points*)
  	- If `Type == "3D Cylinders" || "3D Circles"`, then switch to `Edit Mode` and hit `Calculate Radius`. Enter the necessary points coordinates. These coordinates can be found by right clicking on vertex and then hitting `N`.
	Note: When calculating the radius, the entire object has to be selected in edit mode.

  2. To set these properties for the selected model(s), hit `Set Properties`.
  3. Go to `File > Export > ViSP .cao` to export to .cao format.
