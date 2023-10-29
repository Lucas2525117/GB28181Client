#include "GBTalkDlg.h"

GBTalkDlg::GBTalkDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pb_startTalk, &QPushButton::clicked, [=]() {
		QString deviceID = ui.le_deviceID->text();
		QString deviceIP = ui.le_deviceIP->text();
		QString gbPort = ui.le_gbPort->text();
		QString localIP = ui.le_localIP->text();
		QString localRecvPort = ui.le_localPort->text();
		if (deviceID.isEmpty() || deviceIP.isEmpty() || gbPort.isEmpty() || localIP.isEmpty() || localRecvPort.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("参数不合法,请重新输入"), QMessageBox::Ok);
			return;
		}

		emit sigSTalk(deviceID, deviceIP, gbPort, localIP, localRecvPort);
		});

	connect(ui.pb_stopTalk, &QPushButton::clicked, [=]() {
		emit sigStopTalk();
		});
}

GBTalkDlg::~GBTalkDlg()
{}
