#ifndef QOGREWINDGET_FILE
#define QOGREWINDGET_FILE

#include<qwidget.h>
#include<Ogre.h>
#include<assimp/scene.h>

#include <QRunnable>
#include <QThreadPool>

#include"progress_dialog.h"

class QOgreWidget :public QWidget, public Ogre::FrameListener
{
	Q_OBJECT
public:
	QOgreWidget(QWidget* parent = 0);
	~QOgreWidget();

	void Initialize();

	Ogre::Mesh *LoadMesh(QString filename);
	bool LoadAndConvert(QString filename);

	Ogre::Mesh* Import(QString filename);

	Ogre::HlmsDatablock* GetMaterial(const char* name);
	Ogre::HlmsManager* GetHlmsManager() { return root->getHlmsManager(); }
	Ogre::RenderSystem* GetRenderSystem() { return root->getRenderSystem(); }

	void render(QPainter* painter);
	void paintEvent(QPaintEvent* event);
	void render();

	void CreateScene(Ogre::MeshPtr mesh);
	Ogre::Mesh *GetMesh();
public slots:
	bool eventFilter(QObject* target, QEvent* event);
signals:
	void resizeWindow(unsigned long width, unsigned long height);
private:
	QPaintEngine* paintEngine() const;
	//void exposeEvent(QExposeEvent* event);
	bool event(QEvent* event);

	void mouseMoveEvent(QMouseEvent* e);
	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void wheelEvent(QWheelEvent* e);
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);
	void resizeEvent(QResizeEvent* e);
	
	void initialiseHLMS();

	void processNode(aiNode *node, const aiScene* scene, Ogre::MeshPtr o_mesh);
	void processMesh(aiMesh* mesh, const aiScene* scene, Ogre::MeshPtr o_mesh);

	void computeNodesDerivedTransform(const aiScene* mScene, const aiNode* pNode, const aiMatrix4x4& accTransform);
	Ogre::HlmsDatablock *createMaterial(const aiMaterial* mat, const Ogre::String& group, const Ogre::String& meshName, const aiScene* scene);
	bool createSubMesh(const Ogre::String& name, int index, const aiNode* pNode, const aiMesh* mesh, Ogre::HlmsDatablock *matptr, Ogre::Mesh* mMesh, Ogre::Aabb& mAAB);
	Ogre::Aabb loadDataFromNode(const aiScene* scene, aiNode* node, Ogre::Mesh* mesh);

	Ogre::MeshPtr LoadMeshV1(QString filename);
	Ogre::MeshPtr LoadMeshV2(QString filename);

	Ogre::Root* root;
	Ogre::Window* window;
	Ogre::Camera* camera;
	Ogre::SceneManager* sm;
	Ogre::SceneNode* meshNode;

	bool mouse_down;
	QPoint mouse_position;

	typedef std::map<Ogre::String, aiMatrix4x4> NodeTransformMap;
	NodeTransformMap mNodeDerivedTransformByName;
};

class QImport :public QRunnable
{
public:
	QImport(QOgreWidget *widget, Ogre::HlmsManager* _manager, Ogre::RenderSystem *rs, ProgressDialog* _pd, QString _filename);

	void run();
private:
	void prepare(aiNode* node, unsigned long &count);
	void computeNodesDerivedTransform(const aiScene* mScene, const aiNode* pNode, const aiMatrix4x4& accTransform);
	Ogre::Aabb loadDataFromNode(const aiScene* scene, aiNode* node, Ogre::Mesh* mesh);
	Ogre::HlmsDatablock* createMaterial(const aiMaterial* mat, const Ogre::String& group, const Ogre::String& meshName, const aiScene* scene);
	bool createSubMesh(const Ogre::String& name, int index, const aiNode* pNode, const aiMesh* mesh, Ogre::HlmsDatablock* db, Ogre::Mesh* mMesh, Ogre::Aabb& mAAB);
	
	QOgreWidget* ow;

	Ogre::HlmsManager* manager;
	Ogre::RenderSystem* renderSystem;

	Ogre::MeshPtr mesh;
	ProgressDialog* pd;
	QString filename;

	typedef std::map<Ogre::String, aiMatrix4x4> NodeTransformMap;
	NodeTransformMap mNodeDerivedTransformByName;
};
#endif