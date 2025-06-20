#include"mainwindow.h"
//#include"qogrewidget.h"
#include"ui_mainwindow.h"
#include<qtimer.h>
#include<qfiledialog.h>

MainWindow::MainWindow():ui(new Ui::MainWindow())
{
	ui->setupUi(this);

	ui->ogrewidget->Initialize();
	ui->ogrewidget->show();

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(renderOgre()));
	timer->start(30);

	setMouseTracking(true);
};
MainWindow::~MainWindow()
{
	delete ui;
};
void MainWindow::renderOgre()
{
	ui->ogrewidget->render();
};
void MainWindow::on_actionOpen_triggered()
{
	QString filename = QFileDialog::getOpenFileName(this);
	if (filename.isEmpty())
		return;
	ui->ogrewidget->LoadMesh(filename);
};