#include"qogrewidget.h"
#include<OgreWindow.h>
#include<qevent.h>
#include<OgreAbiUtils.h>
#include "Compositor/OgreCompositorManager2.h"
#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"

#include"OgreSkeleton.h"
#include"OgreAnimation.h"
#include"OgreOldBone.h"

#include"Animation/OgreSkeletonManager.h"
#include"Animation/OgreSkeletonInstance.h"

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

#include<assimp/Importer.hpp>
#include<assimp/postprocess.h>

struct Face
{
	unsigned long a, b, c;
};

QImport::QImport(QOgreWidget* widget, Ogre::HlmsManager* _manager, Ogre::RenderSystem* rs, ProgressDialog* _pd, QString _filename) :pd(_pd), filename(_filename), manager(_manager),
renderSystem(rs),ow(widget), mAnimationSpeedModifier(1)
{
};
void QImport::computeNodesDerivedTransform(const aiScene* mScene, const aiNode* pNode, const aiMatrix4x4& accTransform)
{
	if (mNodeDerivedTransformByName.find(pNode->mName.data) == mNodeDerivedTransformByName.end())
	{
		mNodeDerivedTransformByName[pNode->mName.data] = accTransform;
	}
	for (unsigned int childIdx = 0; childIdx < pNode->mNumChildren; ++childIdx)
	{
		const aiNode* pChildNode = pNode->mChildren[childIdx];
		computeNodesDerivedTransform(mScene, pChildNode, accTransform * pChildNode->mTransformation);
	}
}
void QImport::prepare(aiNode* node, unsigned long& count)
{
	if (node->mNumMeshes > 0)
		count += node->mNumMeshes;
	for (unsigned int childIdx = 0; childIdx < node->mNumChildren; childIdx++)
	{
		aiNode* pChildNode = node->mChildren[childIdx];
		prepare(pChildNode, count);
	}
};
Ogre::HlmsDatablock* QImport::createMaterial(const aiMaterial* mat, const Ogre::String& group, const Ogre::String& meshName, const aiScene* scene)
{
	Ogre::MaterialManager* omatMgr = Ogre::MaterialManager::getSingletonPtr();
	enum aiTextureType type = aiTextureType_DIFFUSE;
	aiString path;

	aiString szPath;
	if (AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_NAME, &szPath))
	{
		/*if (verbose)
		{
			LogManager::getSingleton().logMessage("Using aiGetMaterialString : Name " +
				String(szPath.data));
		}*/
	}

	Ogre::String materialName(szPath.data);

	Ogre::HlmsPbs* pbs = (Ogre::HlmsPbs*)manager->getHlms(Ogre::HLMS_PBS);
	Ogre::HlmsMacroblock mb;
	Ogre::HlmsBlendblock bb;

	Ogre::HlmsPbsDatablock* db;
	db = (Ogre::HlmsPbsDatablock*)pbs->getDatablock(materialName);
	if (db == 0)
		db = (Ogre::HlmsPbsDatablock*)pbs->createDatablock(materialName, materialName, mb, bb, Ogre::HlmsParamVec());

	Ogre::String matName = meshName;
	if (szPath.length)
	{
		matName += Ogre::String(szPath.data);
		replace(matName.begin(), matName.end(), ' ', '_');
		//matName = ReplaceSpaces(matName);
	}
	else
	{
		//matName += Ogre::StringUtil::format("dummyMat%d", dummyMatCount++);
	}
	auto status = omatMgr->createOrRetrieve(matName, group);
	Ogre::MaterialPtr omat = Ogre::static_pointer_cast<Ogre::Material>(status.first);

	//if (!status.second)
	//	return omat;

	aiString tmp;
	Ogre::String alphaMode;
	if (AI_SUCCESS == aiGetMaterialString(mat, "$mat.gltf.alphaMode", 0, 0, &tmp))
	{
		alphaMode = Ogre::String(tmp.data);
	}

	// ambient
	aiColor4D clr(1.0f, 1.0f, 1.0f, 1.0);
	// Ambient is usually way too low! FIX ME!
	if (mat->GetTexture(type, 0, &path) != AI_SUCCESS)
		aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &clr);
	omat->setAmbient(clr.r, clr.g, clr.b);

	// diffuse
	clr = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &clr))
	{
		omat->setDiffuse(clr.r, clr.g, clr.b, clr.a);
		db->setDiffuse(Ogre::Vector3(clr.r, clr.g, clr.b));
	}

	if (clr.a < 1.0f || alphaMode == "BLEND")
	{
		//omat->setSceneBlending(SBT_TRANSPARENT_ALPHA);
		//omat->setDepthWriteEnabled(false);
	}

	// specular
	clr = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &clr))
	{
		omat->setSpecular(clr.r, clr.g, clr.b, clr.a);
		db->setSpecular(Ogre::Vector3(clr.r, clr.g, clr.b));
	}

	// emissive
	clr = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &clr))
	{
		omat->setSelfIllumination(clr.r, clr.g, clr.b);
		db->setEmissive(Ogre::Vector3(clr.r, clr.g, clr.b));
	}

	float fShininess;
	if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_SHININESS, &fShininess))
	{
		omat->setShininess(Ogre::Real(fShininess));
	}

	/*Ogre::String basename;
	if (getTextureName(mat, type, scene, meshName, basename))
	{
		db->setTexture(Ogre::PbsTextureTypes::PBSM_DIFFUSE, basename);
		omat->getTechnique(0)->getPass(0)->createTextureUnitState(basename);
	}*/

	return db;
}
bool QImport::createSubMesh(const Ogre::String& name, int index, const aiNode* pNode, const aiMesh* mesh, Ogre::HlmsDatablock* db, Ogre::Mesh* mMesh, Ogre::Aabb& mAAB)
{
	// now begin the object definition
	// We create a submesh per material
	Ogre::SubMesh* submesh = mMesh->createSubMesh();// name + Ogre::StringConverter::toString(index));

	// prime pointers to vertex related data
	aiVector3D* vec = mesh->mVertices;
	aiVector3D* norm = mesh->mNormals;
	aiVector3D* uv = mesh->mTextureCoords[0];
	aiVector3D* tang = mesh->mTangents;
	aiColor4D* col = mesh->mColors[0];

	Ogre::VertexElement2Vec vertexElements;
	vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));

	int vertexSize = 3 * 4;
	if (norm)
	{
		vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_NORMAL));
		vertexSize += (3 * 4);
	}
	if (uv)
	{
		vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES));
		vertexSize += (2 * 4);
	}

	float* vertexs = (float*)OGRE_MALLOC_SIMD(vertexSize * mesh->mNumVertices, Ogre::MEMCATEGORY_GEOMETRY);

	submesh->setMaterialName(*db->getNameStr());
	//submesh->setMaterialName(matptr->getName());

	aiMatrix4x4 aiM = mNodeDerivedTransformByName.find(pNode->mName.data)->second;

	Ogre::Matrix3 normalMatrix;
	Ogre::Matrix4(aiM[0]).extract3x3Matrix(normalMatrix);

	int _index = 0;
	for (size_t i = 0; i < mesh->mNumVertices; ++i)
	{
		aiVector3D v = aiM * vec[i];
		vertexs[_index] = v.x;
		_index++;
		vertexs[_index] = v.y;
		_index++;
		vertexs[_index] = v.z;
		_index++;

		mAAB.merge(Ogre::Vector3(v.x, v.y, v.z));

		if (norm)
		{
			Ogre::Vector3 nv(norm[i].x, norm[i].y, norm[i].z);

			nv = normalMatrix * nv;
			vertexs[_index] = nv.x;
			_index++;
			vertexs[_index] = nv.y;
			_index++;
			vertexs[_index] = nv.z;
			_index++;
		}
		if (uv)
		{
			vertexs[_index] = uv[i].x;
			_index++;
			vertexs[_index] = uv[i].y;
			_index++;
		}
	}

	Face* faces = reinterpret_cast<Face*>(OGRE_MALLOC_SIMD(sizeof(Face) * mesh->mNumFaces, Ogre::MEMCATEGORY_GEOMETRY));
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		faces[i].a = face.mIndices[0];
		faces[i].b = face.mIndices[1];
		faces[i].c = face.mIndices[2];
	}

	Ogre::VertexBufferPacked* vertexBuffer = 0;

	Ogre::VaoManager* vaoManager = renderSystem->getVaoManager();

	vertexBuffer = vaoManager->createVertexBuffer(vertexElements, mesh->mNumVertices, Ogre::BT_IMMUTABLE, vertexs, true);

	// Now the Vao. We'll just use one vertex buffer source (multi-source not working yet)
	Ogre::VertexBufferPackedVec vertexBuffers;
	vertexBuffers.push_back(vertexBuffer);

	Ogre::IndexBufferPacked* indexBuffer = 0;
	indexBuffer = vaoManager->createIndexBuffer(Ogre::IndexBufferPacked::IT_32BIT, 3 * mesh->mNumFaces,
		Ogre::BT_IMMUTABLE, faces, true);

	Ogre::VertexArrayObject* vao =
		vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer, Ogre::OT_TRIANGLE_LIST);

	// Each Vao pushed to the vector refers to an LOD level.
	// Must be in sync with mesh->mLodValues & mesh->mNumLods if you use more than one level
	submesh->mVao[Ogre::VpNormal].push_back(vao);
	// Use the same geometry for shadow casting.
	submesh->mVao[Ogre::VpShadow].push_back(vao);

	return true;
}
Ogre::Aabb QImport::loadDataFromNode(const aiScene* scene, aiNode* node, Ogre::Mesh* mesh)
{
	Ogre::Aabb aabb;
	if (node->mNumMeshes > 0)
	{
		for (unsigned int idx = 0; idx < node->mNumMeshes; ++idx)
		{
			aiMesh* pAIMesh = scene->mMeshes[node->mMeshes[idx]];

			QString message("SubMesh %1 for mesh %2");
			message = message.arg(idx).arg(node->mName.data);
			QMetaObject::invokeMethod(pd, "SetMessasge", Q_ARG(QString, message));

			const aiMaterial* pAIMaterial = scene->mMaterials[pAIMesh->mMaterialIndex];
			Ogre::HlmsDatablock* matptr = createMaterial(pAIMaterial, mesh->getGroup(), mesh->getName(), scene);// , !mQuietMode);
			createSubMesh(node->mName.data, idx, node, pAIMesh, matptr, mesh, aabb);

			QMetaObject::invokeMethod(pd, "IncreaseProgress");
		}
	}
	// Traverse all child nodes of the current node instance
	for (unsigned int childIdx = 0; childIdx < node->mNumChildren; childIdx++)
	{
		aiNode* pChildNode = node->mChildren[childIdx];
		aabb.merge(loadDataFromNode(scene, pChildNode, mesh));
	}
	return aabb;
};
void QImport::flagNodeAsNeeded(const char* name)
{
	boneMapType::iterator iter = boneMap.find(Ogre::String(name));
	if (iter != boneMap.end())
	{
		iter->second = true;
	}
}
void QImport::markAllChildNodesAsNeeded(const aiNode* pNode)
{
	flagNodeAsNeeded(pNode->mName.data);
	// Traverse all child nodes of the current node instance
	for (unsigned int childIdx = 0; childIdx < pNode->mNumChildren; ++childIdx)
	{
		const aiNode* pChildNode = pNode->mChildren[childIdx];
		markAllChildNodesAsNeeded(pChildNode);
	}
}
void QImport::grabNodeNamesFromNode(const aiScene* mScene, const aiNode* pNode)
{
	boneMap.emplace(Ogre::String(pNode->mName.data), false);
	mBoneNodesByName[pNode->mName.data] = pNode;

	QWidget* p = pd->parentWidget();

	QString message("Node ");
	message += pNode->mName.data;
	message += " found.";
	QMetaObject::invokeMethod(p, "writeLog", Q_ARG(int, 0), Q_ARG(QString, message));

	/*if (!mQuietMode)
	{
		Ogre::LogManager::getSingleton().logMessage("Node " + Ogre::String(pNode->mName.data) + " found.");
	}*/

	// Traverse all child nodes of the current node instance
	for (unsigned int childIdx = 0; childIdx < pNode->mNumChildren; ++childIdx)
	{
		const aiNode* pChildNode = pNode->mChildren[childIdx];
		grabNodeNamesFromNode(mScene, pChildNode);
	}
}
void QImport::grabBoneNamesFromNode(const aiScene* mScene, const aiNode* pNode)
{
	static int meshNum = 0;
	meshNum++;
	if (pNode->mNumMeshes > 0)
	{
		for (unsigned int idx = 0; idx < pNode->mNumMeshes; ++idx)
		{
			aiMesh* pAIMesh = mScene->mMeshes[pNode->mMeshes[idx]];

			if (pAIMesh->HasBones())
			{
				for (Ogre::uint32 i = 0; i < pAIMesh->mNumBones; ++i)
				{
					aiBone* pAIBone = pAIMesh->mBones[i];
					if (NULL != pAIBone)
					{
						mBonesByName[pAIBone->mName.data] = pAIBone;

						QWidget* p = pd->parentWidget();

						QString message("%1 ) REAL BONE with name : %2");
						message = message.arg(i).arg(pAIBone->mName.data);
						QMetaObject::invokeMethod(p, "writeLog", Q_ARG(int, 0), Q_ARG(QString, message));
						/*if (!mQuietMode)
						{
							Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(i) + ") REAL BONE with name : " + Ogre::String(pAIBone->mName.data));
						}*/

						// flag this node and all parents of this node as needed, until we reach the node holding the mesh, or the parent.
						aiNode* node = mScene->mRootNode->FindNode(pAIBone->mName.data);
						while (node)
						{
							if (node->mName.data == pNode->mName.data)
							{
								flagNodeAsNeeded(node->mName.data);
								break;
							}
							if (node->mName.data == pNode->mParent->mName.data)
							{
								flagNodeAsNeeded(node->mName.data);
								break;
							}

							// Not a root node, flag this as needed and continue to the parent
							flagNodeAsNeeded(node->mName.data);
							node = node->mParent;
						}

						// Flag all children of this node as needed
						node = mScene->mRootNode->FindNode(pAIBone->mName.data);
						markAllChildNodesAsNeeded(node);

					} // if we have a valid bone
				} // loop over bones
			} // if this mesh has bones
		} // loop over meshes
	} // if this node has meshes

	// Traverse all child nodes of the current node instance
	for (unsigned int childIdx = 0; childIdx < pNode->mNumChildren; childIdx++)
	{
		const aiNode* pChildNode = pNode->mChildren[childIdx];
		grabBoneNamesFromNode(mScene, pChildNode);
	}
}
bool QImport::isNodeNeeded(const char* name)
{
	boneMapType::iterator iter = boneMap.find(Ogre::String(name));
	if (iter != boneMap.end())
	{
		return iter->second;
	}
	return false;
}
void QImport::createBonesFromNode(const aiScene* mScene, const aiNode* pNode)
{
	if (isNodeNeeded(pNode->mName.data))
	{

		Ogre::v1::OldBone* bone = mSkeleton->createBone(Ogre::String(pNode->mName.data), msBoneCount);

		aiQuaternion rot;
		aiVector3D pos;
		aiVector3D scale;

		// above should be the same as
		aiMatrix4x4 aiM = pNode->mTransformation;

		aiM.Decompose(scale, rot, pos);


		/*
		// debug render
		Ogre::SceneNode* sceneNode = NULL;
		if(parentNode)
		{
			Ogre::SceneNode* parent = static_cast<Ogre::SceneNode*>(
				GOOF::NodeUtils::GetNodeMatch(getSceneManager()->getRootSceneNode(), parentNode->mName.data, false));
			assert(parent);
			sceneNode = parent->createChildSceneNode(pNode->mName.data);
		}
		else
		{
			sceneNode = getSceneManager()->getRootSceneNode()->createChildSceneNode(pNode->mName.data);
		}

		sceneNode->setScale(scale.x, scale.y, scale.z);
		sceneNode->setPosition(pos.x, pos.y, pos.z);
		sceneNode->setOrientation(rot.w, rot.x, rot.y, rot.z);

		sceneNode = sceneNode->createChildSceneNode();
		sceneNode->setScale(0.01, 0.01, 0.01);
		sceneNode->attachObject(getSceneManager()->createEntity("Box1m.mesh"));
		*/

		if (!aiM.IsIdentity())
		{
			bone->setPosition(pos.x, pos.y, pos.z);
			bone->setOrientation(rot.w, rot.x, rot.y, rot.z);
		}

		QWidget* p = pd->parentWidget();
		QString message("%1 ) Creating bone '%2'");
		message = message.arg(msBoneCount).arg(pNode->mName.data);
		QMetaObject::invokeMethod(p, "writeLog", Q_ARG(int, 0), Q_ARG(QString, message));
		/*if (!mQuietMode)
		{
			Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(msBoneCount) + ") Creating bone '" + Ogre::String(pNode->mName.data) + "'");
		}*/

		msBoneCount++;
	}
	// Traverse all child nodes of the current node instance
	for (unsigned int childIdx = 0; childIdx < pNode->mNumChildren; ++childIdx)
	{
		const aiNode* pChildNode = pNode->mChildren[childIdx];
		createBonesFromNode(mScene, pChildNode);
	}
}
void QImport::createBoneHiearchy(const aiScene* mScene, const aiNode* pNode)
{
	if (isNodeNeeded(pNode->mName.data))
	{
		Ogre::v1::OldBone* parent = 0;
		Ogre::v1::OldBone* child = 0;
		if (pNode->mParent)
		{
			if (mSkeleton->hasBone(pNode->mParent->mName.data))
			{
				parent = mSkeleton->getBone(pNode->mParent->mName.data);
			}
		}
		if (mSkeleton->hasBone(pNode->mName.data))
		{
			child = mSkeleton->getBone(pNode->mName.data);
		}
		if (parent && child)
		{
			parent->addChild(child);
		}
	}
	// Traverse all child nodes of the current node instance
	for (unsigned int childIdx = 0; childIdx < pNode->mNumChildren; childIdx++)
	{
		const aiNode* pChildNode = pNode->mChildren[childIdx];
		createBoneHiearchy(mScene, pChildNode);
	}
}

