#include "AddOrgDlg.h"

AddOrgDlg::AddOrgDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("添加组织"));

	connect(ui.pb_ok, &QPushButton::clicked, [=]() {
		QString orgName = ui.le_orgName->text();
		if (orgName.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("组织名不能为空"), QMessageBox::Ok);
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
