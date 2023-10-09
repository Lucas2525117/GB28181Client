#include "GBRecordInfoDlg.h"

GBRecordInfoDlg::GBRecordInfoDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	InitUI();
	InitAction();
}

GBRecordInfoDlg::~GBRecordInfoDlg()
{
}

void GBRecordInfoDlg::InitUI()
{
}

void GBRecordInfoDlg::InitAction()
{
	connect(ui.pb_query, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString startTime = ui.dte_starttime->text();
		QString endTime = ui.dte_endtime->text();
		if (gbid.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("��������ȷ�Ĺ���id"), QMessageBox::Ok);
			return;
		}

		emit sigRecordInfo(gbid, startTime, endTime);
		});
}