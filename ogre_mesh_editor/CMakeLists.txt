project(ogre_mesh_editor)

set(CMAKE_AUTORCC ON)
qt_standard_project_setup()

set(SRC
	src/main.cpp
	src/mainwindow.cpp
	src/qogrewidget.cpp
	src/progress_dialog.cpp

	icons/resource.qrc
)
set(UI_FILES
	ui/mainwindow.ui
	ui/progress.ui
)
set(OGRE_INC
	${OGRE_DIR}/include/OGRE-Next
	${OGRE_DIR}/include/OGRE-Next/Hlms
	${OGRE_DIR}/include/OGRE-Next/Hlms/Common
)
qt6_wrap_ui(UI ${UI_FILES})

add_executable(ogre_mesh_editor ${SRC} ${UI})

target_include_directories(ogre_mesh_editor PRIVATE ${Qt6Widgets_INCLUDE_DIRS} ${OGRE_INC} ${ASSIMP_DIR}/include)

target_link_directories(ogre_mesh_editor PRIVATE ${QT_PATH}/lib ${OGRE_DIR}/lib)

target_link_libraries(ogre_mesh_editor PRIVATE 
	debug Qt6Cored
	debug Qt6Widgetsd
	debug Qt6Core5Compatd
	debug Qt6Guid
	debug Qt6EntryPointd
	debug OgreNextMain_d
	debug OgreNextHlmsUnlit_d
	debug OgreNextHlmsPbs_d
	debug ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/$<CONFIG>/assimp-vc143-mtd.lib
	debug ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/$<CONFIG>/zlibstaticd.lib
)
target_link_libraries(ogre_mesh_editor PRIVATE
	optimized Qt6Core
	optimized Qt6Widgets
	optimized Qt6Core5Compat
	optimized Qt6Gui
	optimized Qt6EntryPoint
	optimized OgreNextMain
	optimized OgreNextHlmsUnlit
	optimized OgreNextHlmsPbs
	optimized ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/$<CONFIG>/assimp-vc143-mt.lib
	optimized ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/$<CONFIG>/zlibstatic.lib
)
set_target_properties(ogre_mesh_editor PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
if (MSVC)
    target_compile_options(ogre_mesh_editor PUBLIC "/Zc:__cplusplus" "/permissive-")
endif()