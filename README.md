# Ogre Tools

## Ogre Studio

## Ogre Mesh Editor

## Ogre Material Editor

## Ogre Studio Plugin

## requirement
- Cmake
- Qt6
- Ogre-next

## Build
Run cmake gui or command line

Cmake Variable:  
OGRE_DIR - Ogre-Next install folder  
QT_PATH - Qt6 install folder

```shell
cmake -DOGRE_DIR=<path> -DQT_PATH=<path> -G <generator-name> -B <build dir>
cmake --build <build dir>
```