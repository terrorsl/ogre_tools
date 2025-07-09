#include<os_plugin.h>

class TestOgreStudioExport :public OgreStudioPluginExport
{
public:
	const char* GetExtension() { return "Ogre Test Export(*.ote)"; }
	bool BeginExport(const char *filename) {
		std::string full(filename);
		full += ".ote";
		file = fopen(full.c_str(), "wb");
		if(file==0)
			return false;
		return true;
	}
	void EndExport() { fclose(file); }
	void DoExport(OgreStudioNode* node){
	}
private:
	FILE* file;
};

OgreStudioPlugin* OsCreatePlugin()
{
	TestOgreStudioExport* instance = new TestOgreStudioExport();
	return instance;
};
void OsDestroyPlugin(OgreStudioPlugin* plugin)
{
	delete plugin;
};