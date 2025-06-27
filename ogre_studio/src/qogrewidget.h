#ifndef QOGREWINDGET_FILE
#define QOGREWINDGET_FILE

#include<qwidget.h>
#include<Ogre.h>

class QOgreWidget :public QWidget, public Ogre::FrameListener
{
	Q_OBJECT
public:
	QOgreWidget(QWidget* parent = 0);
	~QOgreWidget();

	void Initialize();

	bool LoadMesh(QString filename);

	Ogre::Root* GetRoot() { return root; }
	Ogre::SceneManager* CreateSceneManager();
	void DeleteSceneManager(Ogre::SceneManager* sm);

	void render(QPainter* painter);
	void paintEvent(QPaintEvent* event);
	void render();
public slots:
	bool eventFilter(QObject* target, QEvent* event);
signals:
	void resizeWindow(unsigned long width, unsigned long height);
	void mouseMove(float dx, float dy);
	void mouseWheel(float value);
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

	Ogre::MeshPtr LoadMeshV1(QString filename);
	Ogre::MeshPtr LoadMeshV2(QString filename);

	Ogre::Root* root;
	Ogre::Window* window;
	Ogre::Camera* camera;
	Ogre::SceneManager* sm;
	Ogre::SceneNode* meshNode;

	bool mouse_down;
	QPoint mouse_position;
};
#endif