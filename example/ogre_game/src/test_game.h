#ifndef TEST_GAME_FILE
#define TEST_GAME_FILE

#include"game.h"

class TestStaticGameObject :public GameObject
{

};

class TestLogicSystem :public LogicSystem
{
public:
	void Update(float) {}

	void AppendObject(GameObject* obj) {}
};

class TestGameLevel:public GameLevel
{
public:
	TestGameLevel(Ogre::String name, RenderSystem* rs, PhysicSystem* ps);
};

class TestGame :public Game
{
public:
	void OnStart();
	void OnClose();
private:
	TestGameLevel* level;
};
#endif