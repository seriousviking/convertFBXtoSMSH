# ConvertFBXtoSMSH

ConvertFBXtoSMSH application is a command-line utility which converts
scenes from FBX files to light-weight GPU-friendly format.

## Features:
* import meshes from FBX scene
* combine vertices so that all vertex data streams will have the same length
* merge vertices with the same positions and texture coordinates and similar normals
* export each mesh in separate file, also create Json file with scene 
  contents: objects, their names, uids, parents and transform. So you 
  can build scene graph using this information

## Usage:
* build executable
* in command line, use ConvertFBXtoSMSH <source_fbx_file> <destination_file_template>
  
destination_file_template can be just a valid file name for now, later that will be changed

## Project structure
    * src/ - source files
        * Common.h - common constants/data types
        * ConvertFBXtoSMSH.cpp - console utility main entry point - handle
          command-line arguments, import FBX file and export scene files
        * ExportMesh.h/.cpp - export mesh file
        * ExportScene.h/.cpp - export scene file in Json format
        * ImportFBX.h/.cpp - main file which imports FBX scene
        * stdafx.h/.cpp - common includes. However, PCH feature is disabled 
          for this project
        * StreamMaterialData.h - material format structures
        * StreamMeshData.h - mesh format structures
        * targetver.h - sets minimum required Windows version
        * Utils.h/.cpp - utility functions

    * lib/jsoncpp/* - JsonCpp library source and header files
    * ConvertFBXtoSMSH.sln/.vcxproj* - Visual Studio solution and project files

## Build requirements/recommendations:
* Autodesk FBX SDK http://usa.autodesk.com/adsk/servlet/pc/item?siteID=123112&id=26416130

  I used version 2018.1.1 for VS2015 but earlier versions should work too

* Visual Studio 2015 or later

  I did't use any VS-specific features but sometimes used C++11 features
  so any C++11-compliant compiler/IDE should be able to build executable
  if you set up dependencies correctly

## If you want to use source files in different IDE:
* Include directories which put in project settings:

    $(ProjectDir)lib\jsoncpp\include - JsonCpp library https://github.com/open-source-parsers/jsoncpp
    <Autodesk FBX SDK path>\include

* Library directories which put in project settings:

    <Autodesk FBX SDK path>\lib\vs2015\x64\debug
    
    For me, <Autodesk FBX SDK path> is

    C:\Program Files\Autodesk\FBX\FBX SDK\2018.1.1

* Linker dependencies:

    libfbxsdk-md.lib (take appropriate file from FBX SDK)

    Earlier I used libfbxsdk.lib and placed libfbxsdk.dll but ClassId functions 
	in FBX SDK do not work in that way.

* if you're using Visual Studio IDE, it might be useful also to 
  put libfbxsdk.pdb to the same directory as executable file to make 
  FBX import debugging easier

## TODO:

* Implement materials import and export.
  Decide whether to save materials separately or with the mesh geometry

* Test performance with two approaches of working with vertex data:
  Structure of Arrays (current) or Array of Structures (alternative)
  Implement alternative approach if it will be faster in some cases

* CMake build system integration

## License
See LICENSE file for details. In summary, it's licensed under MIT license
