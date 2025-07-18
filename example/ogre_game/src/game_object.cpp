#include"render_system.h"
#include"physic_system.h"
#include"game_object.h"

#include<Compositor/OgreCompositorManager2.h>
#include<OgreWindow.h>
#include<OgreManualObject2.h>

void GameObject::SetTransform(Ogre::Vector3 position, Ogre::Quaternion rotation)
{
	//graphic_object->setPosition(position);
	//graphic_object->setOrientation(rotation);
	if (collision_object)
	{
		btTransform worldTrans(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w), btVector3(position.x, position.y, position.z));
		collision_object->setWorldTransform(worldTrans);
	}
};
void GameObject::getWorldTransform(btTransform& worldTrans) const
{
	Ogre::Vector3 position = graphic_object->getPosition();
	Ogre::Quaternion rot = graphic_object->getOrientation();

	worldTrans.setOrigin(btVector3(position.x, position.y, position.z));
	worldTrans.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));
};
void GameObject::setWorldTransform(const btTransform& worldTrans)
{
	btVector3 position = worldTrans.getOrigin();
	btQuaternion rot = worldTrans.getRotation();

	graphic_object->setPosition(Ogre::Vector3(position.x(), position.y(), position.z()));
	graphic_object->setOrientation(Ogre::Quaternion(rot.w(),rot.x(),rot.y(),rot.z()));
};

GameLevel::GameLevel(Ogre::String name, RenderSystem* rs, PhysicSystem *ps)
{
	render = rs;
	physic = ps;
	scene_manager = rs->root->createSceneManager(Ogre::ST_GENERIC, 1);
};
GameLevel::~GameLevel()
{
	Ogre::CompositorManager2* compositorManager = render->root->getCompositorManager2();
	compositorManager->removeWorkspace(compositor);

	render->root->destroySceneManager(scene_manager);
};
void GameLevel::CreateLight(Ogre::Light::LightTypes type, Ogre::Vector3 position)
{
	Ogre::Light *light = scene_manager->createLight();

	Ogre::SceneNode *node = scene_manager->getRootSceneNode()->createChildSceneNode();
	node->attachObject(light);

	light->setType(type);
	if (type == Ogre::Light::LT_DIRECTIONAL)
		light->setDirection(position.normalisedCopy());
	else
		node->setPosition(position);
};
void GameLevel::Done()
{
	Ogre::Camera *camera = scene_manager->createCamera("Main Camera");

	// Position it at 500 in Z direction
	camera->setPosition(Ogre::Vector3(0, 5, 15));
	// Look back along -Z
	camera->lookAt(Ogre::Vector3(0, 0, 0));
	camera->setNearClipDistance(0.2f);
	camera->setFarClipDistance(1000.0f);
	camera->setAutoAspectRatio(true);

	// Setup a basic compositor with a blue clear colour
	Ogre::CompositorManager2* compositorManager = render->root->getCompositorManager2();
	const Ogre::String workspaceName("PbsMaterialsWorkspace");
	const Ogre::ColourValue backgroundColour(0.2f, 0.4f, 0.6f);
	if (!compositorManager->hasWorkspaceDefinition(workspaceName))
		compositorManager->createBasicWorkspaceDef(workspaceName, backgroundColour, Ogre::IdString());
	compositor = compositorManager->addWorkspace(scene_manager, render->renderWindow->getTexture(), camera, workspaceName, true);

	Ogre::ManualObject* obj = scene_manager->createManualObject();
	scene_manager->getRootSceneNode()->attachObject(obj);
	physic->SetDrawObject(obj);
};