#include "GBRecordInfoDlg.h"

GBRecordInfoDlg::GBRecordInfoDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pb_query, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString startTime = ui.dte_starttime->text();
		QString endTime = ui.dte_endtime->text();

		emit sigRecordInfo(gbid, startTime, endTime);
		});
}

GBRecordInfoDlg::~GBRecordInfoDlg()
{
}
