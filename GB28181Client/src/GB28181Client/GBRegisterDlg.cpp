#include "GBRegisterDlg.h"

GBRegisterDlg::GBRegisterDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	InitUi();
	InitAction();
}

GBRegisterDlg::~GBRegisterDlg()
{
}

void GBRegisterDlg::SetRegisterResult(const QString& result)
{
	ui.le_registerResult->setText(result);
}

void GBRegisterDlg::InitUi()
{
	QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
	ui.le_localIp->setValidator(new QRegExpValidator(rx, this));
	ui.le_localIp->setInputMask("000.000.000.000");
}

void GBRegisterDlg::InitAction()
{
	connect(ui.pb_serverConnect, &QPushButton::clicked, [=]() {
		std::string gbid = ui.le_gbid->text().toStdString();
		std::string ip = ui.le_localIp->text().toStdString();
		int sipport = ui.le_sipPort->text().toInt();
		int transType = ui.cbx_transMode->currentIndex();
		if (gbid.empty() || ip.empty() || sipport <= 0)
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("参数不合法,请重新输入"), QMessageBox::Ok);
			return;
		}

		emit sigSipConnect(gbid, ip, sipport, transType);
		});

	connect(ui.pb_serverDisConnect, &QPushButton::clicked, [=]() {
		emit sigSipDisConnect();
		});
	ui.pb_serverDisConnect->setVisible(false);
}
