#ifndef OS_PLUGIN_FILE
#define OS_PLUGIN_FILE

#ifdef WIN32
#define OS_API __declspec(dllexport)
#else
#define OS_API __attribute__((visibility("default")))
#endif

#include<Ogre.h>

typedef enum {
	OgreStudioPlugin_Export,
	OgreStudioPlugin_Import
}OgreStudioPluginType;

typedef enum {
	OgreStudioPluginObjectType_Light,
	OgreStudioPluginObjectType_Mesh,
	OgreStudioPluginObjectType_Helper
}OgreStudioPluginObjectType;

class OgreStudioObject
{
public:
	OgreStudioObject(OgreStudioPluginObjectType _type):type(_type){}
private:
	OgreStudioPluginObjectType type;
};
class OgreStudioLight:public OgreStudioObject
{
public:
	OgreStudioLight():OgreStudioObject(OgreStudioPluginObjectType_Light){}
};
class OgreStudioNode
{
public:
	~OgreStudioNode() {
		std::vector<OgreStudioObject*>::iterator it;
		for (it = objects.begin(); it != objects.end(); it++)
			delete* it;
	}
	void attachObject(OgreStudioObject* obj) { objects.push_back(obj); }

	void setPosition(float x, float y, float z) { position[0] = x; position[1] = y; position[2] = z; }
	void setRotation(float x, float y, float z, float w) { rotation[0] = x; rotation[1] = y; rotation[2] = z; rotation[3] = w;}

	void getPosition(float& x, float& y, float& z) { x = position[0]; y = position[1]; z = position[2]; }
	void getRotation(float& x, float& y, float& z, float& w) { x = rotation[0]; y = rotation[1]; z = rotation[2]; w = rotation[3]; }
private:
	std::vector<OgreStudioObject*> objects;
	float position[3];
	float rotation[4];
};

class OgreStudioPlugin
{
public:
	OgreStudioPlugin(OgreStudioPluginType _type):type(_type) {}
	virtual ~OgreStudioPlugin() {}

	virtual const OgreStudioPluginType Type()const { return type; }
private:
	OgreStudioPluginType type;
};

class OgreStudioPluginExport :public OgreStudioPlugin
{
public:
	OgreStudioPluginExport():OgreStudioPlugin(OgreStudioPlugin_Export){}

	virtual const char* GetExtension() = 0;

	virtual bool BeginExport(const char *filename) = 0;
	virtual void EndExport() = 0;

	virtual void DoExport(OgreStudioNode* node) = 0;
};

class OgreStudioPluginImport :public OgreStudioPlugin
{
public:
	OgreStudioPluginImport() :OgreStudioPlugin(OgreStudioPlugin_Import) {}
	
	virtual const char* GetExtension() = 0;

	virtual bool BeginImport(const char* filename) = 0;
	virtual void EndImport() = 0;

	virtual size_t NumObjects() = 0;
	virtual OgreStudioNode* GetObject(size_t index) = 0;
};

extern "C"
{
	OS_API OgreStudioPlugin* OsCreatePlugin();
	OS_API void OsDestroyPlugin(OgreStudioPlugin *plugin);
};
#endif