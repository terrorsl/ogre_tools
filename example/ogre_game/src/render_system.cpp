#include"render_system.h"
#include<OgreAbiUtils.h>

#include<OgreHlmsManager.h>
#include<Pbs/OgreHlmsPbs.h>

bool RenderSystem::Initialise(Ogre::String &app_name)
{
	quit = false;
	accumTimeSinceLastLogicFrame = 0;

	Ogre::String plugin = "plugins_" + app_name + OGRE_BUILD_SUFFIX".cfg";
	Ogre::String config = app_name + ".cfg";
	Ogre::String log = app_name + ".log";

	Ogre::AbiCookie cookie = Ogre::generateAbiCookie();
	root = new Ogre::Root(&cookie, plugin, config, log, app_name);

	if (root->restoreConfig() == false)
	{
		Ogre::RenderSystem* rs = root->getAvailableRenderers()[0];
		rs->setConfigOption("Full Screen", "No");
		rs->setConfigOption("VSync", "Yes");
		root->setRenderSystem(rs);
		root->saveConfig();
	}
	renderWindow = root->initialise(true);

	initialiseHLMS();

	Ogre::WindowEventUtilities::addWindowEventListener(renderWindow, this);
	return true;
};
void RenderSystem::DeInitialize()
{
	delete root;
};
void RenderSystem::windowClosed(Ogre::Window* rw)
{
	quit = true;
};
bool RenderSystem::initialiseHLMS()
{
	Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();

	Ogre::String mainFolderPath;
	Ogre::StringVector libraryFoldersPaths;
	Ogre::StringVector::const_iterator libraryFolderPathIt;
	Ogre::StringVector::const_iterator libraryFolderPathEn;

	/*Ogre::HlmsUnlit* hlmsUnlit = 0;
	Ogre::HlmsUnlit::getDefaultPaths(mainFolderPath, libraryFoldersPaths);
	Ogre::Archive* archiveUnlit = archiveManager.load(mainFolderPath, "FileSystem", true);
	Ogre::ArchiveVec archiveUnlitLibraryFolders;

	libraryFolderPathIt = libraryFoldersPaths.begin();
	libraryFolderPathEn = libraryFoldersPaths.end();
	while (libraryFolderPathIt != libraryFolderPathEn)
	{
		Ogre::Archive* archiveLibrary =
			archiveManager.load(*libraryFolderPathIt, "FileSystem", true);
		archiveUnlitLibraryFolders.push_back(archiveLibrary);
		++libraryFolderPathIt;
	}
	hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit(archiveUnlit, &archiveUnlitLibraryFolders);
	Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsUnlit);*/

	Ogre::HlmsPbs* hlmsPbs = 0;
	Ogre::HlmsPbs::getDefaultPaths(mainFolderPath, libraryFoldersPaths);
	Ogre::Archive* archivePbs = archiveManager.load(mainFolderPath, "FileSystem", true);
	Ogre::ArchiveVec archivePbsLibraryFolders;

	libraryFolderPathIt = libraryFoldersPaths.begin();
	libraryFolderPathEn = libraryFoldersPaths.end();
	while (libraryFolderPathIt != libraryFolderPathEn)
	{
		Ogre::Archive* archiveLibrary =
			archiveManager.load(*libraryFolderPathIt, "FileSystem", true);
		archivePbsLibraryFolders.push_back(archiveLibrary);
		++libraryFolderPathIt;
	}

	hlmsPbs = OGRE_NEW Ogre::HlmsPbs(archivePbs, &archivePbsLibraryFolders);
	Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsPbs);
	return true;
};
void RenderSystem::processIncomingMessage(MessageId messageId, const void* data)
{
	switch (messageId)
	{
	case LOGICFRAME_FINISHED:
		{
			Ogre::uint32 newIdx = *reinterpret_cast<const Ogre::uint32*>(data);
			if (newIdx != std::numeric_limits<Ogre::uint32>::max())
			{
				accumTimeSinceLastLogicFrame = 0;

				// Tell the LogicSystem we're no longer using the index previous to the current one.
				this->queueSendMessage(
					notify_system, LOGICFRAME_FINISHED, mCurrentTransformIdx);
				//	(mCurrentTransformIdx + NUM_GAME_ENTITY_BUFFERS - 1) % NUM_GAME_ENTITY_BUFFERS);

				//assert((mCurrentTransformIdx + 1) % NUM_GAME_ENTITY_BUFFERS == newIdx &&
				//	"Graphics is receiving indices out of order!!!");

				// Get the new index the LogicSystem is telling us to use.
				mCurrentTransformIdx = newIdx;
			}
		}
		break;
	case GAME_ENTITY_ADDED:
		//gameEntityAdded(reinterpret_cast<const GameEntityManager::CreatedGameEntity*>(data));
		break;
	case GAME_ENTITY_REMOVED:
		//gameEntityRemoved(*reinterpret_cast<GameEntity* const*>(data));
		break;
	case GAME_ENTITY_SCHEDULED_FOR_REMOVAL_SLOT:
		// Acknowledge/notify back that we're done with this slot.
		this->queueSendMessage(notify_system, GAME_ENTITY_SCHEDULED_FOR_REMOVAL_SLOT, *reinterpret_cast<const Ogre::uint32*>(data));
		break;
	default:
		break;
	}
}
Ogre::SceneManager* RenderSystem::Create(Ogre::String &name)
{
	return root->createSceneManager(Ogre::ST_GENERIC, 1);
};