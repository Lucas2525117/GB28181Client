#include "GBDeviceInfoDlg.h"

GBDeviceInfoDlg::GBDeviceInfoDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pb_search, &QPushButton::clicked, [=]() {
		QString deviceID = ui.le_deviceID->text();
		if (deviceID.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请输入正确的设备ID"), QMessageBox::Ok);
			return;
		}

		emit sigQueryDeviceInfo(deviceID);
		});


}

GBDeviceInfoDlg::~GBDeviceInfoDlg()
{
}

void GBDeviceInfoDlg::AddDeviceInfoData(const CMyDeviceInfo& deviceinfo)
{
	ui.le_deviceID->setText(deviceinfo.deviceID.c_str());
	ui.le_deviceName->setText(deviceinfo.deviceName.c_str());
	ui.le_result->setText(deviceinfo.result.c_str());
	ui.le_deviceType->setText(deviceinfo.deviceType.c_str());
	ui.le_manufacturer->setText(deviceinfo.manufacturer.c_str());
	ui.le_model->setText(deviceinfo.model.c_str());
	ui.le_firmware->setText(deviceinfo.firmware.c_str());
	ui.le_maxCamera->setText(deviceinfo.maxCamera.c_str());
	ui.le_maxAlarm->setText(deviceinfo.maxAlarm.c_str());
	ui.le_channel->setText(deviceinfo.channel.c_str());
	ui.le_port->setText(deviceinfo.port.c_str());
}