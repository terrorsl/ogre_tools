#ifndef OS_PLUGIN_MANAGER_FILE
#define OS_PLUGIN_MANAGER_FILE

#include"os_plugin.h"
#include<map>
#include<vector>

class OgreStudioPluginManager
{
public:
	void Initialise();
	void Deinitialise();

	std::vector<OgreStudioPluginExport*> GetExports();
private:
	bool Load(const char *name);
	void Free(OgreStudioPlugin*);

	std::map<OgreStudioPlugin*, void*> plugins;
};
#endif