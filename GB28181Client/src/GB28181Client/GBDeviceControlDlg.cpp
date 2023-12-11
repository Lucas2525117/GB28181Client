#include "GBDeviceControlDlg.h"

GBDeviceControlDlg::GBDeviceControlDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pb_reboot, &QPushButton::clicked, [=]() {
		emit sigTeleReboot();
		});
}

GBDeviceControlDlg::~GBDeviceControlDlg()
{
}
