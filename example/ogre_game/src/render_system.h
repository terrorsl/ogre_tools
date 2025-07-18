#ifndef RENDER_SYSTEM_FILE
#define RENDER_SYSTEM_FILE

#include<Ogre.h>
#include"base_system.h"

class RenderSystem:public BaseSystem, public Ogre::WindowEventListener
{
	friend class GameLevel;
	friend class PhysicSystem;
public:
	bool Initialise(Ogre::String& app_name);
	void DeInitialize();

	void Update(float timeSinceLast) { Ogre::WindowEventUtilities::messagePump(); if(quit==false)root->renderOneFrame();  accumTimeSinceLastLogicFrame += timeSinceLast;}
	bool IsQuit() { return quit; }

	Ogre::SceneManager* Create(Ogre::String& name);
private:
	void processIncomingMessage(MessageId messageId, const void* data);

	void windowClosed(Ogre::Window* rw);
	bool initialiseHLMS();

	bool quit;
	float accumTimeSinceLastLogicFrame;
	Ogre::uint32 mCurrentTransformIdx;

	Ogre::Root* root;
	Ogre::Window* renderWindow;
};
#endif