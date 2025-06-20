#include"qogrewidget.h"
#include<OgreWindow.h>
#include<qevent.h>
#include<OgreAbiUtils.h>
#include "Compositor/OgreCompositorManager2.h"
#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"

#include<OgreHlmsManager.h>
#include<Hlms/Pbs/OgreHlmsPbs.h>
#include<Hlms/Pbs/OgreHlmsPbsDatablock.h>
#include<Hlms/Unlit/OgreHlmsUnlit.h>

#include<OgreItem.h>
#include<OgreMesh2.h>
#include<OgreMesh2Serializer.h>
#include<OgreSubMesh2.h>
#include<OgreMatrix3.h>

#include<OgreTextureGpuManager.h>

#include "Vao/OgreVaoManager.h"
#include "Vao/OgreVertexArrayObject.h"

struct Vertex
{
	float position[3];
	float normal[3];
	float tex[2];
};

struct Face
{
	unsigned long a,b,c;
};

QOgreWidget::QOgreWidget(QWidget* parent) :QWidget(parent), mouse_down(false), meshNode(0)
{
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAttribute(Qt::WA_PaintOnScreen, true);
	setFocusPolicy(Qt::StrongFocus);
	//installEventFilter(this);
	setMouseTracking(true);
};
QOgreWidget::~QOgreWidget()
{
	delete root;
};
void QOgreWidget::Initialize()
{
	const Ogre::AbiCookie abiCookie = Ogre::generateAbiCookie();
	root = OGRE_NEW Ogre::Root(&abiCookie);

	const Ogre::RenderSystemList& rsList = root->getAvailableRenderers();
	Ogre::RenderSystem* rs = rsList[0];
	if (rs == 0)
	{
		if (root->restoreConfig() == false)
		{

		}
	}

	QString dimensions = QString("%1 x %2").arg(this->width()).arg(this->height());
	rs->setConfigOption("Video Mode", dimensions.toStdString());
	rs->setConfigOption("Full Screen", "No");
	rs->setConfigOption("VSync", "Yes");
	root->setRenderSystem(rs);
	root->initialise(false);

	Ogre::NameValuePairList parameters;
	parameters["externalWindowHandle"] = Ogre::StringConverter::toString((size_t)(this->winId()));
	//parameters["parentWindowHandle"] = Ogre::StringConverter::toString((size_t)(this->winId()));

	window = root->createRenderWindow("QT Window",
		this->width(),
		this->height(),
		false,
		&parameters);

	window->setHidden(false);

	initialiseHLMS();

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./", "FileSystem", "General");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(false);
	
	sm = root->createSceneManager(Ogre::ST_GENERIC, 1);

	Ogre::Light* light = sm->createLight();
	Ogre::SceneNode* lightNode = sm->getRootSceneNode()->createChildSceneNode();
	lightNode->attachObject(light);

	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(Ogre::Vector3(-1, -1, -1).normalisedCopy());

	// Create & setup camera
	camera = sm->createCamera("Main Camera");

	// Position it at 500 in Z direction
	camera->setPosition(Ogre::Vector3(0, 5, 15));
	// Look back along -Z
	camera->lookAt(Ogre::Vector3(0, 0, 0));
	camera->setNearClipDistance(0.2f);
	camera->setFarClipDistance(1000.0f);
	camera->setAspectRatio((float)width() / (float)height());
	//camera->setAutoAspectRatio(true);

	//Ogre::Light *light = sm->createLight();
	//light->setType(Ogre::Light::LT_DIRECTIONAL);

	// Setup a basic compositor with a blue clear colour
	Ogre::CompositorManager2* compositorManager = root->getCompositorManager2();
	const Ogre::String workspaceName("Demo Workspace");
	const Ogre::ColourValue backgroundColour(0.2f, 0.4f, 0.6f);
	compositorManager->createBasicWorkspaceDef(workspaceName, backgroundColour, Ogre::IdString());
	compositorManager->addWorkspace(sm, window->getTexture(), camera, workspaceName, true);

	//LoadMesh("Cottage_FREE.mesh");

	//root->renderOneFrame();
	/*Ogre::HlmsPbs* hlmsPbs = (Ogre::HlmsPbs*)root->getHlmsManager()->getHlms(Ogre::HLMS_PBS);

	Ogre::HlmsMacroblock refMacroblock;
	const Ogre::HlmsMacroblock* newMacroblock;
	newMacroblock = root->getHlmsManager()->getMacroblock(refMacroblock);

	Ogre::HlmsBlendblock refBlendblock;

	Ogre::HlmsDatablock *datablock = hlmsPbs->createDatablock("testMaterial", "testMaterial", *newMacroblock, refBlendblock,Ogre::HlmsParamVec());

	root->getHlmsManager()->saveMaterial(datablock, "test.material.json", 0, "");*/
};
void QOgreWidget::initialiseHLMS()
{
	Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();

	Ogre::String mainFolderPath;
	Ogre::StringVector libraryFoldersPaths;
	Ogre::StringVector::const_iterator libraryFolderPathIt;
	Ogre::StringVector::const_iterator libraryFolderPathEn;

	Ogre::HlmsUnlit* hlmsUnlit = 0;
	Ogre::HlmsPbs* hlmsPbs = 0;

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
	Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsUnlit);

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
};

