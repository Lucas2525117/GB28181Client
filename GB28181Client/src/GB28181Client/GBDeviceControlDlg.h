#pragma once

#include <QWidget>
#include <qpushbutton.h>
#include "ui_GBDeviceControlDlg.h"

class GBDeviceControlDlg : public QWidget
{
	Q_OBJECT

public:
	GBDeviceControlDlg(QWidget *parent = nullptr);
	~GBDeviceControlDlg();

signals:
	void sigTeleReboot();

private:
	Ui::GBDeviceControlDlg ui;
};
