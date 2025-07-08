#include"os_platform.h"
#include"os_physics.h"
#include<OgreItem.h>
#include<OgreMesh2.h>
#include<OgreSubMesh2.h>

OgreStudioVertexIndexToShape::OgreStudioVertexIndexToShape(Ogre::SceneNode* node)
{
	vertexCount = indexCount = 0;
	vertexs = 0;
	indexes = 0;

	Ogre::Item* item = (Ogre::Item*)node->getAttachedObject(0);
	size_t offset = 0;

	Ogre::Matrix4 tr;
	Ogre::Matrix3 rt;
	node->getOrientation().ToRotationMatrix(rt);
	tr = rt;
	tr.setTrans(node->getPosition());
	for (int smi = 0; smi < item->getMesh()->getNumSubMeshes(); smi++)
	{
		Ogre::SubMesh* sm = item->getMesh()->getSubMesh(smi);
		/*size_t outIndex, outOffset;
		const Ogre::VertexElement2* posElem = sm->mVao[0][0]->findBySemantic(Ogre::VES_POSITION, outIndex, outOffset);
		size_t num_element = sm->mVao[0][0]->getVertexBuffers()[0]->getNumElements();*/

		Ogre::VertexArrayObject::ReadRequestsVec requests;
		requests.push_back(Ogre::VertexArrayObject::ReadRequests(Ogre::VES_POSITION));

		sm->mVao[0][0]->readRequests(requests);
		sm->mVao[0][0]->mapAsyncTickets(requests);

		addVertexData(requests[0], tr);

		/*for (int i = 0; i < sm->mVao[0][0]->getPrimitiveCount(); i++)
		{
			float const* position = reinterpret_cast<const float*>(requests[0].data);
			requests[0].data += requests[0].vertexBuffer->getBytesPerElement();
		}*/
		sm->mVao[0][0]->unmapAsyncTickets(requests);

		size_t ic = sm->mVao[0][0]->getIndexBuffer()->getNumElements();
		Ogre::AsyncTicketPtr atp = sm->mVao[0][0]->getIndexBuffer()->readRequest(0, ic);
		while (!atp->queryIsTransferDone());
		const char* data = (const char*)atp->map();
		addIndexData(data, sm->mVao[0][0]->getIndexBuffer()->getIndexType(), ic, offset);
		atp->unmap();
		offset = vertexCount;
		//Ogre::AsyncTicketPtr vt = sm->mVao[0][0]->getVertexBuffers()[0]->readRequest(0, num_element);
		
		//sm->mVao[0][0]->getIndexBuffer()->getIndexType();
		//sm->mVao[0][0]->getIndexBuffer()->getNumElements();
		//Ogre::AsyncTicketPtr t = sm->mVao[0][0]->getIndexBuffer()->readRequest(0, sm->mVao[0][0]->getIndexBuffer()->getNumElements());
		//while(!t->queryIsTransferDone());
		//const unsigned long *data = (const unsigned long *)t->map();
		//t->unmap();
		//void *data = sm->mVao[0][0]->getIndexBuffer()->map(0, sm->mVao[0][0]->getIndexBuffer()->getNumElements());
		//sm->mVao[0][0]->getIndexBuffer()->unmap(Ogre::UO_UNMAP_ALL);
		//sm->mVao[0][0]->getVertexBuffers()[0]->m
	}
	//Ogre::VertexArrayObject* vao = *item->getMesh()->getSubMesh(0);
#if 0
	// Each entity added need to reset size and radius
	// next time getRadius and getSize are asked, they're computed.
	mBounds = Ogre::Vector3(-1, -1, -1);
	mBoundRadius = -1;

	mTransform = transform;
	mScale = node ? node->getScale() : Ogre::Vector3(1, 1, 1);

	Ogre::Item *item = (Ogre::Item*)node->getAttachedObject(0);
	item->getMesh()->getSubMesh(0)
	bool hasSkeleton = entity->hasSkeleton();

	if (entity->getMesh()->sharedVertexData)
	{
		if (hasSkeleton)
			addAnimatedVertexData(entity->getMesh()->sharedVertexData, entity->_getSkelAnimVertexData(),
				&entity->getMesh()->sharedBlendIndexToBoneIndexMap);
		else
			addStaticVertexData(entity->getMesh()->sharedVertexData);
	}

	for (unsigned int i = 0; i < entity->getNumSubEntities(); ++i)
	{
		SubMesh* sub_mesh = entity->getSubEntity(i)->getSubMesh();

		if (!sub_mesh->useSharedVertices)
		{
			addIndexData(sub_mesh->indexData, mVertexCount);

			if (hasSkeleton)
				addAnimatedVertexData(sub_mesh->vertexData,
					entity->getSubEntity(i)->_getSkelAnimVertexData(),
					&sub_mesh->blendIndexToBoneIndexMap);
			else
				addStaticVertexData(sub_mesh->vertexData);
		}
		else
		{
			addIndexData(sub_mesh->indexData);
		}

	}
#endif
};
OgreStudioVertexIndexToShape::~OgreStudioVertexIndexToShape()
{
	if (vertexs)
		delete[] vertexs;
	if (indexes)
		delete[] indexes;

};
void OgreStudioVertexIndexToShape::addVertexData(Ogre::VertexArrayObject::ReadRequests buffer, Ogre::Matrix4 tr)
{
	unsigned long prev_size = vertexCount;
	vertexCount += buffer.vertexBuffer->getNumElements();
	Ogre::Vector3* tmp = OS_NEW Ogre::Vector3[vertexCount];
	if (vertexs)
	{
		memcpy(tmp, vertexs, sizeof(Ogre::Vector3) * prev_size);
		delete[] vertexs;
	}
	vertexs = tmp;
	Ogre::Vector3* curVertex = &vertexs[prev_size];
	for (unsigned long index = 0; index < buffer.vertexBuffer->getNumElements(); index++)
	{
		//Ogre::Vector3* curVertexData = (Ogre::Vector3*)buffer.data;
		const float* p = (const float*)buffer.data;

		curVertex->x = *p;
		curVertex->y = *(p+1);
		curVertex->z = *(p+2);

		//*curVertex = tr * *curVertex;

		curVertex++;
		buffer.data += buffer.vertexBuffer->getBytesPerElement();
	}
};
void OgreStudioVertexIndexToShape::addIndexData(const char* data, Ogre::IndexType type, unsigned long _indexCount, unsigned long offset)
{
	unsigned int prev_size = indexCount;
	indexCount += _indexCount;
	unsigned int* tmp = OS_NEW unsigned int[indexCount];
	if (indexes)
	{
		memcpy(tmp, indexes, sizeof(unsigned int) * prev_size);
		delete[] indexes;
	}
	indexes = tmp;

	if (type == Ogre::IT_32BIT)
	{
		unsigned int* ind = (unsigned int*)data;
		for (unsigned int index = 0; index < _indexCount; index++)
		{
			indexes[prev_size + index] = offset + ind[index];
		}
	}
	else
	{
		unsigned short* ind = (unsigned short*)data;
		for (unsigned int index = 0; index < _indexCount; index++)
		{
			indexes[prev_size + index] = offset + static_cast<unsigned int>(ind[index]);
		}
	}
};
btBvhTriangleMeshShape* OgreStudioVertexIndexToShape::createTrimesh()
{
	btTriangleMesh* trimesh = new btTriangleMesh(true, false);
	unsigned int numFaces = indexCount / 3;
	unsigned int* ind = indexes;
	btVector3 vertexPos[3];
	for (unsigned int f = 0; f < numFaces; f++)
	{
		vertexPos[0][0] = vertexs[*ind].x;
		vertexPos[0][1] = vertexs[*ind].y;
		vertexPos[0][2] = vertexs[*ind].z;

		vertexPos[1][0] = vertexs[*(ind + 1)].x;
		vertexPos[1][1] = vertexs[*(ind + 1)].y;
		vertexPos[1][2] = vertexs[*(ind + 1)].z;

		vertexPos[2][0] = vertexs[*(ind + 2)].x;
		vertexPos[2][1] = vertexs[*(ind + 2)].y;
		vertexPos[2][2] = vertexs[*(ind + 2)].z;

		ind += 3;

		trimesh->addTriangle(vertexPos[0], vertexPos[1], vertexPos[2]);
	}
	btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(trimesh, true);
	return shape;
};
btConvexHullShape* OgreStudioVertexIndexToShape::createConvex()
{
	btConvexHullShape* shape = new btConvexHullShape((btScalar*)&vertexs[0].x, vertexCount, sizeof(Ogre::Vector3));

	return shape;
};

