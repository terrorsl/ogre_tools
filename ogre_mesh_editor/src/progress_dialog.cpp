#include"ui_progress.h"
#include"progress_dialog.h"

ProgressDialog::ProgressDialog(QWidget* parent) :QDialog(parent), ui(new Ui::ProgressDialog()), is_close(false)
{
	ui->setupUi(this);
	ui->progressBar->setValue(0);
};
ProgressDialog::~ProgressDialog()
{
	delete ui;
};
void ProgressDialog::on_cancel_clicked()
{
	is_close = true;
};
void ProgressDialog::closeEvent(QCloseEvent* e)
{
	on_cancel_clicked();
};
void ProgressDialog::SetMessasge(QString text)
{
	ui->message->setText(text);
}
void ProgressDialog::SetProgressMax(unsigned long val)
{
	ui->progressBar->setMaximum(val);
};
void ProgressDialog::IncreaseProgress()
{
	ui->progressBar->setValue(ui->progressBar->value() + 1);
};