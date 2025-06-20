cmake_minimum_required(VERSION 3.24)
project(ogre_tools)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(QT_PATH "C:/Qt/_Qt-6.5.0" CACHE PATH "" FORCE)
set(OGRE_DIR "E:/work/ogre-next-sdk-v3.0.0-vc22-x64" CACHE PATH "" FORCE)

set(CMAKE_PREFIX_PATH ${QT_PATH})

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Core5Compat)

set(QT_INCLUDE ${Qt6Widgets_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ogre_studio)
add_subdirectory(ogre_mesh_editor)
add_subdirectory(ogre_material_editor)