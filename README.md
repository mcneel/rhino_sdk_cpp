# Cross platform C++ SDK for Rhino

This repository contains a complete C++ SDK for Rhino, and is intended to be used as a submodule.

The "main" branch corresponds with the latest service release of Rhino.  If you want to target a specific service release, use the corresponding branch.

To create your own plug-in from scratch, follow the instructions below to set up projects for Windows and OSX.  If you intend your project to be cross platform, begin with the OSX project.

## Apple OSX

### Creating the OSX Bundle

You will need to create a “Bundle” project in Xcode.  The compiled bundle will become your RHP - but it is actually set of folders much like a package.

 * Open XCode and click “Create New Project”
 * Under “Framework & Library” click “Bundle” and click “Next”.
 * Name your Bundle.  Set the Organisation. Set the Bundle extension to “rhp”.
 * Select a location on the disk and click “Create”
 * Create a git repository, for example:

    cd MyProjectName
    git init
    git add .
    git commit -m "Initial commit"
    gh repo create MyOrg/MyProjectName --source=. --push

* Next add this repository as a submodule

    git submodule add https://github.com/mcneel/rhino_sdk_cpp.git SDK
    git commit -m "Add Rhino SDK submodule"

* In XCode, select the name of your project in the Project Navigator (the leftmost bar), and choose “Add files to MYPROJECT”.  Select the XCConfig folder inside the SDK folder and click “Add”  Select "Reference files in place" and click “Finish”.
* In XCode, select the name of your project in the Project Navigator (the leftmost bar), and then select the name of your project under “PROJECT” slightly to the right (not TARGETS).
* Select the leftmost “Info” tab.
* Under “Configurations” - further to the right.  Open the “Debug” configuration drop down and Click where it says “None” on the first line under this. Select PlugInDebug.xcconfig.
* Do the same for the release section - selecting PlugInRelease.xcconfig.

### Adding the frameworks.

* Select the name of your Bundle under “TARGETS”.
* Select the leftmost “General” tab.
* On the right, click “Frameworks and Libraries” and press the “+” button.
* On the following dialog, click “Add Other…” and choose “Add Files”
* Navigate to the “SDK/lib” folder and select the three frameworks inside (OpenNURBS, RhCore, RhMaterialEditor).  Click Open.  They should now appear as Frameworks.

### Adding the source files.

* In XCode, select the name of your project in the Project Navigator (the leftmost bar), and choose New File from Template.
* Add a C++ File and a Header file with the name of your Plugin (adding a C++ file will also give you the option to automatically add a header file).
* Make sure it is included in the target.
* Add the following lines to your header file:

#include "SDK/inc/rhinoSdkStdafxPreamble.h"
#include "SDK/inc/rhinoSdk.h"
#include "SDK/inc/RhRdkHeaders.h"
#include "SDK/inc/rhinoSdkChecks.h"

You are now ready to start setting up your boilerplate CRhinoPlugIn class in the CPP file.
    
    class MYPLUGIN : public CRhinoUtilityPlugIn
    {
        GUID PlugInID() const override
        {
            static const GUID guid = { 0xxxxxx, 0xc1d1, 0x4529, { 0x8e, 0x7e, 0x7b, 0x22, 0x9d, 0x6f, 0x5a, 0xa4 } };
            return guid;
        }
        
        const wchar_t *PlugInName() const override
        {
            return L"MYPLUGIN";
        }
        
        const wchar_t *PlugInVersion() const override
        {
            return L"1.0.0";
        }
        
        int OnLoadPlugIn(void) override
        {   
            return 1;
        }
    };
    static MYPLUGIN my_plug_in;

### Compiling your plugin.

Command-B to build.  Everything should compile.

To find the compiled RHP, Choose “Show Build Folder in Finder” from the Product menu of XCode.  Navigate into the Products/Debug folder.

### Getting your plugin to load into Rhino.

Download Rhino 9.
Use the TestLoadPlugin command and select your compiled bundle.

### Debugging your plug-in

By default, Rhino cannot be debugged.  However, we have included a shell script make_rhino_debuggable.sh which you should run after you download Rhino to make the bundle debuggable.  Notice that the sample xcodeproj includes this script as a RunScript action meaning that you don't actually need to run the shell command.

To run Rhino when you press the "Run" button in Xcode, Edit the Scheme for the Debug configuration to change the executable to RhinoWIP.app (from the applications folder).  Rhino will now start running in the debugger.  You can alteratively attach to the Rhino process once it is running.  In both cases, your breakpoints in your C++ plug-in code should be activated and hit.

To run the shell script, open Terminal, navigate to the repo directory and type sh make_rhino_debuggable.sh

## Microsoft Windows

These instructions build a Windows plug-in project **around the folder you already created in the OSX steps**.  They assume that folder already exists as a git repository, that this SDK is present as the `SDK` submodule, and that your shared `.cpp` / `.hpp` source files are already in place.  The same source files and the same submodule are reused - only the Visual Studio project files are new.  Use Visual Studio 2026.

