#ifndef MAIN_WINDOW_FILE
#define MAIN_WINDOW_FILE

#include<qmainwindow.h>

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
private:
	Ui::MainWindow* ui;
};
#endif