/** translation, rotation, scale */
typedef std::tuple< aiVectorKey*, aiQuatKey*, aiVectorKey* > KeyframeData;
typedef std::map< Ogre::Real, KeyframeData > KeyframesMap;

template <int v>
struct Int2Type
{
	enum { value = v };
};

// T should be a Loki::Int2Type<>
template< typename T > void GetInterpolationIterators(KeyframesMap& keyframes,
	KeyframesMap::iterator it,
	KeyframesMap::reverse_iterator& front,
	KeyframesMap::iterator& back)
{
	front = KeyframesMap::reverse_iterator(it);

	front++;
	for (front; front != keyframes.rend(); front++)
	{
		if (std::get< T::value >(front->second) != NULL)
		{
			break;
		}
	}

	back = it;
	back++;
	for (back; back != keyframes.end(); back++)
	{
		if (std::get< T::value >(back->second) != NULL)
		{
			break;
		}
	}
}

aiVector3D getTranslate(aiNodeAnim* node_anim, KeyframesMap& keyframes, KeyframesMap::iterator it, Ogre::Real ticksPerSecond)
{
	aiVectorKey* translateKey = std::get<0>(it->second);
	aiVector3D vect;
	if (translateKey)
	{
		vect = translateKey->mValue;
	}
	else
	{
		KeyframesMap::reverse_iterator front;
		KeyframesMap::iterator back;


		GetInterpolationIterators< Int2Type<0> >(keyframes, it, front, back);

		KeyframesMap::reverse_iterator rend = keyframes.rend();
		KeyframesMap::iterator end = keyframes.end();
		aiVectorKey* frontKey = NULL;
		aiVectorKey* backKey = NULL;

		if (front != rend)
			frontKey = std::get<0>(front->second);

		if (back != end)
			backKey = std::get<0>(back->second);

		// got 2 keys can interpolate
		if (frontKey && backKey)
		{
			float prop = (float)(((double)it->first - frontKey->mTime) / (backKey->mTime - frontKey->mTime));
			prop /= ticksPerSecond;
			vect = ((backKey->mValue - frontKey->mValue) * prop) + frontKey->mValue;
		}

		else if (frontKey)
		{
			vect = frontKey->mValue;
		}
		else if (backKey)
		{
			vect = backKey->mValue;
		}
	}

	return vect;
}

