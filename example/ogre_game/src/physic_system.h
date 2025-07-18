#ifndef PHYSIC_SYSTEM_FILE
#define PHYSIC_SYSTEM_FILE

#include"game_object.h"

class RenderSystem;

class PhysicSystemMotionState :public btMotionState
{

};

class PhysicDrawSystem :public btIDebugDraw
{
	Ogre::ManualObject* lines;
public:
	PhysicDrawSystem();

	void SetDrawObject(Ogre::ManualObject* obj);

	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {
		drawLine(PointOnB, PointOnB + normalOnB * distance * 20, color);
	};
	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	void reportErrorWarning(const char* warningString);
	void draw3dText(const btVector3& location, const char* textString);
	void setDebugMode(int debugMode);
	int getDebugMode() const;

	void clearLines();
	void flushLines();
private:
	int debugMode;
};

class PhysicSystem
{
public:
	bool Initialize();
	void UnInitialize();

	void Update(float,bool&);

	void DebugDraw();

	void SetDrawObject(Ogre::ManualObject* obj);

	void AppendObject(Ogre::SceneNode* node);
	void RemoveObject(Ogre::SceneNode* node);
private:
	PhysicDrawSystem drawer;

	btCollisionConfiguration* collisionConfiguration;
	btDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;
};
#endif