#include "AddOrgDlg.h"

AddOrgDlg::AddOrgDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("�����֯"));

	connect(ui.pb_ok, &QPushButton::clicked, [=]() {
		QString orgName = ui.le_orgName->text();
		if (orgName.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("��֯������Ϊ��"), QMessageBox::Ok);
			return;
		}

		emit sigOrgName(orgName);
		hide();
		});

	connect(ui.pb_cancel, &QPushButton::clicked, [=]() {
		hide();
		});
}

AddOrgDlg::~AddOrgDlg()
{}
