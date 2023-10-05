#include "AddChannelDlg.h"

AddChannelDlg::AddChannelDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("���ͨ��"));

	connect(ui.pb_ok, &QPushButton::clicked, [=]() {
		QString channelNum = ui.le_channelNum->text();
		if (channelNum.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("ͨ���Ų���Ϊ��"), QMessageBox::Ok);
			return;
		}

		emit sigAddChannel(channelNum);
		hide();
		});

	connect(ui.pb_cancel, &QPushButton::clicked, [=]() {
		hide();
		});
}

AddChannelDlg::~AddChannelDlg()
{}
