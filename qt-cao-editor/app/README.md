# Qt CAO Editor

Install Qt
-------------------------------------------------------------------------------

Before you can compile this cao-editor, ensure Qt (>= 5.5) dev libraries have been installed:

* On Ubuntu: `sudo apt install qt5-default qttools5-dev-tools zlib1g-dev qtdeclarative5-dev`
* On macOS with [Homebrew](http://brew.sh/):
  + `brew install qt5`
  + `brew link qt5 --force`
* On Arch Linux:    `sudo pacman -S qt`
* On Fedora:        `sudo dnf builddep tiled`

Or, you can [download Qt from the official webpage](https://www.qt.io/download-qt-installer).
You will still need to install a development environment alongside and some libraries depending on your system, for example:

* On Ubuntu: `sudo apt install build-essential zlib1g-dev libgl1-mesa-dev`

It is also recommended that you install [Qt Creator](https://doc.qt.io/qtcreator/) if you are interested in exploring the source code.

Build and Run
-------------------------------------------------------------------------------

There are two ways to build and run the Qt application.

**1. Via CLI**:

    $ cd visp_cao_editor/qt-cao-editor/app
    $ mkdir build
    $ cd build
    $ qmake ../app.pro
    $ make

You can then launch the GUI by running the executable named `app`.

**2. Via Qt Creator**:

Qt Creator is a cross-platform IDE which is part of the SDK for the Qt GUI application development framework.

- Launch Qt Creator.
- `Ctrl + O` or `File -> Open File or Project`.
- Select `app.pro` under `visp_cao_editor/qt-cao-editor/app`.
- `Ctrl + B` to build and `Ctrl + R` to Run.

