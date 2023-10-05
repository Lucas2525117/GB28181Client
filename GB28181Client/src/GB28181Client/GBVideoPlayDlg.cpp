#include "GBVideoPlayDlg.h"

GBVideoPlayDlg::GBVideoPlayDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pb_start, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString deviceIP = ui.le_deviceIP->text();
		QString gbPort = ui.le_gbPort->text();
		QString localIP = ui.le_localip->text();
		QString localRecvPort = ui.le_recvPort->text();
		if (gbid.isEmpty() || deviceIP.isEmpty() || gbPort.isEmpty() || localIP.isEmpty() || localRecvPort.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("参数不合法,请重新输入"), QMessageBox::Ok);
			return;
		}

		emit sigStartVideoPlay(gbid, deviceIP, gbPort, localIP, localRecvPort);
		});

	connect(ui.pb_stop, &QPushButton::clicked, [=]() {
		emit sigStopVideoPlay();
		});
}

GBVideoPlayDlg::~GBVideoPlayDlg()
{
} 