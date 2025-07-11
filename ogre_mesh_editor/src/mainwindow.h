#ifndef MAIN_WINDOW_FILE
#define MAIN_WINDOW_FILE

#include<qmainwindow.h>
#include<Ogre.h>

typedef enum {
	MainWindowLogMessageType_Message,
	MainWindowLogMessageType_Warning,
	MainWindowLogMessageType_Error
}MainWindowLogMessageType;

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
	void on_actionImport_triggered();

	void writeLog(int type, QString message);
private:
	void Save();
	void UpdateProperty(Ogre::Mesh *mesh);

	Ui::MainWindow* ui;
	Ogre::Mesh* mesh;
};
#endif