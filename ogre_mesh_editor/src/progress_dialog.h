#ifndef PROGRESS_DIALOG_FILE
#define PROGRESS_DIALOG_FILE

#include<qdialog.h>

namespace Ui {
	class ProgressDialog;
};
class ProgressDialog :public QDialog
{
	Q_OBJECT
public:
	ProgressDialog(QWidget* parent = 0);
	~ProgressDialog();

	bool IsClose()const { return is_close; }
public slots:
	void on_cancel_clicked();

	void SetMessasge(QString text);
	void SetProgressMax(unsigned long val);
	void IncreaseProgress();
private:
	void closeEvent(QCloseEvent* e);

	Ui::ProgressDialog* ui;
	bool is_close;
};
#endif