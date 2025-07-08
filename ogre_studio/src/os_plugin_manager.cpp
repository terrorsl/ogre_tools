#include"os_plugin_manager.h"
#include<qdir.h>

typedef OgreStudioPlugin* (*funcOsCreatePlugin)();
typedef void (*funcOsDestroyPlugin)(OgreStudioPlugin*);
#ifdef WIN32
#include<Windows.h>

bool OgreStudioPluginManager::Load(const char* name)
{
	HMODULE module = LoadLibraryA(name);
	if (module == 0)
		return false;

	funcOsCreatePlugin OsCreatePlugin = (funcOsCreatePlugin)GetProcAddress(module, "OsCreatePlugin");
	funcOsDestroyPlugin OsDestroyPlugin = (funcOsDestroyPlugin)GetProcAddress(module, "OsDestroyPlugin");
	if (OsCreatePlugin == 0 || OsDestroyPlugin == 0)
		return false;

	OgreStudioPlugin* plugin = OsCreatePlugin();

	plugins.insert(std::make_pair(plugin, module));
	return true;
};
void OgreStudioPluginManager::Free(OgreStudioPlugin *plugin)
{
	std::map<OgreStudioPlugin*, void*>::iterator it = plugins.find(plugin);
	if (it == plugins.end())
		return;
	funcOsDestroyPlugin OsDestroyPlugin=(funcOsDestroyPlugin)GetProcAddress((HMODULE)it->second, "OsDestroyPlugin");
	OsDestroyPlugin(plugin);
	plugins.erase(it);
}
#else
#endif

void OgreStudioPluginManager::Initialise()
{
	QDir dir("plugins");
	QFileInfoList infos = dir.entryInfoList(QDir::Files);
	for (size_t index = 0; index < infos.count(); index++)
	{
		if (infos[index].suffix() != "dll")
			continue;
		Load(infos[index].absoluteFilePath().toLocal8Bit().data());
	}
};
void OgreStudioPluginManager::Deinitialise()
{
	while(plugins.size())
	{
		Free(plugins.begin()->first);
	}
};
std::vector<OgreStudioPluginExport*> OgreStudioPluginManager::GetExports()
{
	std::vector<OgreStudioPluginExport*> ret;

	std::map<OgreStudioPlugin*, void*>::iterator it;
	for (it = plugins.begin(); it != plugins.end(); it++)
	{
		if (it->first->Type() == OgreStudioPlugin_Export)
			ret.push_back((OgreStudioPluginExport*)it->first);
	}
	return ret;
};