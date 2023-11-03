#include "GBCataLogDlg.h"

GBCataLogDlg::GBCataLogDlg(QWidget *parent)
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

		emit sigQueryCatalog(deviceID);
		});
}

GBCataLogDlg::~GBCataLogDlg()
{
}

void GBCataLogDlg::AddCatalogData(const CMyCatalogInfo& catalog)
{
	ui.le_deviceID->setText(catalog.DeviceID.c_str());
	ui.le_deviceName->setText(catalog.Name.c_str());
	ui.le_manufacturer->setText(catalog.Manufacturer.c_str());
	ui.le_Address->setText(catalog.Address.c_str());
	ui.le_model->setText(catalog.Model.c_str());
	ui.le_omner->setText(catalog.Owner.c_str());
	ui.le_Civilcode->setText(catalog.Civilcode.c_str());
	ui.le_Registerway->setText(catalog.Registerway.c_str());
	ui.le_Secrecy->setText(catalog.Secrecy.c_str());
	ui.le_PlatformAddr->setText(catalog.PlatformAddr.c_str());
	ui.le_PlatformPort->setText(QString::number(catalog.PlatformPort));
	ui.le_Status->setText(catalog.Status.c_str());

}