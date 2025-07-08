#ifndef MAIN_WINDOW_FILE
#define MAIN_WINDOW_FILE

#include<qmainwindow.h>
#include"os_level.h"
#include"os_physics.h"
#include<qtreewidget.h>
#include"os_plugin_manager.h"

namespace Ui {
	class MainWindow;
};

enum ObjectType
{
	LightObjectType,
	ObjectObjectType
};

class MainWindow :public QMainWindow, public OgreStudioLevelCallback
{
	Q_OBJECT
public:
	MainWindow();
	~MainWindow();

	void CreateNode(Ogre::SceneNode *node, OgreStudioObjectType type);
	void ReceiveMessage(int type, const std::string& message);
public slots:
	void resizeWindow(unsigned long width, unsigned long height);
	void mouseMove(float dx, float dy);
	void mouseWheel(float value);
	
	void renderOgre();

	void on_actionNew_triggered();
	void on_actionOpen_triggered();
	void on_actionSave_triggered();
	void on_actionExport_triggered();

	void objects_double_click(QTreeWidgetItem*, int);
	void level_itemClicked(QTreeWidgetItem*, int);
	void level_itemChanged(QTreeWidgetItem*, int);

	void collision_currentIndexChanged(int index);

	void px_valueChanged(double);
	void py_valueChanged(double);
	//void pz_valueChanged(double);

	void range_light_valueChanged(double);

	void diffuse_light_pressed();
	void specular_light_pressed();
private:
	void AppendLevel(OgreStudioObjectType type, void *data, std::string name);
	void LoadObjects();
	void PrepareLevel();
	void UpdateCommonProperties(Ogre::SceneNode *node);
	void UpdateLightProperties(Ogre::SceneNode* node);

	Ui::MainWindow* ui;
	QTimer* timer;

	OgreStudioLevel* level;
	QStringList objects;

	OgreStudioPhysics* physic;

	QMap<int, QTreeWidgetItem*> root_level_items;

	OgreStudioPluginManager plugin_manager;
};
#endif