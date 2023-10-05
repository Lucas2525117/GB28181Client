#pragma once

#include <QWidget>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include "ui_AddOrgDlg.h"

class AddOrgDlg : public QWidget
{
	Q_OBJECT

public:
	AddOrgDlg(QWidget *parent = nullptr);
	~AddOrgDlg();

signals:
	void sigOrgName(const QString& orgName);

private:
	Ui::AddOrgDlgClass ui;
};
