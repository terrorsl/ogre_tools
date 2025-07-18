#ifndef GAME_OBJECT_FILE
#define GAME_OBJECT_FILE

#include<Ogre.h>
#include<btBulletDynamicsCommon.h>

class RenderSystem;
class PhysicSystem;

class GameObject:public btMotionState
{
public:
	GameObject():graphic_object(0), collision_object(0){}
	virtual ~GameObject() {}
	void SetTransform(Ogre::Vector3 position, Ogre::Quaternion rotation);

	void SetObject(Ogre::SceneNode* node) { graphic_object = node; }
	void SetObject(btRigidBody* node) { collision_object = node; }

	void getWorldTransform(btTransform& worldTrans) const;
	//Bullet only calls the update of worldtransform for active objects
	void setWorldTransform(const btTransform& worldTrans);
protected:
	Ogre::SceneNode* graphic_object;
	btRigidBody* collision_object;
};

class GameLevel
{
public:
	GameLevel(Ogre::String name, RenderSystem* rs, PhysicSystem* ps);
	virtual ~GameLevel();

	void CreateLight(Ogre::Light::LightTypes type, Ogre::Vector3 position);
	void AppendObject(GameObject* obj);

	void Done();
private:
	RenderSystem* render;
	PhysicSystem* physic;
	Ogre::SceneManager* scene_manager;
	Ogre::CompositorWorkspace* compositor;
};
#endif