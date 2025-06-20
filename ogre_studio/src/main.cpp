#include<qapplication.h>
#include"mainwindow.h"

int main(int argc, char **argv)
{
#if defined(WIN32) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	QApplication app(argc, argv);
	QMainWindow* window = new MainWindow();
	window->show();
	app.exec();
	delete window;
	return 0;
}