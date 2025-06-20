#include"mainwindow.h"
//#include"qogrewidget.h"
#include"ui_mainwindow.h"
#include<qtimer.h>
#include<qfiledialog.h>

MainWindow::MainWindow():ui(new Ui::MainWindow()), level(0)
{
	ui->setupUi(this);

	QObject::connect(ui->objects, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(objects_double_click(QTreeWidgetItem*, int)));

	ui->ogrewidget->Initialize();
	ui->ogrewidget->show();

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(renderOgre()));
	timer->start(30);

	setMouseTracking(true);

	LoadObjects();
};
MainWindow::~MainWindow()
{
	if (level)
		delete level;
	delete ui;
};
void MainWindow::LoadObjects()
{
	QDir dir("models");
	objects = dir.entryList(QStringList() << "*.mesh");
	for (QStringList::iterator file = objects.begin(); file != objects.end(); file++)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, *file);
		item->setData(0, Qt::UserRole, *file);

		ui->objects->addTopLevelItem(item);
	}
	QTreeWidgetItem *root_lights = new QTreeWidgetItem(QStringList() << "lights");
	root_lights->setData(0, Qt::UserRole, LightObjectType);
	ui->objects->addTopLevelItem(root_lights);
	for (int type = Ogre::Light::LT_DIRECTIONAL; type < Ogre::Light::NUM_LIGHT_TYPES; type++)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem();
		switch (type)
		{
		case Ogre::Light::LT_DIRECTIONAL:
			item->setText(0, "directional");
			break;
		case Ogre::Light::LT_POINT:
			item->setText(0, "point");
			break;
		case Ogre::Light::LT_SPOTLIGHT:
			item->setText(0, "spot");
			break;
		default:
			item->setText(0, "other");
		}
		item->setData(0, Qt::UserRole, type);
		root_lights->addChild(item);
	}
};
void MainWindow::renderOgre()
{
	ui->ogrewidget->render();
};
void MainWindow::on_actionNew_triggered()
{
	if (level)
	{
		ui->ogrewidget->DeleteSceneManager(level->GetSceneManager());
	}
	Ogre::SceneManager *sm = ui->ogrewidget->CreateSceneManager();
	std::string name("default.level.json");
	level = OGRE_NEW OgreStudioLevel(name, sm);
	level->Save();
};
void MainWindow::on_actionOpen_triggered()
{
	QString filename = QFileDialog::getOpenFileName(this);
	if (filename.isEmpty())
		return;
	ui->ogrewidget->LoadMesh(filename);
};
void MainWindow::objects_double_click(QTreeWidgetItem *item, int)
{
	if (item->parent() == 0)
		return;
	switch (item->parent()->data(0, Qt::UserRole).toInt())
	{
	case LightObjectType:
		level->CreateLight(item->data(0, Qt::UserRole).toInt());
		break;
	}
};