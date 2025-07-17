#ifndef MAIN_WINDOW_FILE
#define MAIN_WINDOW_FILE

#include<qmainwindow.h>
#include<Ogre.h>

#include<qtreewidget.h>

typedef enum {
	MainWindowLogMessageType_Message,
	MainWindowLogMessageType_Warning,
	MainWindowLogMessageType_Error
}MainWindowLogMessageType;

typedef enum {
	MaterialType_Color,
	MaterialType_Texture
}MaterialType;

namespace Ui {
	class MainWindow;
};
class MainWindow :public QMainWindow
{
	Q_OBJECT
public:
	MainWindow();
	~MainWindow();
public slots:
	void renderOgre();

	void on_actionOpen_triggered();
	void on_actionSave_triggered();
	void on_actionImport_triggered();

	void animation_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void play_clicked();

	void material_itemDoubleClicked(QTreeWidgetItem* item, int column);

	void writeLog(int type, QString message);
private:
	void Save();
	void UpdateProperty(Ogre::Mesh *mesh);

	Ui::MainWindow* ui;
	Ogre::Mesh* mesh;
};
#endif