If you are starting on Windows first, complete the "Creating the OSX Bundle" steps up to and including adding the submodule and creating the source files (you can do the git and file steps on Windows - Xcode is not required), then continue here.

### Prerequisites

* Visual Studio 2026 with the **Desktop development with C++** workload, including the **MFC** component and a recent **Windows 10/11 SDK**.  The Rhino C++ SDK is MFC-based.
* Rhino 9 installed.  The Windows import libraries (`RhinoCore.lib`, `opennurbs.lib` and `rdk.lib`) live in this submodule's `lib` folder, alongside the macOS frameworks, so you do not need to reference anything under `Program Files`.  **Note:** adding the Windows `.lib` files to `lib` is still to be done - until they are present the link step will fail.

### Creating the Windows project

* Open Visual Studio 2026 and choose **Create a new project**.
* Select the **Dynamic-Link Library (DLL)** C++ template and click **Next**.
* Set the project name to match your plug-in and set the **Location** to your existing project folder (the one that contains the `SDK` submodule and your `.xcodeproj`), so the `.sln` and `.vcxproj` sit alongside them.  Click **Create**.
* Delete the auto-generated `dllmain.cpp`, `pch.h` and `pch.cpp` (or `framework.h`) files that the template adds - you will use the shared source and your own precompiled header instead.
* In **Solution Explorer**, right-click the project, choose **Add > Existing Item**, and add your shared `.cpp` and `.hpp` files (the same ones used by the Xcode target).

### Adding a precompiled header

The SDK requires `rhinoSdkStdafxPreamble.h` to be the very first header compiled in every translation unit.  Create a precompiled header so this happens automatically without editing the shared, cross-platform source.

* Add a new header `stdafx.h` to the project with this content:

    #pragma once
    #include "SDK/inc/rhinoSdkStdafxPreamble.h"
    #include "SDK/inc/rhinoSdk.h"
    #include "SDK/inc/RhRdkHeaders.h"
    #include "SDK/inc/rhinoSdkChecks.h"

* Add a source file `stdafx.cpp` containing a single line:

    #include "stdafx.h"

### Configuring the project

Open the project **Properties**.  Set the **Configuration** to *All Configurations* and the **Platform** to **x64** (Rhino 9 is 64-bit only; you can remove the Win32/x86 platform).

* **Configuration Properties > General**
    * *Configuration Type*: `Dynamic Library (.dll)`
    * *Target Extension*: `.rhp`
    * *Use of MFC*: `Use MFC in a Shared DLL`
    * *Character Set*: `Use Unicode Character Set`
* **C/C++ > General > Additional Include Directories**: add `$(ProjectDir)` (so the `SDK/inc/...` includes in the source resolve relative to the project folder).
* **C/C++ > Preprocessor > Preprocessor Definitions**: add `RHINO_LIB_DIR` pointing at the submodule's `lib` folder.  It must expand to a string literal; use a path relative to the project directory (forward slashes are fine):

    RHINO_LIB_DIR="SDK/lib"

    The SDK's linking pragmas concatenate this with the library names and resolve the result relative to `$(ProjectDir)`, so the three libraries link automatically and no manual entries are needed under **Linker > Input**.
* **C/C++ > Precompiled Headers**: set *Precompiled Header* to `Use (/Yu)` and *Precompiled Header File* to `stdafx.h`.  Then select `stdafx.cpp` on its own, and in its properties set *Precompiled Header* to `Create (/Yc)`.
* **C/C++ > Advanced > Forced Include File**: add `stdafx.h`.  This injects the precompiled header (and therefore the preamble) first in every translation unit, keeping your shared `.cpp` / `.hpp` unchanged and cross-platform.

### Compiling your plugin

Build the solution (Ctrl+Shift+B) for the **x64 Debug** configuration.  The compiled plug-in will be written to the configuration output folder (for example `x64\Debug\MyProjectName.rhp`).

### Getting your plugin to load into Rhino

Start Rhino 9 and run the **TestLoadPlugin** command, then select your compiled `.rhp`.  (Alternatively, drag the `.rhp` onto an open Rhino window, or install it from **Tools > Options > Plug-ins**.)

### Debugging your plug-in

* Open the project **Properties > Configuration Properties > Debugging**.
* Set **Command** to your Rhino executable, for example `C:\Program Files\Rhino 9\System\Rhino.exe`.
* Press **F5**.  Rhino starts under the Visual Studio debugger; load your plug-in with `TestLoadPlugin` and your breakpoints in the C++ code will be hit.  You can also use **Debug > Attach to Process** to attach to an already-running Rhino.

### Committing the Windows project

Commit the new Windows files (the `.sln`, `.vcxproj`, `.vcxproj.filters`, `stdafx.h` and `stdafx.cpp`) to your repository.  The shared `.cpp` / `.hpp` and the `SDK` submodule are unchanged and remain common to both platforms.

























