#include"mainwindow.h"
//#include"qogrewidget.h"
#include"ui_mainwindow.h"
#include<qtimer.h>
#include<qfiledialog.h>
#include<qcolordialog.h>

MainWindow::MainWindow():ui(new Ui::MainWindow()), level(0)
{
	ui->setupUi(this);

	QObject::connect(ui->objects, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(objects_double_click(QTreeWidgetItem*, int)));
	QObject::connect(ui->levelTree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(level_itemClicked(QTreeWidgetItem*, int)));

	QObject::connect(ui->px, SIGNAL(valueChanged(double)), this, SLOT(px_valueChanged(double)));
	QObject::connect(ui->py, SIGNAL(valueChanged(double)), this, SLOT(py_valueChanged(double)));

	QObject::connect(ui->range_light, SIGNAL(valueChanged(double)), this, SLOT(range_light_valueChanged(double)));

	QObject::connect(ui->diffuse_light, SIGNAL(pressed()), this, SLOT(diffuse_light_pressed()));
	QObject::connect(ui->specular_light, SIGNAL(pressed()), this, SLOT(specular_light_pressed()));

	QObject::connect(ui->ogrewidget, SIGNAL(resizeWindow(unsigned long, unsigned long)), this, SLOT(resizeWindow(unsigned long, unsigned long)));
	QObject::connect(ui->ogrewidget, SIGNAL(mouseMove(float, float)), this, SLOT(mouseMove(float, float)));
	QObject::connect(ui->ogrewidget, SIGNAL(mouseWheel(float)), this, SLOT(mouseWheel(float)));

	ui->ogrewidget->Initialize();
	ui->ogrewidget->show();

	ui->properyDock->setEnabled(false);
	ui->projectDock->setEnabled(false);

	physic = new OgreStudioPhysics();
	physic->Initialize();

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(renderOgre()));
	timer->start(30);

	setMouseTracking(true);

	LoadObjects();
	PrepareLevel();
};
MainWindow::~MainWindow()
{
	if (level)
		delete level;

	delete physic;

	delete ui;
};
void MainWindow::LoadObjects()
{
	QDir dir("models");
	objects = dir.entryList(QStringList() << "*.mesh");
	QTreeWidgetItem* root_objects = new QTreeWidgetItem(QStringList() << "objects");
	root_objects->setData(0, Qt::UserRole, OgreStudioObjectType_Mesh);
	ui->objects->addTopLevelItem(root_objects);
	for (QStringList::iterator file = objects.begin(); file != objects.end(); file++)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, *file);
		item->setData(0, Qt::UserRole, *file);

		root_objects->addChild(item);
	}
	QTreeWidgetItem *root_lights = new QTreeWidgetItem(QStringList() << "lights");
	root_lights->setData(0, Qt::UserRole, OgreStudioObjectType_Light);
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
void MainWindow::PrepareLevel()
{
	ui->levelTree->clear();
	root_level_items.clear();

	QList<QTreeWidgetItem*> items;
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText(0, "lights");
	items.append(item);
	root_level_items.insert(OgreStudioObjectType_Light, item);
	
	item = new QTreeWidgetItem();
	item->setText(0, "objects");
	items.append(item);
	root_level_items.insert(OgreStudioObjectType_Mesh, item);
	
	ui->levelTree->addTopLevelItems(items);
};
void MainWindow::UpdateCommonProperties(Ogre::SceneNode* node)
{
	Ogre::Vector3 position = node->getPosition();
	//node->getAttachedObject(0)

	ui->px->blockSignals(true);
	ui->px->setValue(position.x);
	ui->px->blockSignals(false);
};
void MainWindow::px_valueChanged(double value)
{
	Ogre::SceneNode* node = (Ogre::SceneNode*)ui->levelTree->currentItem()->data(0, Qt::UserRole).value<void*>();
	Ogre::Vector3 p = node->getPosition();
	p.x = value;
	node->setPosition(p);
};
void MainWindow::py_valueChanged(double value)
{
	Ogre::SceneNode* node = (Ogre::SceneNode*)ui->levelTree->currentItem()->data(0, Qt::UserRole).value<void*>();
	Ogre::Vector3 p = node->getPosition();
	p.y = value;
	node->setPosition(p);
};
void MainWindow::range_light_valueChanged(double range)
{
	Ogre::SceneNode* node = (Ogre::SceneNode*)ui->levelTree->currentItem()->data(0, Qt::UserRole).value<void*>();
	Ogre::Light *light = (Ogre::Light*)node->getAttachedObject(0);
	//light->setAttenuationBasedOnRadius(range, 0);
	light->setAttenuation(range, 0.5, 0.01, 0.001);
};
void MainWindow::diffuse_light_pressed()
{
	QColor initColor =(QColor)ui->diffuse_light->palette().color(QPalette::Button);
	QColorDialog dialog(initColor, this);
	if (dialog.exec() == QDialog::Accepted)
	{
		QColor color = dialog.currentColor();
		QPalette palette = ui->diffuse_light->palette();
		palette.setColor(QPalette::Button, color);
		ui->diffuse_light->setPalette(palette);
		ui->diffuse_light->update();

		Ogre::SceneNode* node = (Ogre::SceneNode*)ui->levelTree->currentItem()->data(0, Qt::UserRole).value<void*>();
		Ogre::Light *light = (Ogre::Light*)node->getAttachedObject(0);

		Ogre::ColourValue cv(color.redF(), color.greenF(), color.blueF());
		light->setDiffuseColour(cv);
	}
};
void MainWindow::specular_light_pressed()
{
	QColor initColor = (QColor)ui->specular_light->palette().color(QPalette::Button);
	QColorDialog dialog(initColor, this);
	if (dialog.exec() == QDialog::Accepted)
	{
		QColor color = dialog.currentColor();
		QPalette palette = ui->specular_light->palette();
		palette.setColor(QPalette::Button, color);
		ui->specular_light->setPalette(palette);
		ui->specular_light->update();

		Ogre::SceneNode* node = (Ogre::SceneNode*)ui->levelTree->currentItem()->data(0, Qt::UserRole).value<void*>();
		Ogre::Light* light = (Ogre::Light*)node->getAttachedObject(0);

		Ogre::ColourValue cv(color.redF(), color.greenF(), color.blueF());
		light->setSpecularColour(cv);
	}
};
void MainWindow::renderOgre()
{
	ui->ogrewidget->render();
	physic->Update(30);
};
void MainWindow::on_actionNew_triggered()
{
	if (level)
	{
		ui->ogrewidget->DeleteSceneManager(level->GetSceneManager());
	}
	Ogre::SceneManager *sm = ui->ogrewidget->CreateSceneManager();
	std::string name("default.level.json");
	level = OGRE_NEW OgreStudioLevel(name, ui->ogrewidget->GetRoot(), sm, this);
	level->resizeCamera(ui->ogrewidget->size().width(), ui->ogrewidget->size().height());
	level->Save();

	physic->New(sm);

	ui->projectDock->setEnabled(true);
};
void MainWindow::on_actionOpen_triggered()
{
	QString filename = QFileDialog::getOpenFileName(this);
	if (filename.isEmpty())
		return;
	std::string name = filename.toStdString();
	Ogre::SceneManager* sm = ui->ogrewidget->CreateSceneManager();
	OgreStudioLevel* nl = new OgreStudioLevel(name, ui->ogrewidget->GetRoot(), sm, this);
	if (nl->Load(name))
	{
		if (level)
			delete level;
		level = nl;
		physic->New(sm);
		ui->projectDock->setEnabled(true);
	}
	else
		delete nl;
};
void MainWindow::on_actionSave_triggered()
{
	level->Save();
};
void MainWindow::objects_double_click(QTreeWidgetItem *item, int)
{
	if (item->parent() == 0)
		return;
	Ogre::SceneNode* node;
	OgreStudioObjectType type = (OgreStudioObjectType)item->parent()->data(0, Qt::UserRole).toInt();
	switch (type)
	{
	case OgreStudioObjectType_Light:
		node = level->CreateLight(item->data(0, Qt::UserRole).toInt());
		break;
	case OgreStudioObjectType_Mesh:
		{
			std::string name = item->data(0, Qt::UserRole).toString().toStdString();
			node = level->CreateDynamicObject(name);
			//physic->AppendObject(node);
		}
		break;
	}
	AppendLevel(type, node);
};
void MainWindow::level_itemClicked(QTreeWidgetItem *item, int column)
{
	if (item->parent() == 0)
	{
		ui->properyDock->setEnabled(false);
		return;
	}

	Ogre::SceneNode *node = (Ogre::SceneNode*)item->data(column, Qt::UserRole).value<void*>();
	if (node == 0)
		return;
	level->SelectNode(node);

	ui->properyDock->setEnabled(true);

	UpdateCommonProperties(node);
	UpdateLightProperties(node);
};
void MainWindow::UpdateLightProperties(Ogre::SceneNode* node)
{
	Ogre::Any type = node->getUserObjectBindings().getUserAny("type");

	if (Ogre::any_cast<OgreStudioObjectType>(type) != OgreStudioObjectType_Light)
	{
		ui->lightGroup->setVisible(false);
		return;
	}
	ui->lightGroup->setVisible(true);

	Ogre::Light* light = (Ogre::Light*)node->getAttachedObject(0);
	Ogre::ColourValue color = light->getDiffuseColour();
	
	QColor qcolor(color.r * 255, color.g * 255, color.b * 255);
	QPalette palette = ui->diffuse_light->palette();
	palette.setColor(QPalette::Button, qcolor);
	ui->diffuse_light->blockSignals(true);
	ui->diffuse_light->setPalette(palette);
	ui->diffuse_light->blockSignals(false);

	color = light->getSpecularColour();
	qcolor= QColor(color.r * 255, color.g * 255, color.b * 255);
	palette = ui->specular_light->palette();
	palette.setColor(QPalette::Button, qcolor);
	ui->specular_light->blockSignals(true);
	ui->specular_light->setPalette(palette);
	ui->specular_light->blockSignals(false);

	float range = light->getAttenuationRange();
	ui->range_light->blockSignals(true);
	ui->range_light->setValue(range);
	ui->range_light->blockSignals(false);
};
void MainWindow::resizeWindow(unsigned long width, unsigned long height)
{
	if (level == 0)
		return;

	level->resizeCamera(width, height);
};
void MainWindow::mouseMove(float dx, float dy)
{
	if (level == 0)
		return;

	level->rotateCamera(dx, dy);
};
void MainWindow::mouseWheel(float value)
{
	if (level == 0)
		return;

	level->moveCamera(value);
};
void MainWindow::AppendLevel(OgreStudioObjectType type, void *data)
{
	const char* names[] = { "light%0", "mesh%0" };
	QTreeWidgetItem* root = root_level_items.find(type).value();
	QTreeWidgetItem* child = OGRE_NEW QTreeWidgetItem();
	QString _name(names[type]);
	_name = _name.arg(root->childCount());
	child->setText(0, _name);
	child->setData(0, Qt::UserRole, QVariant::fromValue(data));
	root->addChild(child);
};
void MainWindow::CreateNode(Ogre::SceneNode* node, OgreStudioObjectType type)
{
	AppendLevel(type, node);

	physic->AppendObject(node);
};
void MainWindow::ReceiveMessage(int type, const std::string& message)
{
	QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(message));
	ui->messageList->addItem(item);
};