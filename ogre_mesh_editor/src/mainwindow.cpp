#include"mainwindow.h"
#include"qogrewidget.h"
#include"ui_mainwindow.h"
#include<qtimer.h>
#include<qfiledialog.h>

#include<OgreMesh2.h>
#include<OgreSubMesh2.h>
#include<OgreMesh2Serializer.h>

#include<Hlms/Pbs/OgreHlmsPbsDatablock.h>

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

	if(filename.endsWith("mesh"))
		mesh = ui->ogrewidget->LoadMesh(filename);
	else
		ui->ogrewidget->LoadAndConvert(filename);

	UpdateProperty(mesh);
};
void MainWindow::on_actionImport_triggered()
{
	QString filter("Support formats (*.fbx *.stl)");
	QString filename = QFileDialog::getOpenFileName(this, "Import mesh", QDir::currentPath(), filter);
	if (filename.isEmpty())
		return;

	Ogre::String basename, ext, path;
	Ogre::StringUtil::splitFullFilename(filename.toStdString(), basename, ext, path);

	ProgressDialog pd(this);

	QImport* importer = new QImport(ui->ogrewidget, ui->ogrewidget->GetHlmsManager(), ui->ogrewidget->GetRenderSystem(), & pd, filename);
	QThreadPool::globalInstance()->start(importer);

	if (pd.exec() == QDialog::Accepted)
	{
		UpdateProperty(ui->ogrewidget->GetMesh());
	}

	/*mesh = ui->ogrewidget->Import(filename);

	Ogre::MeshSerializer mesh_serializer(0);
	mesh_serializer.exportMesh(mesh, "models/"+basename+".mesh");

	std::string matName = mesh->getSubMesh(0)->getMaterialName();
	Ogre::HlmsDatablock *material = ui->ogrewidget->GetMaterial(matName.c_str());
	ui->ogrewidget->GetHlmsManager()->saveMaterial(material, "models/"+matName+ ".material.json", 0, "");*/
};
void MainWindow::UpdateProperty(Ogre::Mesh* mesh)
{
	ui->materialTree->clear();

	for (unsigned int index = 0; index < mesh->getNumSubMeshes(); index++)
	{
		QTreeWidgetItem* root = new QTreeWidgetItem();
		root->setText(0, QString("submesh%1").arg(index));
		ui->materialTree->addTopLevelItem(root);

		Ogre::SubMesh *submesh = mesh->getSubMesh(index);
		Ogre::HlmsDatablock *material = ui->ogrewidget->GetMaterial(submesh->getMaterialName().c_str());
		//material->mType Ogre::HLMS_PBS;
		//Ogre::HlmsPbsDatablock
		if (material)
		{
			for (int i = Ogre::PbsTextureTypes::PBSM_DIFFUSE; i < Ogre::PbsTextureTypes::PBSM_ROUGHNESS; i++)
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
				}
				item->setText(0, name);
				Ogre::TextureGpu *tex = ((Ogre::HlmsPbsDatablock*)material)->getTexture(i);
				if(tex)
					item->setText(1, QString::fromStdString(tex->getNameStr()));
				root->addChild(item);
			}
		}
	}
};