OgrestudioPhysicsDebugDraw::OgrestudioPhysicsDebugDraw(Ogre::ManualObject* line, btDiscreteDynamicsWorld* world):lines(line),debugMode(DBG_DrawWireframe)
{
	world->setDebugDrawer(this);
};
void OgrestudioPhysicsDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	if (lines->getNumSections() == 0)
	{
		lines->begin("Ogre/Debug/LinesMat", Ogre::OT_LINE_LIST);
	}
	else
	{
		if (lines->getCurrentVertexCount() == 0)
			lines->beginUpdate(0);
	}
	Ogre::ColourValue col(color.x(), color.y(), color.z());
	int index = lines->getCurrentVertexCount();
	lines->position(Ogre::Vector3(from.x(), from.y(), from.z()));
	lines->colour(col);
	lines->position(Ogre::Vector3(to.x(), to.y(), to.z()));
	lines->colour(col);

	lines->line(index, index + 1);
};
void OgrestudioPhysicsDebugDraw::reportErrorWarning(const char* warningString)
{
};
void OgrestudioPhysicsDebugDraw::draw3dText(const btVector3& location, const char* textString)
{
};
void OgrestudioPhysicsDebugDraw::setDebugMode(int debugMode)
{
	this->debugMode = debugMode;
	if (debugMode == DBG_NoDebug)
		clear();
};
int OgrestudioPhysicsDebugDraw::getDebugMode() const
{
	return debugMode;
};
void OgrestudioPhysicsDebugDraw::update(btDiscreteDynamicsWorld *world)
{
	world->debugDrawWorld();
	if (lines->getNumSections() && lines->getCurrentVertexCount())
		lines->end();
};
void OgrestudioPhysicsDebugDraw::clear()
{
	lines->clear();
};