aiQuaternion getRotate(aiNodeAnim* node_anim, KeyframesMap& keyframes, KeyframesMap::iterator it, Ogre::Real ticksPerSecond)
{
	aiQuatKey* rotationKey = std::get<1>(it->second);
	aiQuaternion rot;
	if (rotationKey)
	{
		rot = rotationKey->mValue;
	}
	else
	{
		KeyframesMap::reverse_iterator front;
		KeyframesMap::iterator back;

		GetInterpolationIterators< Int2Type<1> >(keyframes, it, front, back);

		KeyframesMap::reverse_iterator rend = keyframes.rend();
		KeyframesMap::iterator end = keyframes.end();
		aiQuatKey* frontKey = NULL;
		aiQuatKey* backKey = NULL;

		if (front != rend)
			frontKey = std::get<1>(front->second);

		if (back != end)
			backKey = std::get<1>(back->second);

		// got 2 keys can interpolate
		if (frontKey && backKey)
		{
			float prop = (float)(((double)it->first - frontKey->mTime) / (backKey->mTime - frontKey->mTime));
			prop /= ticksPerSecond;
			aiQuaternion::Interpolate(rot, frontKey->mValue, backKey->mValue, prop);
		}

		else if (frontKey)
		{
			rot = frontKey->mValue;
		}
		else if (backKey)
		{
			rot = backKey->mValue;
		}
	}

	return rot;
}

aiVector3D getScale(aiNodeAnim* node_anim, KeyframesMap& keyframes, KeyframesMap::iterator it, Ogre::Real ticksPerSecond)
{
	aiVectorKey* scaleKey = std::get<2>(it->second);
	aiVector3D vect(1,1,1);
	if (scaleKey)
	{
		vect = scaleKey->mValue;
	}
	else
	{
		KeyframesMap::reverse_iterator front;
		KeyframesMap::iterator back;


		GetInterpolationIterators< Int2Type<2> >(keyframes, it, front, back);

		KeyframesMap::reverse_iterator rend = keyframes.rend();
		KeyframesMap::iterator end = keyframes.end();
		aiVectorKey* frontKey = NULL;
		aiVectorKey* backKey = NULL;

		if (front != rend)
			frontKey = std::get<0>(front->second);

		if (back != end)
			backKey = std::get<0>(back->second);

		// got 2 keys can interpolate
		if (frontKey && backKey)
		{
			float prop = (float)(((double)it->first - frontKey->mTime) / (backKey->mTime - frontKey->mTime));
			prop /= ticksPerSecond;
			vect = ((backKey->mValue - frontKey->mValue) * prop) + frontKey->mValue;
		}

		else if (frontKey)
		{
			vect = frontKey->mValue;
		}
		else if (backKey)
		{
			vect = backKey->mValue;
		}
	}

	return vect;
}

