project(ogre_studio)

qt_standard_project_setup()

set(SRC
	src/main.cpp
	src/mainwindow.cpp
	src/qogrewidget.cpp
	src/os_level.cpp
	${JSON_CPP_DIR}/jsoncpp.cpp
)
set(UI_FILES
	ui/mainwindow.ui
)
set(OGRE_INC
	${OGRE_DIR}/include/OGRE-Next
	${OGRE_DIR}/include/OGRE-Next/Hlms
	${OGRE_DIR}/include/OGRE-Next/Hlms/Common
)
qt6_wrap_ui(UI ${UI_FILES})

#add_executable(ogre_studio ${SRC})
qt_add_executable(ogre_studio ${SRC} ${UI})

target_include_directories(ogre_studio PRIVATE ${OGRE_INC} ${JSON_CPP_DIR} ${Qt6Widgets_INCLUDE_DIRS})

target_link_directories(ogre_studio PRIVATE debug ${Qt6Widgets_LIBS} ${OGRE_DIR}/lib/Debug)
target_link_libraries(ogre_studio PRIVATE debug Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Core5Compat
	OgreNextMain_d.lib OgreNextHlmsUnlit_d.lib OgreNextHlmsPbs_d.lib
)

#target_link_directories(ogre_studio PRIVATE optimized ${Qt6Widgets_LIBS})
#target_link_libraries(ogre_studio PRIVATE optimized Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Core5Compat)
set_target_properties(ogre_studio PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)