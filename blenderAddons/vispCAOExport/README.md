# Blender Export Addon for ViSP CAD file (.cao)

**Installation**:
  
  1) Run `blender --background --python setup.py` in project directory.
  2) Launch Blender and open User Preferences (**Ctrl+Atl+U**)
  3) Search for "visp" in Addons section and enable the plugin called "Export: ViSP CAO".

**Usage**

  1) In the **ViSP CAD Properties Panel** under the Tools menu of blender, the user can assign model properties. To do this select the model(s) in the scene: 
  	1.1) Set the model(s) export type (*default is 3D Points*)
	1.2) If `Type == "3D Cylinders" || "3D Circles"`, then switch to `Edit Mode` and hit `Calculate Radius`. Enter the necessary points coordinates. These coordinates can be found by right clicking on vertex and then hitting `N`. 
	Note: When calculating the radius, the entire object has to be selected in edit mode.

  2) To set these properties for the selected model(s), hit `Set Properties`.
  3) Go to `File > Export > ViSP .cao` to export to .cao format.
