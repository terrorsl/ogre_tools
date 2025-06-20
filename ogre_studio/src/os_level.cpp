#include"os_level.h"
#include<fstream>

#include<OgreMeshManager2.h>

OgreStudioLevel::OgreStudioLevel(std::string& name, Ogre::SceneManager* _sm):filename(name),sm(_sm)
{
	Ogre::Light *light = sm->createLight();
	Ogre::SceneNode* lightNode = sm->getRootSceneNode()->createChildSceneNode();
	lightNode->attachObject(light);
	
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setName("light");
};
bool OgreStudioLevel::Load(std::string& fname)
{
	Json::CharReaderBuilder builder;
	std::ifstream file;
	file.open(fname);
	Json::Value root;
	Json::String err;
	if (parseFromStream(builder, file, &root, &err) == false)
	{
		return false;
	}
	file.close();
	filename = fname;
	return true;
};
bool OgreStudioLevel::Save()
{
	Json::Value root;
	serialize(root);

	Json::StreamWriterBuilder builder;
	builder["indentation"] = "\t";
	Json::StreamWriter *writer = builder.newStreamWriter();
	std::ofstream file;
	file.open(filename, std::ios_base::out);
	writer->write(root, &file);
	file.close();
	delete writer;
	return true;
};
void OgreStudioLevel::CreateLight(int type)
{
	Ogre::Light* light = sm->createLight();
	Ogre::SceneNode* lightNode = sm->getRootSceneNode()->createChildSceneNode();
	lightNode->attachObject(light);

	light->setName("light");
	light->setType((Ogre::Light::LightTypes)type);
	switch (type)
	{
	case Ogre::Light::LT_DIRECTIONAL:
		break;
	}
};
void OgreStudioLevel::CreateDynamicObject(std::string& name)
{
	Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load(name, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

	Ogre::Item* item = sm->createItem(mesh);
	Ogre::SceneNode* node = sm->getRootSceneNode()->createChildSceneNode();
	node->attachObject((Ogre::MovableObject*)item);
};
void OgreStudioLevel::serialize(Json::Value& root)
{
	root["ogre_studio"]["version"] = OgreStudiVersion;
	root["lights"] = serialize_lights();
	root["dynamics"] = serialize_dynamic_objects();
	root["statics"] = serialize_static_objects();
};
Json::Value OgreStudioLevel::serialize_lights()
{
	Json::Value lights(Json::arrayValue);

	Ogre::SceneNode* root = sm->getRootSceneNode();
	for (size_t index = 0; index < root->numChildren(); index++)
	{
		Ogre::SceneNode* child = (Ogre::SceneNode*)root->getChild(index);
		Ogre::String name = child->getAttachedObject(0)->getName();
		if (name != "light")
			continue;
		Json::Value js_light;

		Ogre::Light* light = (Ogre::Light*)child->getAttachedObject(0);

		js_light["type"] = light->getType();
		switch (light->getType())
		{
		case Ogre::Light::LT_DIRECTIONAL:
			js_light["direction"].append(light->getDirection().x);
			js_light["direction"].append(light->getDirection().y);
			js_light["direction"].append(light->getDirection().z);
			break;
		}

		lights.append(js_light);
	}
	return lights;
};
Json::Value OgreStudioLevel::serialize_static_objects()
{
	Json::Value objects(Json::arrayValue);
	return objects;
};
Json::Value OgreStudioLevel::serialize_dynamic_objects()
{
	Json::Value objects(Json::arrayValue);
	return objects;
};