void QImport::parseAnimation(const aiScene* mScene, int index, aiAnimation* anim)
{
	// DefBonePose a matrix that represents the local bone transform (can build from Ogre bone components)
	// PoseToKey a matrix representing the keyframe translation
	// What assimp stores aiNodeAnim IS the decomposed form of the transform (DefBonePose * PoseToKey)
	// To get PoseToKey which is what Ogre needs we'ed have to build the transform from components in
	// aiNodeAnim and then DefBonePose.Inverse() * aiNodeAnim(generated transform) will be the right transform

	Ogre::String animName;
	if (mCustomAnimationName != "")
	{
		animName = mCustomAnimationName;
		if (index >= 1)
		{
			animName += Ogre::StringConverter::toString(index);
		}
	}
	else
	{
		animName = Ogre::String(anim->mName.data);
	}
	if (animName.length() < 1)
	{
		animName = "Animation" + Ogre::StringConverter::toString(index);
	}

	QWidget* p = pd->parentWidget();
	QString message("Animation name = '%1' duration = %2 tick/sec = %3 channels = %4");
	message = message.arg(animName.c_str()).arg(anim->mDuration).arg(anim->mTicksPerSecond).arg(anim->mNumChannels);
	QMetaObject::invokeMethod(p, "writeLog", Q_ARG(int, 0), Q_ARG(QString, message));
	/*if (!mQuietMode)
	{
		Ogre::LogManager::getSingleton().logMessage("Animation name = '" + animName + "'");
		Ogre::LogManager::getSingleton().logMessage("duration = " + Ogre::StringConverter::toString(Ogre::Real(anim->mDuration)));
		Ogre::LogManager::getSingleton().logMessage("tick/sec = " + Ogre::StringConverter::toString(Ogre::Real(anim->mTicksPerSecond)));
		Ogre::LogManager::getSingleton().logMessage("channels = " + Ogre::StringConverter::toString(anim->mNumChannels));
	}*/

	Ogre::v1::Animation* animation;
	mTicksPerSecond = (Ogre::Real)((0 == anim->mTicksPerSecond) ? 24 : anim->mTicksPerSecond);
	mTicksPerSecond *= mAnimationSpeedModifier;

	Ogre::Real cutTime = 0.0;
#if 0
	if (mLoaderParams & LP_CUT_ANIMATION_WHERE_NO_FURTHER_CHANGE)
	{
		for (int i = 1; i < (int)anim->mNumChannels; i++)
		{
			aiNodeAnim* node_anim = anim->mChannels[i];

			// times of the equality check
			Ogre::Real timePos = 0.0;
			Ogre::Real timeRot = 0.0;

			for (unsigned int i = 1; i < node_anim->mNumPositionKeys; i++)
			{
				if (node_anim->mPositionKeys[i] != node_anim->mPositionKeys[i - 1])
				{
					timePos = (Ogre::Real)node_anim->mPositionKeys[i].mTime;
					timePos /= mTicksPerSecond;
				}
			}

			for (unsigned int i = 1; i < node_anim->mNumRotationKeys; i++)
			{
				if (node_anim->mRotationKeys[i] != node_anim->mRotationKeys[i - 1])
				{
					timeRot = (Ogre::Real)node_anim->mRotationKeys[i].mTime;
					timeRot /= mTicksPerSecond;
				}
			}

			if (timePos > cutTime) { cutTime = timePos; }
			if (timeRot > cutTime) { cutTime = timeRot; }
		}

		animation = mSkeleton->createAnimation(Ogre::String(animName), cutTime);
	}
	else
	{
		cutTime = Ogre::Math::POS_INFINITY;
		animation = mSkeleton->createAnimation(Ogre::String(animName), Ogre::Real(anim->mDuration / mTicksPerSecond));
	}
#else
	cutTime = Ogre::Math::POS_INFINITY;
	animation = mSkeleton->createAnimation(Ogre::String(animName), Ogre::Real(anim->mDuration / mTicksPerSecond));
#endif

	animation->setInterpolationMode(Ogre::v1::Animation::IM_LINEAR); //FIXME: Is this always true?

	message = QString("Cut Time '%1'");
	message = message.arg(cutTime);
	QMetaObject::invokeMethod(p, "writeLog", Q_ARG(int, 0), Q_ARG(QString, message));
	/*if (!mQuietMode)
	{
		Ogre::LogManager::getSingleton().logMessage("Cut Time " + Ogre::StringConverter::toString(cutTime));
	}*/

	for (int i = 0; i < (int)anim->mNumChannels; i++)
	//for (int i = 0; i < 28; i++)
	{
		/*if (i == 27)
		{
			int k = 0;
		}*/
		Ogre::v1::TransformKeyFrame* keyframe;

		aiNodeAnim* node_anim = anim->mChannels[i];

		message = QString("Channel %1 affecting node: %2");
		message = message.arg(i).arg(node_anim->mNodeName.data);
		QMetaObject::invokeMethod(p, "writeLog", Q_ARG(int, 0), Q_ARG(QString, message));
		/*if (!mQuietMode)
		{
			Ogre::LogManager::getSingleton().logMessage("Channel " + Ogre::StringConverter::toString(i));
			Ogre::LogManager::getSingleton().logMessage("affecting node: " + Ogre::String(node_anim->mNodeName.data));
		}*/
		Ogre::String boneName = Ogre::String(node_anim->mNodeName.data);

		if (mSkeleton->hasBone(boneName))
		{
			Ogre::v1::OldBone* bone = mSkeleton->getBone(boneName);

			//Affine3 defBonePoseInv;
			//defBonePoseInv.makeInverseTransform(bone->getPosition(), bone->getScale(), bone->getOrientation());
			Ogre::Matrix4 defBonePoseInv;
			defBonePoseInv.makeInverseTransform(bone->getPosition(), bone->getScale(), bone->getOrientation());

			Ogre::v1::OldNodeAnimationTrack* track = animation->createOldNodeTrack(i, bone);
			// Ogre needs translate rotate and scale for each keyframe in the track
			KeyframesMap keyframes;

			for (unsigned int i = 0; i < node_anim->mNumPositionKeys; i++)
			{
				keyframes[(Ogre::Real)node_anim->mPositionKeys[i].mTime / mTicksPerSecond] = KeyframeData(&(node_anim->mPositionKeys[i]), NULL, NULL);
			}

			for (unsigned int i = 0; i < node_anim->mNumRotationKeys; i++)
			{
				KeyframesMap::iterator it = keyframes.find((Ogre::Real)node_anim->mRotationKeys[i].mTime / mTicksPerSecond);
				if (it != keyframes.end())
				{
					std::get<1>(it->second) = &(node_anim->mRotationKeys[i]);
				}
				else
				{
					keyframes[(Ogre::Real)node_anim->mRotationKeys[i].mTime / mTicksPerSecond] = KeyframeData(NULL, &(node_anim->mRotationKeys[i]), NULL);
				}
			}

			for (unsigned int i = 0; i < node_anim->mNumScalingKeys; i++)
			{
				KeyframesMap::iterator it = keyframes.find((Ogre::Real)node_anim->mScalingKeys[i].mTime / mTicksPerSecond);
				if (it != keyframes.end())
				{
					std::get<2>(it->second) = &(node_anim->mScalingKeys[i]);
				}
				else
				{
					keyframes[(Ogre::Real)node_anim->mRotationKeys[i].mTime / mTicksPerSecond] = KeyframeData(NULL, NULL, &(node_anim->mScalingKeys[i]));
				}
			}

			KeyframesMap::iterator it = keyframes.begin();
			KeyframesMap::iterator it_end = keyframes.end();
			for (it; it != it_end; ++it)
			{
				if (it->first < cutTime)	// or should it be <=
				{
					aiVector3D aiTrans = getTranslate(node_anim, keyframes, it, mTicksPerSecond);

					Ogre::Vector3 trans(aiTrans.x, aiTrans.y, aiTrans.z);

					aiQuaternion aiRot = getRotate(node_anim, keyframes, it, mTicksPerSecond);
					Ogre::Quaternion rot(aiRot.w, aiRot.x, aiRot.y, aiRot.z);

					aiVector3D aiScale = getScale(node_anim, keyframes, it, mTicksPerSecond);
					Ogre::Vector3 scale(aiScale.x, aiScale.y, aiScale.z);

					Ogre::Vector3 transCopy = trans;

					//Affine3 fullTransform;
					Ogre::Matrix4 fullTransform;
					fullTransform.makeTransform(trans, scale, rot);

					//Affine3 poseTokey = defBonePoseInv * fullTransform;
					Ogre::Matrix4 poseTokey = defBonePoseInv * fullTransform;
					poseTokey.decomposition(trans, scale, rot);

					keyframe = track->createNodeKeyFrame(Ogre::Real(it->first));

					// weirdness with the root bone, But this seems to work
					//if (mSkeleton->getRootBones()[0]->getName() == boneName)
					if (mSkeleton->getRootBone()->getName() == boneName)
					{
						trans = transCopy - bone->getPosition();
					}

					keyframe->setTranslate(trans);
					keyframe->setRotation(rot);
					keyframe->setScale(scale);
				}
			}
		} // if bone exists
	} // loop through channels

	mSkeleton->optimiseAllAnimations();

}
void QImport::run()
{
	QMetaObject::invokeMethod(pd, "SetMessasge", Q_ARG(QString, "Import mesh"));

	Ogre::String basename, ext, path;
	Ogre::StringUtil::splitFullFilename(filename.toStdString(), basename, ext, path);

	//unsigned int Flag = aiProcess_FindInvalidData | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_GenBoundingBoxes | aiProcess_GlobalScale | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded;

	unsigned int Flag = aiProcessPreset_TargetRealtime_Quality | aiProcess_FindInvalidData | aiProcess_GlobalScale | aiProcess_ConvertToLeftHanded;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename.toStdString(), Flag);
	if (scene == 0)
	{
		QMetaObject::invokeMethod(pd, "reject");
		return;
	}

	unsigned long nodes;
	prepare(scene->mRootNode, nodes);

	QMetaObject::invokeMethod(pd, "SetProgressMax", Q_ARG(unsigned long, nodes));

	QMetaObject::invokeMethod(pd, "SetMessasge", Q_ARG(QString, "Get Nodes transform"));
	
	grabNodeNamesFromNode(scene, scene->mRootNode);
	grabBoneNamesFromNode(scene, scene->mRootNode);

	computeNodesDerivedTransform(scene, scene->mRootNode, scene->mRootNode->mTransformation);

	mesh = Ogre::MeshManager::getSingleton().createManual(basename, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	if (mBonesByName.size())
	{

		//mSkeleton = Ogre::SkeletonManager::getSingleton().create(basename + ".skeleton", Ogre::RGN_DEFAULT, true);
		mSkeleton = Ogre::v1::OldSkeletonManager::getSingleton().create(basename + ".skeleton", "General", true);
		//mSkeleton = Ogre::SkeletonManager::getSingleton().getSkeletonDef(v1Skeleton.getPointer());

		msBoneCount = 0;
		createBonesFromNode(scene, scene->mRootNode);
		msBoneCount = 0;
		createBoneHiearchy(scene, scene->mRootNode);

		if (scene->HasAnimations())
		{
			for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
			{
				parseAnimation(scene, i, scene->mAnimations[i]);
			}
		}
	}

	Ogre::Aabb aabb = loadDataFromNode(scene, scene->mRootNode, mesh.get());

	Ogre::v1::SkeletonPtr skeletonPtr;
	if (mSkeleton)
	{
		/*if (!mQuietMode)
		{
			Ogre::LogManager::getSingleton().logMessage("Root bone: " + mSkeleton->getRootBones()[0]->getName());
		}*/

		unsigned short numBones = mSkeleton->getNumBones();
		unsigned short i;

		for (i = 0; i < numBones; ++i)
		{
			Ogre::v1::OldBone* pBone = mSkeleton->getBone(i);
			assert(pBone);
		}
		skeletonPtr = mSkeleton;
		//mesh->setSkeletonName(mSkeleton->getName());
	}

#if 0
	for (auto sm : mesh->getSubMeshes())
	{
		/*if (!sm->useSharedVertices)
		{

			Ogre::VertexDeclaration* newDcl =
				sm->vertexData->vertexDeclaration->getAutoOrganisedDeclaration(mesh->hasSkeleton(), mesh->hasVertexAnimation(), false);

			if (*newDcl != *(sm->vertexData->vertexDeclaration))
			{
				sm->vertexData->reorganiseBuffers(newDcl);
			}
		}*/
	}
#endif

	// We must indicate the bounding box
	mesh->_setBounds(aabb);
	mesh->_setBoundingSphereRadius((aabb.getMaximum() - aabb.getMinimum()).length() / 2);
	mesh->load();

	if (skeletonPtr)
	{
		Ogre::v1::SkeletonSerializer skelSer;
		skelSer.exportSkeleton(skeletonPtr.get(), "models/" + skeletonPtr->getName());

		mesh->setSkeletonName(mSkeleton->getName());

		for (auto sm : mesh->getSubMeshes())
		{
			sm->_buildBoneIndexMap();
		}
	}

	QMetaObject::invokeMethod(pd, "SetMessasge", Q_ARG(QString, "Saving mesh"));
	
	Ogre::MeshSerializer mesh_serializer(0);
	mesh_serializer.exportMesh(mesh.get(), "models/" + basename + ".mesh");

	QMetaObject::invokeMethod(pd, "SetMessasge", Q_ARG(QString, "Saving mesh material"));
	for (int index = 0; index < mesh->getNumSubMeshes(); index++)
	{
		std::string matName = mesh->getSubMesh(index)->getMaterialName();
		Ogre::HlmsDatablock* material = ow->GetMaterial(matName.c_str());
		ow->GetHlmsManager()->saveMaterial(material, "models/" + matName + ".material.json", 0, "");
	}

	ow->CreateScene(mesh);

	QMetaObject::invokeMethod(pd, "accept");
};

