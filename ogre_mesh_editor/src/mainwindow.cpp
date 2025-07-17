#include"mainwindow.h"
#include"qogrewidget.h"
#include"ui_mainwindow.h"
#include<qtimer.h>
#include<qfiledialog.h>

#include<OgreItem.h>
#include<OgreMesh2.h>
#include<OgreSubMesh2.h>
#include<OgreMesh2Serializer.h>

#include"Animation/OgreSkeletonInstance.h"

#include<Hlms/Pbs/OgreHlmsPbsDatablock.h>

MainWindow::MainWindow():ui(new Ui::MainWindow())
{
	ui->setupUi(this);

	ui->play->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
	QObject::connect(ui->play, SIGNAL(clicked()), this, SLOT(play_clicked()));

	QObject::connect(ui->animationTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(animation_currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));

	QObject::connect(ui->materialTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(material_itemDoubleClicked(QTreeWidgetItem*, int)));

	ui->animationGroup->setEnabled(false);

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
	QString filename = QFileDialog::getOpenFileName(this, "Load OGRE mesh", QDir::currentPath(), "OGRE mesh (*.mesh)");
	if (filename.isEmpty())
		return;
	
	mesh = ui->ogrewidget->LoadMesh(filename);
	if(mesh)
		UpdateProperty(mesh);
};
void MainWindow::on_actionImport_triggered()
{
	QString filter("Support formats (*.fbx *.stl *.obj *.3ds)");
	QString filename = QFileDialog::getOpenFileName(this, "Import mesh", QDir::currentPath(), filter);
	if (filename.isEmpty())
		return;

	Ogre::String basename, ext, path;
	Ogre::StringUtil::splitFullFilename(filename.toStdString(), basename, ext, path);

	ProgressDialog pd(this);

	ui->ogrewidget->SetDraw(false);

	QImport* importer = new QImport(ui->ogrewidget, ui->ogrewidget->GetHlmsManager(), ui->ogrewidget->GetRenderSystem(), & pd, filename);
	QThreadPool::globalInstance()->start(importer);

	if (pd.exec() == QDialog::Accepted)
	{
		UpdateProperty(ui->ogrewidget->GetMesh());
	}
	ui->ogrewidget->SetDraw(true);
};
void MainWindow::UpdateProperty(Ogre::Mesh* mesh)
{
	ui->propertyTree->clear();
	QTreeWidgetItem* rootProperty = new QTreeWidgetItem();
	rootProperty->setText(0, QString::fromStdString(mesh->getName()));
	ui->propertyTree->addTopLevelItem(rootProperty);

	ui->materialTree->clear();
	ui->animationTree->clear();

	Ogre::Vector3 size = mesh->getAabb().getSize();
	QString text("Size x:%1 y:%2 z:%3");
	text = text.arg(size.x).arg(size.y).arg(size.z);
	ui->obj_size->setText(text);

	for (unsigned int index = 0; index < mesh->getNumSubMeshes(); index++)
	{
		QTreeWidgetItem* sub = new QTreeWidgetItem();
		sub->setText(0, QString("submesh%1").arg(index));
		rootProperty->addChild(sub);

		Ogre::SubMesh *submesh = mesh->getSubMesh(index);
		Ogre::HlmsDatablock *material = ui->ogrewidget->GetMaterial(submesh->getMaterialName().c_str());
		//material->mType Ogre::HLMS_PBS;
		//Ogre::HlmsPbsDatablock
		if (material)
		{
			QList<QTreeWidgetItem*> items = ui->materialTree->findItems(QString::fromStdString(submesh->getMaterialName()), Qt::MatchExactly);
			if (items.isEmpty()==false)
				continue;

			QTreeWidgetItem* root = new QTreeWidgetItem();
			//root->setText(0, QString("submesh%1").arg(index));
			root->setText(0, QString::fromStdString(submesh->getMaterialName()));
			ui->materialTree->addTopLevelItem(root);

			root->setData(0, Qt::UserRole, QVariant::fromValue(material));
			for (int i = Ogre::PbsTextureTypes::PBSM_DIFFUSE; i < Ogre::PbsTextureTypes::PBSM_DETAIL_WEIGHT; i++)
			{
				QTreeWidgetItem* item = new QTreeWidgetItem();
				const char* name;
				switch (i)
				{
				case Ogre::PbsTextureTypes::PBSM_DIFFUSE:
					name = "diffuse_map";
					break;
				case Ogre::PbsTextureTypes::PBSM_NORMAL:
					name = "normal_map";
					break;
				case Ogre::PbsTextureTypes::PBSM_SPECULAR:
					name = "specular_map";
					break;
				case Ogre::PbsTextureTypes::PBSM_ROUGHNESS:
					name = "roughness_map";
					break;
				}
				item->setText(0, name);
				item->setData(0, Qt::UserRole, MaterialType::MaterialType_Texture);
				Ogre::TextureGpu *tex = ((Ogre::HlmsPbsDatablock*)material)->getTexture(i);
				if(tex)
					item->setText(1, QString::fromStdString(tex->getNameStr()));
				item->setData(1, Qt::UserRole, i);
				root->addChild(item);
			}
			QTreeWidgetItem* item = new QTreeWidgetItem();
			item->setText(0, "diffuse");
			item->setData(0, Qt::UserRole, MaterialType::MaterialType_Color);
			root->addChild(item);
		}
	}

	Ogre::SceneNode * node = ui->ogrewidget->GetMeshNode();

	Ogre::Item* item = (Ogre::Item*)node->getAttachedObject(0);
	Ogre::SkeletonInstance* sk_inst = item->getSkeletonInstance();
	if (sk_inst)
	{
		const Ogre::SkeletonAnimationVec anims = sk_inst->getAnimations();
		ui->animationGroup->setDisabled(anims.empty());
		for (Ogre::SkeletonAnimationVec::const_iterator it = anims.begin(); it != anims.end(); it++)
		{
			char str[32];
			it->getName().getFriendlyText(str, 32);
			QTreeWidgetItem* root = new QTreeWidgetItem();
			root->setText(0, str);
			ui->animationTree->addTopLevelItem(root);
		}
	}
	else
	{
		ui->animationGroup->setEnabled(false);
	}
};
void MainWindow::writeLog(int type, QString message)
{
	QListWidgetItem* item = new QListWidgetItem(message);
	ui->logList->addItem(item);
};
void MainWindow::play_clicked()
{
	ui->ogrewidget->PlayAnimation(ui->animationTree->currentItem()->text(0).toLocal8Bit().data(), true);
	ui->play->setIcon(QIcon(":/icon/stop-button.png"));
};
void MainWindow::animation_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
	unsigned long ticks = ui->ogrewidget->GetAnimationTicks(current->text(0).toLocal8Bit().data());
	ui->animationSlider->setMaximum(ticks);
};
void MainWindow::material_itemDoubleClicked(QTreeWidgetItem* item, int column)
{
	if (item->data(0, Qt::UserRole).value<MaterialType>() == MaterialType::MaterialType_Texture)
	{
		QString filename = QFileDialog::getOpenFileName(this);
		if (filename.isEmpty())
			return;

		Ogre::String basename, ext, path;
		Ogre::StringUtil::splitFullFilename(filename.toStdString(), basename, ext, path);

		Ogre::PbsTextureTypes type = item->data(column, Qt::UserRole).value<Ogre::PbsTextureTypes>();

		Ogre::HlmsDatablock* material = item->parent()->data(0, Qt::UserRole).value<Ogre::HlmsDatablock*>();
		((Ogre::HlmsPbsDatablock*)material)->setTexture(type, basename + "." + ext);
		item->setText(column, QString::fromStdString(basename + "." + ext));
	}
};
void MainWindow::on_actionSave_triggered()
{
	ui->ogrewidget->Save();
};