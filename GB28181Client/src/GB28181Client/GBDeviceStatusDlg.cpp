#include "GBDeviceStatusDlg.h"

GBDeviceStatusDlg::GBDeviceStatusDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pb_SearchDeviceStatus, &QPushButton::clicked, [=]() {
		emit sigQueryDeviceStatus();
		});
}

GBDeviceStatusDlg::~GBDeviceStatusDlg()
{
}

void GBDeviceStatusDlg::AddDeviceStatusData(const CMyDeviceStatus& devicestatus)
{
	ui.le_deviceID->setText(devicestatus.deviceID.c_str());
	ui.le_online->setText(devicestatus.online.c_str());
	ui.le_status->setText(devicestatus.status.c_str());
	ui.le_result->setText(devicestatus.result.c_str());
	ui.le_deviceTime->setText(devicestatus.deviceTime.c_str());
	ui.le_encode->setText(devicestatus.encode.c_str());
	ui.le_record->setText(devicestatus.record.c_str());
}