void QOgreWidget::render(QPainter* painter)
{
	int k = 0;
};
void QOgreWidget::paintEvent(QPaintEvent* event)
{
	//root->renderOneFrame();
}
QPaintEngine * QOgreWidget::paintEngine() const
{
	return 0;
}
void QOgreWidget::render()
{
	Ogre::WindowEventUtilities::messagePump();
	if(root->isInitialised())
		root->renderOneFrame();
};
bool QOgreWidget::eventFilter(QObject* target, QEvent* event)
{
	if (target == this)
	{
		if (event->type() == QEvent::Resize)
		{
			QResizeEvent* re = (QResizeEvent*)event;
			QSize size = re->size();
			window->windowMovedOrResized();
			//window->requestResolution(size.width(), size.height());
			/*if (isExposed() && m_ogreWindow != NULL)
			{
				m_ogreWindow->resize(this->width(), this->height());
			}*/
		}
	}
	return false;
};
/*void QOgreWidget::exposeEvent(QExposeEvent* event)
{
	int k = 0;
	//if(isExposed())
};*/
bool QOgreWidget::event(QEvent* event)
{
	switch (event->type())
	{
	case QEvent::MouseMove:
		{
			int k = 0;
		}
		break;
	case QEvent::Resize:
		{
			QResizeEvent* re = (QResizeEvent*)event;
			QSize size = re->size();
			//window->requestResolution(size.width(), size.height());
			window->windowMovedOrResized();
			camera->setAspectRatio((float)size.width() / (float)size.height());
		}
		break;
	case QEvent::UpdateLater:
		render();
		break;
	case QEvent::UpdateRequest:
		return true;
	}
	return QWidget::event(event);
};
void QOgreWidget::mouseMoveEvent(QMouseEvent* e)
{
	if (mouse_down == false)
		return;
	float mx = (e->pos().x() - mouse_position.x()) / (float)width();
	float my = (e->pos().y() - mouse_position.y()) / (float)height();

	Ogre::Quaternion rotation = meshNode->getOrientation();

	Ogre::Quaternion r = Ogre::Quaternion(Ogre::Radian(mx), Ogre::Vector3::UNIT_Y) * Ogre::Quaternion(Ogre::Radian(my), Ogre::Vector3::UNIT_X);
	rotation = r * rotation;
	meshNode->setOrientation(rotation);

	mouse_position = e->pos();
};
void QOgreWidget::mousePressEvent(QMouseEvent* e)
{
	mouse_down = true;
	mouse_position = e->pos();
};
void QOgreWidget::mouseReleaseEvent(QMouseEvent* e)
{
	mouse_down = false;
};
void QOgreWidget::wheelEvent(QWheelEvent* e)
{
	if (e->angleDelta().y() > 0)
	{
		camera->move(Ogre::Vector3(0, 0, 1));
	}
	else
	{
		camera->move(Ogre::Vector3(0, 0, -1));
	}
};
void QOgreWidget::keyPressEvent(QKeyEvent* e)
{
};
void QOgreWidget::keyReleaseEvent(QKeyEvent* e)
{
};
void QOgreWidget::resizeEvent(QResizeEvent* e)
{
	camera->setAspectRatio((float)e->size().width() / (float)e->size().height());
	window->windowMovedOrResized();
};
bool QOgreWidget::LoadMesh(QString filename)
{
	if (meshNode)
	{
		sm->getRootSceneNode()->removeAndDestroyChild(meshNode);
		meshNode = 0;
	}

	Ogre::MeshPtr mesh;
	mesh = LoadMeshV2(filename);
	if (mesh.isNull())
	{
		mesh = LoadMeshV1(filename);
	}

	if (mesh.isNull() == false)
	{
		Ogre::Item* item = sm->createItem(mesh);

		//item->setDatablock("Cottage_FREE");

		meshNode = sm->getRootSceneNode(Ogre::SCENE_DYNAMIC)
			->createChildSceneNode(Ogre::SCENE_DYNAMIC);

		meshNode->attachObject((Ogre::MovableObject*)item);

		camera->setPosition(Ogre::Vector3(0, mesh->getBoundingSphereRadius()/2.f,2*mesh->getBoundingSphereRadius()));
	}
	//sm->createEntity()
	return true;
};
Ogre::MeshPtr QOgreWidget::LoadMeshV2(QString filename)
{
	int pos = filename.lastIndexOf('/');
	if(pos!=-1)
		filename.remove(0, pos+1);

	Ogre::MeshPtr mesh;
	try
	{
		mesh = Ogre::MeshManager::getSingleton().load(filename.toStdString(), Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
	}
	catch (Ogre::Exception& ex)
	{
		printf("%s\n", ex.what());
	}
	return mesh;
};
Ogre::MeshPtr QOgreWidget::LoadMeshV1(QString filename)
{
	int pos = filename.lastIndexOf('/');
	if (pos != -1)
		filename.remove(0, pos + 1);

	Ogre::v1::MeshPtr mesh1;
	Ogre::MeshPtr mesh;
	try
	{
		mesh1 = Ogre::v1::MeshManager::getSingleton().load(filename.toStdString(), Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
		mesh = Ogre::MeshManager::getSingleton().createByImportingV1("", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, mesh1.get(), true, true, true);
		mesh1->unload();

		/*Ogre::AxisAlignedBox aabb = mesh1->getBounds();
		Ogre::Aabb _aabb(aabb.getCenter(), aabb.getHalfSize());
		mesh->_setBoundingSphereRadius(mesh1->getBoundingSphereRadius());
		mesh->_setBounds(_aabb);

		Ogre::MeshSerializer mesh_serializer(0);
		mesh_serializer.exportMesh(mesh.get(),"test.mesh");*/
	}
	catch (Ogre::Exception& ex)
	{
		//Ogre::v1::MeshManager::getSingleton().removeUnreferencedResources();
		mesh1 = Ogre::v1::MeshManager::getSingleton().getByName(filename.toStdString(), Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
		Ogre::v1::MeshManager::getSingleton().remove(mesh1);
		mesh = Ogre::MeshManager::getSingleton().getByName(filename.toStdString(), Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
		Ogre::MeshManager::getSingleton().remove(mesh);
		mesh.reset();
		printf("%s\n", ex.what());
	}
	return mesh;
};