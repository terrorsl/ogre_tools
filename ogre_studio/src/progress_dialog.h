#ifndef PROGRESS_DIALOG_FILE
#define PROGRESS_DIALOG_FILE

#include<qdialog.h>

namespace Ui {
	class ProgressDialog;
};
class ProgressDialog: public QDialog
{
	Q_OBJECT
public:
	ProgressDialog(QWidget* parent = 0);
	~ProgressDialog();

	bool IsStop();
public slots:
	void on_cancel_clicked();

	void SetProgressMax(unsigned long val);
	void UpdateProgress();
private:
	bool is_stop;
	Ui::ProgressDialog* ui;
};
#endif