#include"physic_system.h"
#include"render_system.h"

#include<OgreManualObject2.h>

bool PhysicSystem::Initialize()
{
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	dynamicsWorld->setDebugDrawer(&drawer);

	btCollisionShape* shape;
	btRigidBody* body;
	
	shape = new btBoxShape(btVector3(0.5, 0.5, 0.5));
	body = new btRigidBody(0, 0, shape);
	dynamicsWorld->addRigidBody(body);

	shape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
	body = new btRigidBody(0, 0, shape);
	dynamicsWorld->addRigidBody(body);
	return true;
};
void PhysicSystem::UnInitialize()
{
	while (dynamicsWorld->getNumCollisionObjects())
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[0];
		btRigidBody* body = btRigidBody::upcast(obj);

		btCollisionShape* shape = body->getCollisionShape();
		switch (shape->getShapeType()) {
		case CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE:
			delete ((btConvexTriangleMeshShape*)shape)->getMeshInterface();
			break;
		case TRIANGLE_MESH_SHAPE_PROXYTYPE:
			delete ((btBvhTriangleMeshShape*)shape)->getMeshInterface();
			break;
		}

		delete shape;
		dynamicsWorld->removeRigidBody(body);
		delete body;
	}
	delete dynamicsWorld;
	delete solver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;
}
void PhysicSystem::DebugDraw()
{
	dynamicsWorld->debugDrawWorld();
};
void PhysicSystem::Update(float timeStep,bool & update)
{
#if 0
	//drawer.clearLines();
	if (update == false)
	{
		dynamicsWorld->debugDrawWorld();
		update = true;
	}
	//drawer.flushLines();
#endif
	dynamicsWorld->stepSimulation(timeStep, 10);
};
void PhysicSystem::SetDrawObject(Ogre::ManualObject* obj)
{
	drawer.SetDrawObject(obj);
};
void PhysicSystem::AppendObject(Ogre::SceneNode* node)
{

};
void PhysicSystem::RemoveObject(Ogre::SceneNode* node)
{
};

PhysicDrawSystem::PhysicDrawSystem():lines(0), debugMode(DBG_DrawWireframe)
{
}
void PhysicDrawSystem::SetDrawObject(Ogre::ManualObject* obj)
{
	lines = obj;
	//lines->begin("Ogre/Debug/LinesMat", Ogre::OT_LINE_LIST);
	//lines->end();
}
void PhysicDrawSystem::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	int index = lines->getCurrentVertexCount();
	lines->position(Ogre::Vector3(from.x(), from.y(), from.z()));
	lines->position(Ogre::Vector3(to.x(), to.y(), to.z()));
	lines->line(index, index + 1);
}

void PhysicDrawSystem::reportErrorWarning(const char* warningString)
{
}

void PhysicDrawSystem::draw3dText(const btVector3& location, const char* textString)
{
}

void PhysicDrawSystem::setDebugMode(int debugMode)
{
}

int PhysicDrawSystem::getDebugMode() const
{
	return debugMode;
}
void PhysicDrawSystem::clearLines()
{
	if (lines)
	{
		if(lines->getNumSections()==0)
			lines->begin("Ogre/Debug/LinesMat", Ogre::OT_LINE_LIST);
		else
			lines->beginUpdate(0);
	}
};
void PhysicDrawSystem::flushLines()
{
	if (lines)
		lines->end();
};