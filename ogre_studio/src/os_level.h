#ifndef OS_LEVEL_FILE
#define OS_LEVEL_FILE

#include<string>
#include<json/json.h>

#include<Ogre.h>

#define OgreStudiVersion 100

typedef enum {
	OgreStudioObjectType_Light,
	OgreStudioObjectType_Mesh,
	OgreStudioObjectType_Helper
}OgreStudioObjectType;

class OgreStudioLevelCallback
{
public:
	virtual void CreateNode(Ogre::SceneNode *node, OgreStudioObjectType type) = 0;
	virtual void ReceiveMessage(int type, const std::string& message) = 0;
};

class OgreStudioLevel
{
public:
	OgreStudioLevel(std::string& name, Ogre::Root *_root, Ogre::SceneManager *_sm, OgreStudioLevelCallback* _clb);
	~OgreStudioLevel();
	
	bool Load(std::string &fname);
	bool Save();

	void resizeCamera(unsigned long width, unsigned long height);
	void rotateCamera(float dx, float dy);
	void moveCamera(float step);

	Ogre::SceneManager* GetSceneManager() { return sm; }
	//bool export();

	Ogre::SceneNode *CreateLight(int type);
	Ogre::SceneNode* CreateStaticObject(std::string& name);
	Ogre::SceneNode* CreateDynamicObject(std::string& name);
	Ogre::SceneNode* CreateNPC();
	void CreateTerrain();

	void SelectNode(Ogre::SceneNode* node);
private:
	void set_object_type(Ogre::SceneNode* node, OgreStudioObjectType type);

	void serialize(Json::Value& root);
	Json::Value serialize_light(Ogre::SceneNode* node);
	Json::Value serialize_object(Ogre::SceneNode* node);

	void serialize_vector(std::string name, const Ogre::Vector3& vec, Json::Value& node);
	void serialize_vector(std::string name, const Ogre::Vector4& vec, Json::Value& node);
	void serialize_quaternion(std::string name, const Ogre::Quaternion& vec, Json::Value& node);

	Json::Value serialize_static_objects();
	Json::Value serialize_dynamic_objects();

	void deserialize(Json::Value& root);
	void deserialize_lights(Json::Value &root);
	void deserialize_objects(Json::Value& root);

	void deserialize_vector(std::string name, Ogre::Vector3& vec, Json::Value& node);
	void deserialize_quaternion(std::string name, Ogre::Quaternion& vec, Json::Value& node);

	std::string filename;
	Ogre::Root* root;
	Ogre::SceneManager* sm;
	Ogre::WireAabb* selectedNode;

	OgreStudioLevelCallback* callback;
};
#endif