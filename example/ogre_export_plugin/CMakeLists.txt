project(ogre_export_plugin)

set(SRC
	src/main.cpp
)

set(OGRE_INC
	${OGRE_DIR}/include/OGRE-Next
)

add_library(ogre_export_plugin SHARED ${SRC})

target_include_directories(ogre_export_plugin PRIVATE ${PLUGIN_DIR}/src ${OGRE_INC})
#set(runtime_output "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/plugins")

set_target_properties(ogre_export_plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/plugins")

#target_link_directories(ogre_studio PRIVATE debug ${Qt6Widgets_LIBS} ${OGRE_DIR}/lib/Debug ${BULLET_DIR}/build/lib/Debug)
#target_link_libraries(ogre_studio PRIVATE debug Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Core5Compat
#	OgreNextMain_d.lib OgreNextHlmsUnlit_d.lib OgreNextHlmsPbs_d.lib BulletDynamics_Debug.lib BulletCollision_Debug.lib LinearMath_Debug.lib
#)