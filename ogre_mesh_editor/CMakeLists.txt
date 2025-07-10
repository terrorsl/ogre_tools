project(ogre_mesh_editor)

qt_standard_project_setup()

set(SRC
	src/main.cpp
	src/mainwindow.cpp
	src/qogrewidget.cpp
	src/progress_dialog.cpp
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

qt_add_executable(ogre_mesh_editor ${SRC} ${UI})

target_include_directories(ogre_mesh_editor PRIVATE ${OGRE_INC} ${Qt6Widgets_INCLUDE_DIRS} ${ASSIMP_DIR}/include)

target_link_directories(ogre_mesh_editor PRIVATE debug ${Qt6Widgets_LIBS} ${OGRE_DIR}/lib/Debug)
target_link_libraries(ogre_mesh_editor PRIVATE debug Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Core5Compat
	OgreNextMain_d.lib OgreNextHlmsUnlit_d.lib OgreNextHlmsPbs_d.lib 
	${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/$<CONFIG>/assimp-vc143-mtd.lib
	${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/$<CONFIG>/zlibstaticd.lib
)

set_target_properties(ogre_mesh_editor PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)