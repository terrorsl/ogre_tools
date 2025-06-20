#ifndef OS_LEVEL_FILE
#define OS_LEVEL_FILE

#include<string>
#include<json/json.h>

#include<Ogre.h>

#define OgreStudiVersion 100

class OgreStudioLevel
{
public:
	OgreStudioLevel(std::string& name, Ogre::SceneManager *_sm);
	
	bool Load(std::string &fname);
	bool Save();

	Ogre::SceneManager* GetSceneManager() { return sm; }
	//bool export();

	void CreateLight(int type);
	void CreateDynamicObject(std::string& name);
	void CreateStaticObject(std::string& name);
private:
	void serialize(Json::Value& root);
	Json::Value serialize_lights();
	Json::Value serialize_static_objects();
	Json::Value serialize_dynamic_objects();

	std::string filename;
	Ogre::SceneManager* sm;
};
#endif