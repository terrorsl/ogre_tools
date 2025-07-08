#include"ui_progress.h"
#include"progress_dialog.h"

ProgressDialog::ProgressDialog(QWidget* parent):QDialog(parent), ui(new Ui::ProgressDialog()), is_stop(false)
{
	ui->setupUi(this);
	setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
	ui->progressBar->setValue(0);
};
ProgressDialog::~ProgressDialog()
{
	delete ui;
};
void ProgressDialog::on_cancel_clicked()
{
	is_stop = true;
};
bool ProgressDialog::IsStop()
{
	return is_stop;
};
void ProgressDialog::SetProgressMax(unsigned long val)
{
	ui->progressBar->setMaximum(val);
};
void ProgressDialog::UpdateProgress()
{
	ui->progressBar->setValue(ui->progressBar->value() + 1);
};