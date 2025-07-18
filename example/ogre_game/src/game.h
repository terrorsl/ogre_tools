#ifndef GAME_FILE
#define GAME_FILE

#include"logic_system.h"
#include"render_system.h"
#include"physic_system.h"

class Game
{
public:
	bool Initialize(std::string app_name, LogicSystem *);
	void UnInitialize();

	void Run();

	virtual void OnStart() = 0;
	virtual void OnClose() = 0;
protected:
	RenderSystem* render;
	LogicSystem* logic;
	PhysicSystem* physic;
};
#endif