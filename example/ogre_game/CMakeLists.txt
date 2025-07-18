project(ogre_game)

set(SRC
	src/main.cpp
	src/test_game.cpp
	src/game.cpp
	src/render_system.cpp
	src/message_system.cpp
	src/physic_system.cpp
	src/logic_system.cpp
	src/game_object.cpp
)

set(OGRE_INC
	${OGRE_DIR}/include/OGRE-Next
	${OGRE_DIR}/include/OGRE-Next/Hlms
	${OGRE_DIR}/include/OGRE-Next/Hlms/Common
)

add_executable(ogre_game WIN32 ${SRC})

target_include_directories(ogre_game PRIVATE ${PLUGIN_DIR}/src ${OGRE_INC} ${CMAKE_CURRENT_SOURCE_DIR}/src ${BULLET_DIR}/src)
#set(runtime_output "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/plugins")

#set_target_properties(ogre_export_plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/plugins")

target_link_directories(ogre_game PRIVATE debug ${OGRE_DIR}/lib/Debug ${BULLET_DIR}/build/lib/Debug ${BULLET_DIR}/build/lib/Debug)
target_link_libraries(ogre_game PRIVATE debug OgreNextMain_d.lib OgreNextHlmsUnlit_d.lib OgreNextHlmsPbs_d.lib
	BulletDynamics_Debug.lib BulletCollision_Debug.lib LinearMath_Debug.lib
)