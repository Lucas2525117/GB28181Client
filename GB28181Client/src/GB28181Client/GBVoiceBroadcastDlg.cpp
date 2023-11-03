#include "GBVoiceBroadcastDlg.h"

GBVoiceBroadcastDlg::GBVoiceBroadcastDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pb_startVoiceBroadcast, &QPushButton::clicked, [=]() {
		QString sourceid = ui.le_sourceID->text();
		QString targetid = ui.le_targetID->text();
		if (sourceid.isEmpty() || targetid.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("参数不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigVoiceBroadcast(sourceid, targetid);
		});
}

GBVoiceBroadcastDlg::~GBVoiceBroadcastDlg()
{
}

void GBVoiceBroadcastDlg::SetBroadcastResult(const QString& result)
{
	ui.lb_result->setText(result);
}
