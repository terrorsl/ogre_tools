#ifndef BASE_SYSTEM_FILE
#define BASE_SYSTEM_FILE

#include<Ogre.h>
#include"message_system.h"

#define NUM_GAME_ENTITY_BUFFERS 4

class BaseSystem:public MessageQueueSystem
{
public:
	void BeginFrameParallel() { processIncomingMessages(); }
	virtual void FinishFrameParallel() { flushQueuedMessages(); }

	void NotifySystem(BaseSystem* sys) { notify_system = sys; }

	virtual void Update(float timeSinceLast) = 0;
protected:
	BaseSystem* notify_system;
};
#endif