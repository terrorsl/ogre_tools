#include<qapplication.h>
#include"mainwindow.h"

int main(int argc, char **argv)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	QApplication app(argc, argv);
	QMainWindow* window = new MainWindow();
	window->show();
	app.exec();
	delete window;
	return 0;
}