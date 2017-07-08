# Blender Import Addon for ViSP CAD file (.cao)

This Addon was tested with:
- Blender 2.78 on Ubuntu 16.04 (manual installation)
- Blender 2.76 on Ubuntu 16.04 and 14.04 (using apt-get install blender)
- Blender 2.78 on OSX.

## Installation:

  1. Run `python build.py` in project directory. `visp_cao_import.zip` file will be created.
  2. Launch Blender and open `File > User Preferences`.
    - (**Ctrl+Atl+U**) under Linux
    - (**Cmd+,**) under OSX
  3. Click on `Install from File...` and navigate to the above project directory and select the zip file.
  4. Search for "*visp*" in Add-ons section and enable the plugin called "*Import: ViSP CAO* ".
  5. Click on `Save User Settings` and close `Blender User Preferences` pannel.


## Usage

* Go to `File > Import > ViSP .cao` to import a .cao format file.


## Uninstall:

  1. Open `File > User Preferences` and search for "*visp*"" in Add-ons search section.
  2. Click on the triangle next to "*Import: ViSP CAO*" to expand the section and then click on `Remove`.