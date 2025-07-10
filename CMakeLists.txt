cmake_minimum_required(VERSION 3.24)
project(ogre_tools)

UNSET(INCLUDE_PATH CACHE)
UNSET(LIBRARY_PATH CACHE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/archive)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(QT_PATH "C:/Qt/_Qt-6.5.0" CACHE PATH "" FORCE)
set(OGRE_DIR "" CACHE PATH "OGRE path")

set(JSON_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/library/jsoncpp-1.9.6/dist)
set(BULLET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/library/bullet3-3.25)
set(ASSIMP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/library/assimp-5.4.3)

set(PLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ogre_studio_plugin)

set(CMAKE_PREFIX_PATH ${QT_PATH})

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Core5Compat)
#find_package(OGRE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ogre_studio)
add_subdirectory(ogre_mesh_editor)
add_subdirectory(ogre_material_editor)
add_subdirectory(example/ogre_export_plugin)

set(ASSIMP_BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(${ASSIMP_DIR})