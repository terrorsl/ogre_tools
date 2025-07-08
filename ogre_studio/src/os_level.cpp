#include"os_level.h"
#include<fstream>

#include<OgreMeshManager2.h>
#include<OgreMesh2.h>
#include<OgreItem.h>
#include<OgreWireAabb.h>

#include<OgreDecal.h>
#include<OgreTextureGpuManager.h>

#include<ParticleSystem/OgreBillboardSet2.h>

OgreStudioLevel::OgreStudioLevel(std::string& name, Ogre::Root *_root, Ogre::SceneManager* _sm,
	OgreStudioLevelCallback* _clb):filename(name),sm(_sm),root(_root), selectedNode(0),callback(_clb)
{
	/*Ogre::Light* light = sm->createLight();
	Ogre::SceneNode* lightNode = sm->getRootSceneNode()->createChildSceneNode();
	lightNode->attachObject(light);
	
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(Ogre::Vector3(-1, -1, -1).normalisedCopy());
	light->setName("light");
	light->setPowerScale(Ogre::Math::PI);  // Since we don't do HDR, counter the PBS' division by
	// PI
	light->setCastShadows(true);
	*/
	//sm->setAmbientLight(Ogre::ColourValue(0.3f, 0.5f, 0.7f) * 0.1f * 0.75f, Ogre::ColourValue(0.6f, 0.45f, 0.3f) * 0.065f * 0.75f,
	//	-Ogre::Vector3(-1,-1,-1).normalisedCopy() + Ogre::Vector3::UNIT_Y * 0.2f);

	Ogre::Camera* camera = sm->createCamera("OgreStudioCamera");
	sm->setForward3D(true, 4, 4, 4, 32, 0.01, 700);
	//sm->setForwardClustered(true, 16, 8, 24, 96, 0, 0, 5, 500);

	//CreateNPC();

	Ogre::TextureGpuManager* textureManager = root->getRenderSystem()->getTextureGpuManager();
	//textureManager->createTexture()
	sm->setSky(true, Ogre::SceneManager::SkyCubemap, "stormy.dds", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

	selectedNode = sm->createWireAabb();
	//set_object_type((Ogre::SceneNode*)selectedNode, OgreStudioObjectType_Helper);
};
OgreStudioLevel::~OgreStudioLevel()
{
	/*if (selectedNode)
	{
		selectedNode->track(0);
		sm->destroyWireAabb(selectedNode);
	}*/
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
		callback->ReceiveMessage(0, err);
		return false;
	}
	file.close();
	filename = fname;

	deserialize(root);
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
void OgreStudioLevel::SelectNode(Ogre::SceneNode* node)
{
	Ogre::MovableObject *mo = node->getAttachedObject(0);

	Ogre::Any type = node->getUserObjectBindings().getUserAny("type");
	if (Ogre::any_cast<OgreStudioObjectType>(type) == OgreStudioObjectType_Light)
	{
		Ogre::Light* light = (Ogre::Light*)node->getAttachedObject(0);
		if(light->getType()==Ogre::Light::LT_POINT || light->getType() == Ogre::Light::LT_SPOTLIGHT)
			selectedNode->track(mo);
	}
	else
	{
		selectedNode->track(mo);
	}
	/*if (mo->getName() != "light")
		selectedNode->track(mo);
	else
	{

	}*/
};
Ogre::SceneNode* OgreStudioLevel::CreateLight(int type)
{
	Ogre::Light* light = sm->createLight();
	Ogre::SceneNode* lightNode = sm->getRootSceneNode()->createChildSceneNode();
	lightNode->setName("light");
	lightNode->attachObject(light);
	set_object_type(lightNode, OgreStudioObjectType_Light);

	light->setType((Ogre::Light::LightTypes)type);
	switch (type)
	{
	case Ogre::Light::LT_DIRECTIONAL:
		light->setDirection(Ogre::Vector3(-1, -1, -1).normalisedCopy());
		break;
	case Ogre::Light::LT_POINT:
		light->setAttenuationBasedOnRadius(300, 0.001);
		break;
	}
	light->setPowerScale(Ogre::Math::PI);
	light->setCastShadows(true);
	return lightNode;
};
Ogre::SceneNode* OgreStudioLevel::CreateNPC()
{
	Ogre::v1::BillboardSet* bbs = sm->createBillboardSet(1);
	bbs->setDefaultDimensions(1, 1);

	bbs->createBillboard(Ogre::Vector3(0, 10, 0));

	Ogre::SceneNode* sceneNode = sm->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_DYNAMIC);
	sceneNode->attachObject(bbs);
	/*Ogre::Decal* decal = sm->createDecal();
	Ogre::SceneNode* sceneNode = sm->getRootSceneNode()->createChildSceneNode();
	sceneNode->attachObject(decal);
	sceneNode->setPosition(Ogre::Vector3(0, 0.4f, 0));
	sceneNode->setOrientation(
		Ogre::Quaternion(Ogre::Degree(45.0f), Ogre::Vector3::UNIT_Y));
	sceneNode->setScale(Ogre::Vector3(10.0f));
	sm->createWireAabb()->track(decal);

	Ogre::TextureGpuManager* textureManager = root->getRenderSystem()->getTextureGpuManager();

	const Ogre::uint32 decalDiffuseId = 1;
	Ogre::TextureGpu* textureDiff, * textureNorm = 0;
	textureDiff = textureManager->createOrRetrieveTexture(
		"Cottage_Clean_Base_Color.PNG", Ogre::GpuPageOutStrategy::Discard,
		Ogre::CommonTextureTypes::Diffuse,
		Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, decalDiffuseId);

	textureDiff->scheduleTransitionTo(Ogre::GpuResidency::Resident);
	decal->setDiffuseTexture(textureDiff);
	sm->setDecalsDiffuse(textureDiff);*/
	return 0;
};
Ogre::SceneNode* OgreStudioLevel::CreateDynamicObject(std::string& name)
{
	Ogre::Item* item = sm->createItem(name);
	item->getMesh()->isLoaded();
	//Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load(name, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

	//Ogre::Item* item = sm->createItem(mesh);
	item->setCastShadows(true);
	Ogre::SceneNode* node = sm->getRootSceneNode()->createChildSceneNode();
	node->attachObject((Ogre::MovableObject*)item);
	node->setName("object");
	set_object_type(node, OgreStudioObjectType_Mesh);
	return node;
};
void OgreStudioLevel::CreateTerrain()
{
#if 0
	// Render terrain after most objects, to improve performance by taking advantage of early Z
	Ogre::Terra *mTerra =
		new Ogre::Terra(Ogre::Id::generateNewId<Ogre::MovableObject>(),
			&sceneManager->_getEntityMemoryManager(Ogre::SCENE_STATIC), sceneManager,
			11u, root->getCompositorManager2(), mGraphicsSystem->getCamera(), false);
	mTerra->setCastShadows(false);

	// mTerra->load( "Heightmap.png", Ogre::Vector3::ZERO, Ogre::Vector3( 256.0f, 1.0f, 256.0f ),
	// false ); mTerra->load( "Heightmap.png", Ogre::Vector3( 64.0f, 0, 64.0f ), Ogre::Vector3(
	// 128.0f, 5.0f, 128.0f ), false ); mTerra->load( "Heightmap.png", Ogre::Vector3( 64.0f, 0, 64.0f
	// ), Ogre::Vector3( 1024.0f, 5.0f, 1024.0f ), false ); mTerra->load( "Heightmap.png",
	// Ogre::Vector3( 64.0f, 0, 64.0f ), Ogre::Vector3( 4096.0f * 4, 15.0f * 64.0f*4, 4096.0f * 4 ),
	// false );
	mTerra->load("Heightmap.png", Ogre::Vector3(64.0f, 4096.0f * 0.5f, 64.0f),
		Ogre::Vector3(4096.0f, 4096.0f, 4096.0f), false, false);
	// mTerra->load( "Heightmap.png", Ogre::Vector3( 64.0f, 4096.0f * 0.5f, 64.0f ), Ogre::Vector3(
	// 14096.0f, 14096.0f, 14096.0f ), false );

	Ogre::SceneNode* rootNode = sceneManager->getRootSceneNode(Ogre::SCENE_STATIC);
	Ogre::SceneNode* sceneNode = rootNode->createChildSceneNode(Ogre::SCENE_STATIC);
	sceneNode->attachObject(mTerra);

	Ogre::HlmsManager* hlmsManager = root->getHlmsManager();
	Ogre::HlmsDatablock* datablock = hlmsManager->getDatablock("TerraExampleMaterial");
	//        Ogre::HlmsDatablock *datablock = hlmsManager->getHlms( Ogre::HLMS_USER3
	//        )->getDefaultDatablock(); Ogre::HlmsMacroblock macroblock; macroblock.mPolygonMode =
	//        Ogre::PM_WIREFRAME;
	// datablock->setMacroblock( macroblock );
	mTerra->setDatablock(datablock);

	{
		mHlmsPbsTerraShadows = new Ogre::HlmsPbsTerraShadows();
		mHlmsPbsTerraShadows->setTerra(mTerra);
		// Set the PBS listener so regular objects also receive terrain shadows
		Ogre::Hlms* hlmsPbs = root->getHlmsManager()->getHlms(Ogre::HLMS_PBS);
		hlmsPbs->setListener(mHlmsPbsTerraShadows);
	}
#endif
};
void OgreStudioLevel::resizeCamera(unsigned long width, unsigned long height)
{
	Ogre::Camera* camera = sm->findCameraNoThrow("OgreStudioCamera");
	camera->setAspectRatio((float)width / (float)height);
};
void OgreStudioLevel::rotateCamera(float dx, float dy)
{
	Ogre::Camera* camera = sm->findCameraNoThrow("OgreStudioCamera");
	camera->pitch(Ogre::Radian(dx));
	camera->yaw(Ogre::Radian(dy));
};
void OgreStudioLevel::moveCamera(float value)
{
	Ogre::Camera* camera = sm->findCameraNoThrow("OgreStudioCamera");
	camera->move(Ogre::Vector3(0, 0, value));
};
void OgreStudioLevel::set_object_type(Ogre::SceneNode* node, OgreStudioObjectType type)
{
	Ogre::Any value(type);
	node->getUserObjectBindings().setUserAny("type", value);
};
void OgreStudioLevel::serialize(Json::Value& root)
{
	root["ogre_studio"]["version"] = OgreStudiVersion;

	Json::Value lights(Json::arrayValue);
	Json::Value objects(Json::arrayValue);

	Ogre::SceneNode* _root = sm->getRootSceneNode();
	for (size_t index = 0; index < _root->numChildren(); index++)
	{
		Ogre::SceneNode* child = (Ogre::SceneNode*)_root->getChild(index);
		Ogre::Any type = child->getUserObjectBindings().getUserAny("type");
		if (type.isEmpty())
		{
			callback->ReceiveMessage(0, "object unknown type");
			continue;
		}
		switch (Ogre::any_cast<OgreStudioObjectType>(type))
		{
		case OgreStudioObjectType_Light:
			lights.append(serialize_light(child));
			break;
		case OgreStudioObjectType_Mesh:
			objects.append(serialize_object(child));
			break;
		}
	}

	root["lights"] = lights;
	root["objects"] = objects;
	root["dynamics"] = serialize_dynamic_objects();
	root["statics"] = serialize_static_objects();
};
Json::Value OgreStudioLevel::serialize_light(Ogre::SceneNode* node)
{
	Json::Value js_light;

	Ogre::Light* light = (Ogre::Light*)node->getAttachedObject(0);

	js_light["type"] = light->getType();
	switch (light->getType())
	{
	case Ogre::Light::LT_DIRECTIONAL:
		js_light["direction"].append(light->getDirection().x);
		js_light["direction"].append(light->getDirection().y);
		js_light["direction"].append(light->getDirection().z);
		break;
	default:
		serialize_vector("position", node->getPosition(), js_light);
		/*js_light["position"].append(node->getPosition().x);
		js_light["position"].append(node->getPosition().y);
		js_light["position"].append(node->getPosition().z);*/
		break;
	}

	Ogre::ColourValue color = light->getDiffuseColour();
	js_light["diffuse"].append(color.r);
	js_light["diffuse"].append(color.g);
	js_light["diffuse"].append(color.b);
	js_light["diffuse"].append(color.a);
	return js_light;
};
Json::Value OgreStudioLevel::serialize_object(Ogre::SceneNode* node)
{
	Json::Value js_obj;
	Ogre::Item *obj = (Ogre::Item*)node->getAttachedObject(0);
	
	js_obj["name"] = node->getName();
	js_obj["mesh"] = obj->getMesh()->getName();

	//js_obj["static"] = obj->isStatic();

	serialize_vector("position", node->getPosition(), js_obj);
	serialize_quaternion("rotation", node->getOrientation(), js_obj);
	return js_obj;
};
void OgreStudioLevel::serialize_vector(std::string name, const Ogre::Vector3& vec, Json::Value &node)
{
	node[name].append(vec.x);
	node[name].append(vec.y);
	node[name].append(vec.z);
};
void OgreStudioLevel::serialize_vector(std::string name, const Ogre::Vector4& vec, Json::Value& node)
{
	node[name].append(vec.x);
	node[name].append(vec.y);
	node[name].append(vec.z);
	node[name].append(vec.w);
};
void OgreStudioLevel::serialize_quaternion(std::string name, const Ogre::Quaternion& vec, Json::Value& node)
{
	node[name].append(vec.x);
	node[name].append(vec.y);
	node[name].append(vec.z);
	node[name].append(vec.w);
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
void OgreStudioLevel::deserialize(Json::Value& root)
{
	if (root["ogre_studio"]["version"] != OgreStudiVersion)
	{
		callback->ReceiveMessage(0, "bad version");
		return;
	}
	
	deserialize_lights(root["lights"]);
	deserialize_objects(root["objects"]);
	//root["dynamics"] = serialize_dynamic_objects();
	//root["statics"] = serialize_static_objects();
};
void OgreStudioLevel::deserialize_lights(Json::Value& root)
{
	for (Json::Value::ArrayIndex index = 0; index < root.size(); index++)
	{
		Json::Value js_light = root[index];

		Ogre::Light *light = sm->createLight();
		Ogre::SceneNode *node = sm->getRootSceneNode()->createChildSceneNode();
		node->attachObject(light);

		set_object_type(node, OgreStudioObjectType_Light);
		
		callback->CreateNode(node, OgreStudioObjectType::OgreStudioObjectType_Light);

		Ogre::ColourValue color(js_light["diffuse"][0].asFloat(), js_light["diffuse"][1].asFloat(), js_light["diffuse"][2].asFloat(), js_light["diffuse"][3].asFloat());
		light->setDiffuseColour(color);

		Ogre::Light::LightTypes type = (Ogre::Light::LightTypes)js_light["type"].asUInt();
		light->setType(type);
		switch (type)
		{
		case Ogre::Light::LT_DIRECTIONAL:
			{
				Ogre::Vector3 p(js_light["direction"][0].asFloat(), js_light["direction"][1].asFloat(), js_light["direction"][2].asFloat());
				light->setDirection(p);
			}
			break;
		}
		light->setPowerScale(Ogre::Math::PI);
	}
};
void OgreStudioLevel::deserialize_objects(Json::Value& root)
{
	for (Json::Value::ArrayIndex index = 0; index < root.size(); index++)
	{
		Json::Value js_obj = root[index];

		std::string str = js_obj["mesh"].asString();
		Ogre::SceneNode *node = CreateDynamicObject(str);
		node->setName(js_obj["name"].asString());

		Ogre::Vector3 v3;
		deserialize_vector("position", v3, js_obj);
		node->setPosition(v3);

		Ogre::Quaternion q;
		deserialize_quaternion("rotation", q, js_obj);
		node->setOrientation(q);

		callback->CreateNode(node, OgreStudioObjectType::OgreStudioObjectType_Mesh);
	}
};
void OgreStudioLevel::deserialize_vector(std::string name, Ogre::Vector3& vec, Json::Value& node)
{
	vec.x = node[name][0].asFloat();
	vec.y = node[name][1].asFloat();
	vec.z = node[name][2].asFloat();
};
void OgreStudioLevel::deserialize_quaternion(std::string name, Ogre::Quaternion& vec, Json::Value& node)
{
	vec.x = node[name][0].asFloat();
	vec.y = node[name][1].asFloat();
	vec.z = node[name][2].asFloat();
	vec.w = node[name][3].asFloat();
};