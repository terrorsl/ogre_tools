#include"game.h"

bool Game::Initialize(std::string app_name, LogicSystem* ls)
{
	logic = ls;
	render = new RenderSystem();
	render->Initialise(app_name);

	physic = new PhysicSystem();
	physic->Initialize();

	render->NotifySystem(logic);
	logic->NotifySystem(render);

	OnStart();
	return true;
};
void Game::UnInitialize()
{
	OnClose();

	if (physic)
	{
		physic->UnInitialize();
		delete physic;
	}
	if (render)
	{
		render->DeInitialize();
		delete render;
	}
};
void Game::Run()
{
	Ogre::Timer timer;

	Ogre::uint64 startTime = timer.getMicroseconds();
	const double cFrametime = 1.0 / 25.0;
	double accumulator = cFrametime;

	double timeSinceLast = 1.0 / 60.0;
	bool update = false;
	while (render->IsQuit()==false)
	{
		while (accumulator >= cFrametime)
		{
			render->BeginFrameParallel();
			
			logic->BeginFrameParallel();
			logic->Update(cFrametime);
			physic->Update(cFrametime, update);
			logic->FinishFrameParallel();
			
			render->FinishFrameParallel();

			accumulator -= cFrametime;
		}

		physic->DebugDraw();

		render->BeginFrameParallel();
		render->Update(timeSinceLast);
		render->FinishFrameParallel();
		update = false;

		Ogre::uint64 endTime = timer.getMicroseconds();
		timeSinceLast = double(endTime - startTime) / 1000000.0;
		timeSinceLast = std::min(1.0, timeSinceLast);  // Prevent from going haywire.
		accumulator += timeSinceLast;
		startTime = endTime;
	}
};