OgreStudioPhysics::~OgreStudioPhysics()
{
	if (debugDraw)
		delete debugDraw;

	while (dynamicsWorld->getNumCollisionObjects())
	{
		btCollisionObject *obj = dynamicsWorld->getCollisionObjectArray()[0];
		btRigidBody* body = btRigidBody::upcast(obj);

		btCollisionShape *shape = body->getCollisionShape();
		switch (shape->getShapeType()) {
		case CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE:
			delete ((btConvexTriangleMeshShape*)shape)->getMeshInterface();
			break;
		case TRIANGLE_MESH_SHAPE_PROXYTYPE:
			delete ((btBvhTriangleMeshShape*)shape)->getMeshInterface();
			break;
		}

		delete shape;
		delete body->getMotionState();
		dynamicsWorld->removeRigidBody(body);
		delete body;
	}

	delete dynamicsWorld;
	delete solver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;
};
void OgreStudioPhysics::Initialize()
{
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	debugDraw = 0;
};
void OgreStudioPhysics::Update(float dt)
{
	dynamicsWorld->stepSimulation(dt, 10);
	if (debugDraw)
		debugDraw->update(dynamicsWorld);

	for (int i = 0; i < dynamicsWorld->getNumCollisionObjects(); i++)
	{
		btCollisionObject *obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if(body && body->getMotionState())
		{
			/*btTransform transform;
			body->getMotionState()->getWorldTransform(transform);
			Ogre::SceneNode* node = (Ogre::SceneNode*)body->getUserPointer();

			node->setPosition(Ogre::Vector3(transform.getOrigin().x(), transform.getOrigin().y(), transform.getOrigin().z()));
			node->setOrientation(Ogre::Quaternion(transform.getRotation().w(), transform.getRotation().x(), transform.getRotation().y(), transform.getRotation().z()));*/
		}
	}
};
void OgreStudioPhysics::New(Ogre::SceneManager* sm)
{
	Ogre::ManualObject* mo = sm->createManualObject();
	sm->getRootSceneNode()->attachObject(mo);
	if (debugDraw)
		delete debugDraw;
	debugDraw=OS_NEW OgrestudioPhysicsDebugDraw(mo, dynamicsWorld);
};
void OgreStudioPhysics::AppendObject(Ogre::SceneNode* node)
{
	Ogre::MovableObject* obj = node->getAttachedObject(0);
	Ogre::Aabb aabb = obj->getLocalAabb();

	btTransform transform;
	//transform.setOrigin(btVector3(0,0,));
	transform.setIdentity();
	transform.setRotation(btQuaternion(node->getOrientation().x, node->getOrientation().y, node->getOrientation().z, node->getOrientation().w));
	
	OgreStudioVertexIndexToShape v(node);

	//delete v.createTrimesh();
	btCollisionShape* shape = v.createTrimesh();
	//btCollisionShape* shape = v.createConvex();
	//btCollisionShape* shape=new btBoxShape(btVector3(aabb.mHalfSize.x, aabb.mHalfSize.y, aabb.mHalfSize.z));

	btScalar mass = 0.f;
	if (node->isStatic()==false)
	{
		mass = 1;
	}
	btMotionState* motionState = OS_NEW OgreStudioPhysicsMotionState(node);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape);
	btRigidBody* body = new btRigidBody(rbInfo);
	//body->setRestitution
	body->setUserPointer(node);
	dynamicsWorld->addRigidBody(body);
};
void OgreStudioPhysics::RemoveObject(Ogre::SceneNode* node)
{
	for (int index = 0; index < dynamicsWorld->getNumCollisionObjects(); index++)
	{
		btRigidBody* body = btRigidBody::upcast(dynamicsWorld->getCollisionObjectArray()[index]);
		if (node == body->getUserPointer())
		{
			btCollisionShape* shape = body->getCollisionShape();
			switch (shape->getShapeType()) {
			case CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE:
				delete ((btConvexTriangleMeshShape*)shape)->getMeshInterface();
				break;
			case TRIANGLE_MESH_SHAPE_PROXYTYPE:
				delete ((btBvhTriangleMeshShape*)shape)->getMeshInterface();
				break;
			}

			delete shape;
			delete body->getMotionState();
			dynamicsWorld->removeRigidBody(body);
			delete body;

			debugDraw->clear();
			return;
		}
	}
};
bool OgreStudioPhysics::IsObjectInWorld(Ogre::SceneNode* node)
{
	for (int index = 0; index < dynamicsWorld->getNumCollisionObjects(); index++)
	{
		btRigidBody* body = btRigidBody::upcast(dynamicsWorld->getCollisionObjectArray()[index]);
		if (node == body->getUserPointer())
			return true;
	}
	return false;
};