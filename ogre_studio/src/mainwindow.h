#ifndef MAIN_WINDOW_FILE
#define MAIN_WINDOW_FILE

#include<qmainwindow.h>
#include"os_level.h"
#include<qtreewidget.h>

namespace Ui {
	class MainWindow;
};

enum ObjectType
{
	LightObjectType,
};

class MainWindow :public QMainWindow
{
	Q_OBJECT
public:
	MainWindow();
	~MainWindow();
public slots:
	void renderOgre();

	void on_actionNew_triggered();
	void on_actionOpen_triggered();

	void objects_double_click(QTreeWidgetItem*, int);
private:
	void LoadObjects();

	Ui::MainWindow* ui;

	OgreStudioLevel* level;
	QStringList objects;
};
#endif