#include "AddDeviceDlg.h"

AddDeviceDlg::AddDeviceDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("添加设备"));
	QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
	ui.le_deviceIp->setValidator(new QRegExpValidator(rx, this));
	ui.le_deviceIp->setInputMask("000.000.000.000");

	connect(ui.pb_ok, &QPushButton::clicked, [=]() {
		QString deviceIp = ui.le_deviceIp->text();
		if (deviceIp.isEmpty() || !ipAddrIsOK(deviceIp))
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("设备ip不合法"), QMessageBox::Ok);
			return;
		}

		emit sigAddDevice(deviceIp);
		hide();
		});

	connect(ui.pb_cancel, &QPushButton::clicked, [=]() {
		hide();
		});
}

AddDeviceDlg::~AddDeviceDlg()
{}
