#include"test_game.h"

TestGameLevel::TestGameLevel(Ogre::String name, RenderSystem* rs, PhysicSystem *ps):GameLevel(name, rs, ps)
{
};

void TestGame::OnStart()
{
	level = new TestGameLevel("test", render, physic);

	level->CreateLight(Ogre::Light::LT_DIRECTIONAL, Ogre::Vector3(-1, -1, -1));

	level->Done();
};
void TestGame::OnClose()
{
	delete level;
}