struct Vertex
{
	float position[3];
	float normal[3];
	float tex[2];
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
	root = OGRE_NEW Ogre::Root(&abiCookie, "plugins" OGRE_BUILD_SUFFIX ".cfg", "ogre.cfg", "OgreMeshEditor.log");

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
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./models", "FileSystem", "General");
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
	const Ogre::String workspaceName("PbsMaterialsWorkspace");
	const Ogre::ColourValue backgroundColour(0.2f, 0.4f, 0.6f);
	if (!compositorManager->hasWorkspaceDefinition(workspaceName))
		compositorManager->createBasicWorkspaceDef(workspaceName, backgroundColour, Ogre::IdString());
	compositorManager->addWorkspace(sm, window->getTexture(), camera, workspaceName, true);

	is_draw = true;

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
	if (root->isInitialised() && is_draw)
	{
		if (meshNode)
		{
			for (auto a : meshNode->getAttachedObject(0)->getSkeletonInstance()->getActiveAnimations())
			{
				a->addTime(1.f/30.f);
			}
			//meshNode->getAttachedObject(0)->getSkeletonInstance()->getAnimation("Idle")->addTime(1);
		}
		root->renderOneFrame(1.f/30.f);
	}
};
bool QOgreWidget::eventFilter(QObject* target, QEvent* event)
{
	if (target == this)
	{
		if (event->type() == QEvent::Resize)
		{
			QResizeEvent* re = (QResizeEvent*)event;
			QSize size = re->size();
			camera->setAspectRatio((float)size.width() / (float)size.height());
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
	if (camera)
		camera->setAspectRatio((float)e->size().width() / (float)e->size().height());
	emit resizeWindow(e->size().width(), e->size().height());
	window->windowMovedOrResized();
};
void QOgreWidget::computeNodesDerivedTransform(const aiScene* mScene, const aiNode* pNode, const aiMatrix4x4& accTransform)
{
	if (mNodeDerivedTransformByName.find(pNode->mName.data) == mNodeDerivedTransformByName.end())
	{
		mNodeDerivedTransformByName[pNode->mName.data] = accTransform;
	}
	for (unsigned int childIdx = 0; childIdx < pNode->mNumChildren; ++childIdx)
	{
		const aiNode* pChildNode = pNode->mChildren[childIdx];
		computeNodesDerivedTransform(mScene, pChildNode, accTransform * pChildNode->mTransformation);
	}
}
bool getTextureName(const aiMaterial* mat, aiTextureType type, const aiScene* scene, const Ogre::String& meshName,
	Ogre::String& basename)
{
	aiString path;
	Ogre::String outPath;
	if (mat->GetTexture(type, 0, &path) == AI_SUCCESS)
	{
		const aiTexture* tex = scene->GetEmbeddedTexture(path.C_Str());
		if (tex)
		{
			basename.resize(256);
			sprintf(basename.data(), "%s%s%s.%.8s", meshName.c_str(), tex->mFilename.C_Str(), path.C_Str() + 1,
				tex->achFormatHint);
			/*basename = Ogre::StringUtil::format("%s%s%s.%.8s", meshName.c_str(), tex->mFilename.C_Str(), path.C_Str() + 1,
				tex->achFormatHint);*/
			return true;
		}

		Ogre::StringUtil::splitFilename(Ogre::String(path.data), basename, outPath);
		return true;
	}
	return false;
};
Ogre::HlmsDatablock* QOgreWidget::createMaterial(const aiMaterial* mat, const Ogre::String& group, const Ogre::String& meshName, const aiScene* scene)
{
	Ogre::MaterialManager* omatMgr = Ogre::MaterialManager::getSingletonPtr();
	enum aiTextureType type = aiTextureType_DIFFUSE;
	aiString path;

	aiString szPath;
	if (AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_NAME, &szPath))
	{
		/*if (verbose)
		{
			LogManager::getSingleton().logMessage("Using aiGetMaterialString : Name " +
				String(szPath.data));
		}*/
	}

	Ogre::HlmsManager* hlmsManager = root->getHlmsManager();

	Ogre::String materialName(szPath.data);

	Ogre::HlmsPbs* pbs = (Ogre::HlmsPbs*)hlmsManager->getHlms(Ogre::HLMS_PBS);
	Ogre::HlmsMacroblock mb;
	Ogre::HlmsBlendblock bb;

	Ogre::HlmsPbsDatablock *db = (Ogre::HlmsPbsDatablock*)pbs->createDatablock(materialName, materialName, mb, bb, Ogre::HlmsParamVec());

	Ogre::String matName = meshName;
	if (szPath.length)
	{
		matName += Ogre::String(szPath.data);
		replace(matName.begin(), matName.end(), ' ', '_');
		//matName = ReplaceSpaces(matName);
	}
	else
	{
		//matName += Ogre::StringUtil::format("dummyMat%d", dummyMatCount++);
	}
	auto status = omatMgr->createOrRetrieve(matName, group);
	Ogre::MaterialPtr omat = Ogre::static_pointer_cast<Ogre::Material>(status.first);
	
	//if (!status.second)
	//	return omat;

	aiString tmp;
	Ogre::String alphaMode;
	if (AI_SUCCESS == aiGetMaterialString(mat, "$mat.gltf.alphaMode", 0, 0, &tmp))
	{
		alphaMode = Ogre::String(tmp.data);
	}

	// ambient
	aiColor4D clr(1.0f, 1.0f, 1.0f, 1.0);
	// Ambient is usually way too low! FIX ME!
	if (mat->GetTexture(type, 0, &path) != AI_SUCCESS)
		aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &clr);
	omat->setAmbient(clr.r, clr.g, clr.b);

	// diffuse
	clr = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &clr))
	{
		omat->setDiffuse(clr.r, clr.g, clr.b, clr.a);
		db->setDiffuse(Ogre::Vector3(clr.r, clr.g, clr.b));
	}

	if (clr.a < 1.0f || alphaMode == "BLEND")
	{
		//omat->setSceneBlending(SBT_TRANSPARENT_ALPHA);
		//omat->setDepthWriteEnabled(false);
	}

	// specular
	clr = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &clr))
	{
		omat->setSpecular(clr.r, clr.g, clr.b, clr.a);
		db->setSpecular(Ogre::Vector3(clr.r, clr.g, clr.b));
	}

	// emissive
	clr = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &clr))
	{
		omat->setSelfIllumination(clr.r, clr.g, clr.b);
		db->setEmissive(Ogre::Vector3(clr.r, clr.g, clr.b));
	}
	
	float fShininess;
	if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_SHININESS, &fShininess))
	{
		omat->setShininess(Ogre::Real(fShininess));
	}

	Ogre::String basename;
	if (getTextureName(mat, type, scene, meshName, basename))
	{
		/*if (verbose)
		{
			LogManager::getSingleton().logMessage("Found texture " + basename + " for channel " +
				StringConverter::toString(uvindex));
		}*/
		db->setTexture(Ogre::PbsTextureTypes::PBSM_DIFFUSE, basename);
		omat->getTechnique(0)->getPass(0)->createTextureUnitState(basename);
	}

	return db;
}
bool QOgreWidget::createSubMesh(const Ogre::String& name, int index, const aiNode* pNode, const aiMesh* mesh, Ogre::HlmsDatablock *db, Ogre::Mesh* mMesh, Ogre::Aabb& mAAB)
{
	// now begin the object definition
	// We create a submesh per material
	Ogre::SubMesh* submesh = mMesh->createSubMesh();// name + Ogre::StringConverter::toString(index));

	// prime pointers to vertex related data
	aiVector3D* vec = mesh->mVertices;
	aiVector3D* norm = mesh->mNormals;
	aiVector3D* uv = mesh->mTextureCoords[0];
	aiVector3D* tang = mesh->mTangents;
	aiColor4D* col = mesh->mColors[0];

	Ogre::VertexElement2Vec vertexElements;
	vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));

	int vertexSize = 3*4;
	if (norm)
	{
		vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_NORMAL));
		vertexSize += (3*4);
	}
	if (uv)
	{
		vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES));
		vertexSize += (2 * 4);
	}

	float* vertexs = (float*)OGRE_MALLOC_SIMD(vertexSize * mesh->mNumVertices, Ogre::MEMCATEGORY_GEOMETRY);

	submesh->setMaterialName(*db->getNameStr());
	//submesh->setMaterialName(matptr->getName());

	aiMatrix4x4 aiM = mNodeDerivedTransformByName.find(pNode->mName.data)->second;
	
	Ogre::Matrix3 normalMatrix;
	Ogre::Matrix4(aiM[0]).extract3x3Matrix(normalMatrix);

	int _index = 0;
	for (size_t i = 0; i < mesh->mNumVertices; ++i)
	{
		aiVector3D v = aiM * vec[i];
		vertexs[_index] = v.x;
		_index++;
		vertexs[_index] = v.y;
		_index++;
		vertexs[_index] = v.z;
		_index++;
		
		mAAB.merge(Ogre::Vector3(v.x, v.y, v.z));
		
		if (norm)
		{
			Ogre::Vector3 nv(norm[i].x, norm[i].y, norm[i].z);

			nv = normalMatrix * nv;
			vertexs[_index] = nv.x;
			_index++;
			vertexs[_index] = nv.y;
			_index++;
			vertexs[_index] = nv.z;
			_index++;
		}
		if (uv)
		{
			vertexs[_index] = uv[i].x;
			_index++;
			vertexs[_index] = uv[i].y;
			_index++;
		}
	}

	Face* faces = reinterpret_cast<Face*>(OGRE_MALLOC_SIMD(sizeof(Face) * mesh->mNumFaces, Ogre::MEMCATEGORY_GEOMETRY));
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		faces[i].a = face.mIndices[0];
		faces[i].b = face.mIndices[1];
		faces[i].c = face.mIndices[2];
	}

	Ogre::VertexBufferPacked* vertexBuffer = 0;

	Ogre::RenderSystem* renderSystem = root->getRenderSystem();
	Ogre::VaoManager* vaoManager = renderSystem->getVaoManager();

	vertexBuffer = vaoManager->createVertexBuffer(vertexElements, mesh->mNumVertices, Ogre::BT_IMMUTABLE, vertexs, true);

	// Now the Vao. We'll just use one vertex buffer source (multi-source not working yet)
	Ogre::VertexBufferPackedVec vertexBuffers;
	vertexBuffers.push_back(vertexBuffer);

	Ogre::IndexBufferPacked* indexBuffer = 0;
	indexBuffer = vaoManager->createIndexBuffer(Ogre::IndexBufferPacked::IT_32BIT, 3 * mesh->mNumFaces,
		Ogre::BT_IMMUTABLE, faces, true);

	Ogre::VertexArrayObject* vao =
		vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer, Ogre::OT_TRIANGLE_LIST);

	// Each Vao pushed to the vector refers to an LOD level.
	// Must be in sync with mesh->mLodValues & mesh->mNumLods if you use more than one level
	submesh->mVao[Ogre::VpNormal].push_back(vao);
	// Use the same geometry for shadow casting.
	submesh->mVao[Ogre::VpShadow].push_back(vao);

	return true;
}
Ogre::Aabb QOgreWidget::loadDataFromNode(const aiScene* scene, aiNode* node, Ogre::Mesh* mesh)
{
	Ogre::Aabb aabb;
	if (node->mNumMeshes > 0)
	{
		for (unsigned int idx = 0; idx < node->mNumMeshes; ++idx)
		{
			aiMesh* pAIMesh = scene->mMeshes[node->mMeshes[idx]];
			/*if (!mQuietMode)
			{
				LogManager::getSingleton().logMessage("SubMesh " + StringConverter::toString(idx) +
					" for mesh '" + String(pNode->mName.data) + "'");
			}*/
			const aiMaterial* pAIMaterial = scene->mMaterials[pAIMesh->mMaterialIndex];
			Ogre::HlmsDatablock *matptr = createMaterial(pAIMaterial, mesh->getGroup(), mesh->getName(), scene);// , !mQuietMode);
			createSubMesh(node->mName.data, idx, node, pAIMesh, matptr, mesh, aabb);
		}
	}
	// Traverse all child nodes of the current node instance
	for (unsigned int childIdx = 0; childIdx < node->mNumChildren; childIdx++)
	{
		aiNode* pChildNode = node->mChildren[childIdx];
		aabb.merge(loadDataFromNode(scene, pChildNode, mesh));
	}
	return aabb;
};
void QOgreWidget::processMesh(aiMesh* mesh, const aiScene* scene, Ogre::MeshPtr o_mesh)
{
	Ogre::SubMesh *sub_mesh = o_mesh->createSubMesh();

	Ogre::VertexElement2Vec vertexElements;
	vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));
	vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_NORMAL));
	vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES));

	aiMatrix4x4 mat;
	aiVector3D scale(1000, 1000, 1000);
	aiMatrix4x4::Scaling(scale, mat);

	Vertex* vertexs = reinterpret_cast<Vertex*>(OGRE_MALLOC_SIMD(sizeof(Vertex) * mesh->mNumVertices, Ogre::MEMCATEGORY_GEOMETRY));
	for (unsigned int index = 0; index < mesh->mNumVertices; index++)
	{
		aiVector3D v = mesh->mVertices[index];
		vertexs[index].position[0] = v.x;
		vertexs[index].position[1] = v.y;
		vertexs[index].position[2] = v.z;

		vertexs[index].normal[0] = mesh->mNormals[index].x;
		vertexs[index].normal[1] = mesh->mNormals[index].y;
		vertexs[index].normal[2] = mesh->mNormals[index].z;
	}
	Face* faces = reinterpret_cast<Face*>(OGRE_MALLOC_SIMD(sizeof(Face)*mesh->mNumFaces, Ogre::MEMCATEGORY_GEOMETRY));
	for (unsigned int index = 0; index < mesh->mNumFaces; index++)
	{
		aiFace face = mesh->mFaces[index];

		faces[index].a = face.mIndices[0];
		faces[index].b = face.mIndices[1];
		faces[index].c = face.mIndices[2];
	}
	
	Ogre::VertexBufferPacked* vertexBuffer = 0;

	Ogre::RenderSystem* renderSystem = root->getRenderSystem();
	Ogre::VaoManager* vaoManager = renderSystem->getVaoManager();

	vertexBuffer = vaoManager->createVertexBuffer(vertexElements, mesh->mNumVertices, Ogre::BT_IMMUTABLE, vertexs, true);

	// Now the Vao. We'll just use one vertex buffer source (multi-source not working yet)
	Ogre::VertexBufferPackedVec vertexBuffers;
	vertexBuffers.push_back(vertexBuffer);

	Ogre::IndexBufferPacked* indexBuffer = 0;
	indexBuffer = vaoManager->createIndexBuffer(Ogre::IndexBufferPacked::IT_32BIT, 3 * mesh->mNumFaces * 4,
		Ogre::BT_IMMUTABLE, faces, true);

	Ogre::VertexArrayObject* vao =
		vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer, Ogre::OT_TRIANGLE_LIST);

	// Each Vao pushed to the vector refers to an LOD level.
	// Must be in sync with mesh->mLodValues & mesh->mNumLods if you use more than one level
	sub_mesh->mVao[Ogre::VpNormal].push_back(vao);
	// Use the same geometry for shadow casting.
	sub_mesh->mVao[Ogre::VpShadow].push_back(vao);

	o_mesh->_setBounds(Ogre::Aabb(Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_SCALE), false);
	o_mesh->_setBoundingSphereRadius(1.732f);
};
void QOgreWidget::processNode(aiNode* node, const aiScene* scene, Ogre::MeshPtr o_mesh)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		processMesh(mesh, scene, o_mesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, o_mesh);
	}
};
bool QOgreWidget::LoadAndConvert(QString filename)
{
	Assimp::Importer importer;
	unsigned long flags = aiProcessPreset_TargetRealtime_Fast | aiProcess_TransformUVCoords | aiProcess_FlipUVs | aiProcess_Triangulate;
	flags &= ~(aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace); // optimize for fast loading

	//flags |= postProcessSteps;

	if ((flags & (aiProcess_GenSmoothNormals | aiProcess_GenNormals)) != aiProcess_GenNormals)
		flags &= ~aiProcess_GenNormals; // prefer smooth normals*/

	Ogre::String basename, ext, path;
	Ogre::StringUtil::splitFullFilename(filename.toStdString(), basename, ext, path);
	
	//const aiScene* scene = importer.ReadFile(filename.toStdString(), flags);

	const aiScene* scene = importer.ReadFile(filename.toStdString(), aiProcess_FindInvalidData | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_GenBoundingBoxes | aiProcess_GlobalScale | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	if (scene == 0)
		return false;

	Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual(basename + ".mesh", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	computeNodesDerivedTransform(scene, scene->mRootNode, scene->mRootNode->mTransformation);

	//root->getRenderSystem()->getTextureGpuManager()->createTexture()
	// Create embedded textures
	/*for (unsigned int i = 0; i < scene->mNumTextures; ++i)
	{
		const aiTexture* tex = scene->mTextures[i];
		auto texname =
			Ogre::StringUtil::format("%s%s%d.%s", mesh->getName().c_str(), tex->mFilename.C_Str(), i, tex->achFormatHint);
		if (Ogre::TextureManager::getSingleton().resourceExists(texname, mesh->getGroup()))
			continue;

		Image img;
		if (tex->mHeight == 0)
		{
			auto stream = std::make_shared<MemoryDataStream>(tex->pcData, tex->mWidth, false);
			try
			{
				img.load(stream, tex->achFormatHint);
			}
			catch (Exception& e)
			{
				LogManager::getSingleton().logError("Could not load embedded image - " + e.getDescription());
				continue;
			}
		}
		else
		{
			img.loadDynamicImage((uchar*)tex->pcData, tex->mWidth, tex->mHeight, PF_A8R8G8B8);
		}

		TextureManager::getSingleton().loadImage(texname, mesh->getGroup(), img);
	}*/

	Ogre::Aabb aabb = loadDataFromNode(scene, scene->mRootNode, mesh.get());

	// We must indicate the bounding box
	mesh->_setBounds(aabb);
	mesh->_setBoundingSphereRadius((aabb.getMaximum() - aabb.getMinimum()).length() / 2);

	//processNode(scene->mRootNode, scene, mesh);

	mesh->load();

	//Ogre::RenderSystem* renderSystem = root->getRenderSystem();
	//Ogre::VaoManager* vaoManager = renderSystem->getVaoManager();

	Ogre::MeshSerializer mesh_serializer(0);
	mesh_serializer.exportMesh(mesh.get(), path + basename + ".mesh");
	
	root->getHlmsManager()->saveMaterials(Ogre::HLMS_PBS, path + basename + ".material.json", 0,"");
	//root->getHlmsManager()->getHlms(Ogre::HLMS_PBS)->getDefaultDatablock();
			
	//root->getHlmsManager()->saveMaterial(root->getHlmsManager()->getHlms(Ogre::HLMS_PBS)->getDefaultDatablock(), "test.json", 0, "");

	/*Ogre::MaterialSerializer ms;
	for (Ogre::SubMesh* sm : mesh->getSubMeshes())
		ms.queueForExport(Ogre::MaterialManager::getSingleton().getByName(sm->getMaterialName()));
	ms.exportQueued("test.material");*/

	Ogre::Item* item = sm->createItem(mesh);
	if (meshNode)
	{
		sm->getRootSceneNode(Ogre::SCENE_DYNAMIC)->removeAndDestroyChild(meshNode);
	}

	meshNode = sm->getRootSceneNode(Ogre::SCENE_DYNAMIC)
		->createChildSceneNode(Ogre::SCENE_DYNAMIC);

	meshNode->attachObject((Ogre::MovableObject*)item);

	camera->setPosition(Ogre::Vector3(0, mesh->getBoundingSphereRadius() / 2.f, mesh->getBoundingSphereRadius()));
	return true;
};
Ogre::Mesh* QOgreWidget::Import(QString filename)
{
	Ogre::MeshPtr mesh;

	Assimp::Importer importer;
	unsigned long flags = aiProcessPreset_TargetRealtime_Fast | aiProcess_TransformUVCoords | aiProcess_FlipUVs | aiProcess_Triangulate;
	flags &= ~(aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace); // optimize for fast loading

	//flags |= postProcessSteps;

	if ((flags & (aiProcess_GenSmoothNormals | aiProcess_GenNormals)) != aiProcess_GenNormals)
		flags &= ~aiProcess_GenNormals; // prefer smooth normals*/

	Ogre::String basename, ext, path;
	Ogre::StringUtil::splitFullFilename(filename.toStdString(), basename, ext, path);

	//const aiScene* scene = importer.ReadFile(filename.toStdString(), flags);

	const aiScene* scene = importer.ReadFile(filename.toStdString(), aiProcess_FindInvalidData | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_GenBoundingBoxes | aiProcess_GlobalScale | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	if (scene == 0)
		return 0;

	mesh = Ogre::MeshManager::getSingleton().createManual(basename, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	computeNodesDerivedTransform(scene, scene->mRootNode, scene->mRootNode->mTransformation);

	Ogre::Aabb aabb = loadDataFromNode(scene, scene->mRootNode, mesh.get());
	// We must indicate the bounding box
	mesh->_setBounds(aabb);
	mesh->_setBoundingSphereRadius((aabb.getMaximum() - aabb.getMinimum()).length() / 2);
	mesh->load();

	Ogre::Item* item = sm->createItem(mesh);
	if (meshNode)
	{
		sm->getRootSceneNode(Ogre::SCENE_DYNAMIC)->removeAndDestroyChild(meshNode);
	}

	meshNode = sm->getRootSceneNode(Ogre::SCENE_DYNAMIC)
		->createChildSceneNode(Ogre::SCENE_DYNAMIC);

	meshNode->attachObject((Ogre::MovableObject*)item);

	camera->setPosition(Ogre::Vector3(0, mesh->getBoundingSphereRadius() / 2.f, mesh->getBoundingSphereRadius()));

	return mesh.getPointer();
};
Ogre::Mesh* QOgreWidget::LoadMesh(QString filename)
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
		
		Ogre::SkeletonInstance* sk_inst = item->getSkeletonInstance();// ->getAnimation("Take 001");
		if (sk_inst)
		{
			sk_inst->getAnimation("Idle")->setEnabled(true);
			sk_inst->getAnimation("Idle")->setLoop(true);
			//sk_inst->getAnimation("Idle")->addTime(1);
			//sa->setEnabled(true);
		}

		meshNode = sm->getRootSceneNode(Ogre::SCENE_DYNAMIC)->createChildSceneNode(Ogre::SCENE_DYNAMIC);

		meshNode->attachObject((Ogre::MovableObject*)item);

		camera->setPosition(Ogre::Vector3(0, mesh->getBoundingSphereRadius()/2.f,2*mesh->getBoundingSphereRadius()));
	}
	//sm->createEntity()
	return mesh.getPointer();
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
Ogre::HlmsDatablock* QOgreWidget::GetMaterial(const char* name)
{
	return root->getHlmsManager()->getMaterial(name);
};
void QOgreWidget::CreateScene(Ogre::MeshPtr mesh)
{
	Ogre::Item* item = sm->createItem(mesh);
	if (meshNode)
	{
		sm->getRootSceneNode(Ogre::SCENE_DYNAMIC)->removeAndDestroyChild(meshNode);
	}

	Ogre::SkeletonAnimation* sa = item->getSkeletonInstance()->getAnimation("Take 001");
	sa->setEnabled(true);
	sa->setLoop(true);
	//item->getSkeletonInstance()->update();

	meshNode = sm->getRootSceneNode(Ogre::SCENE_DYNAMIC)->createChildSceneNode(Ogre::SCENE_DYNAMIC);

	meshNode->attachObject((Ogre::MovableObject*)item);

	camera->setPosition(Ogre::Vector3(0, mesh->getBoundingSphereRadius() / 2.f, mesh->getBoundingSphereRadius()));
};
Ogre::Mesh *QOgreWidget::GetMesh()
{ 
	return ((Ogre::Item*)meshNode->getAttachedObject(0))->getMesh().getPointer();
}