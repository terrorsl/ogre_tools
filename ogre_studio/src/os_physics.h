#ifndef OS_PHYSICS_FILE
#define OS_PHYSICS_FILE

#include<btBulletDynamicsCommon.h>
#include<Ogre.h>
#include<OgreManualObject2.h>
#include<Vao/OgreAsyncTicket.h>

class OgrestudioPhysicsDebugDraw:public btIDebugDraw
{
	Ogre::ManualObject *lines;
public:
	OgrestudioPhysicsDebugDraw(Ogre::ManualObject *line, btDiscreteDynamicsWorld* world);

	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {
		drawLine(PointOnB, PointOnB + normalOnB * distance * 20, color);
	};
	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	void reportErrorWarning(const char* warningString);
	void draw3dText(const btVector3& location, const char* textString);
	void setDebugMode(int debugMode);
	int getDebugMode() const;

	void update(btDiscreteDynamicsWorld *);
	void clear();
private:
	int debugMode;
};

class OgreStudioVertexIndexToShape
{
public:
	OgreStudioVertexIndexToShape(Ogre::SceneNode *node);
	~OgreStudioVertexIndexToShape();

	btBvhTriangleMeshShape* createTrimesh();
	btConvexHullShape* createConvex();
private:
	void addVertexData(Ogre::VertexArrayObject::ReadRequests buffer, Ogre::Matrix4 tr);
	void addIndexData(const char* data, Ogre::IndexType type, unsigned long indexCount, unsigned long offset);

	unsigned long vertexCount, indexCount;
	Ogre::Vector3 *vertexs;
	unsigned int* indexes;
};

class OgreStudioPhysicsMotionState :public btMotionState
{
public:
	OgreStudioPhysicsMotionState(Ogre::SceneNode *n):node(n){}

	void getWorldTransform(btTransform& ret) const override
	{
		ret = btTransform(btQuaternion(node->getOrientation().x, node->getOrientation().y, node->getOrientation().z, node->getOrientation().w),
			btVector3(node->getPosition().x, node->getPosition().y, node->getPosition().z));
	}

	void setWorldTransform(const btTransform& in) override
	{
		btQuaternion rot = in.getRotation();
		btVector3 pos = in.getOrigin();
		node->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
		node->setPosition(pos.x(), pos.y(), pos.z());
	}
private:
	Ogre::SceneNode* node;
};

class OgreStudioPhysics
{
public:
	~OgreStudioPhysics();

	void Initialize();
	void Update(float dt);

	void New(Ogre::SceneManager *sm);
	void AppendObject(Ogre::SceneNode* node);
	void RemoveObject(Ogre::SceneNode* node);

	bool IsObjectInWorld(Ogre::SceneNode* node);
private:
	btCollisionConfiguration* collisionConfiguration;
	btDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;

	OgrestudioPhysicsDebugDraw *debugDraw;
};
#endif