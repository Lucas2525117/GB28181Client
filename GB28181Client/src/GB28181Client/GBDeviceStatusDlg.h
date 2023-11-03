#pragma once

#include <QWidget>
#include <qmessagebox.h>
#include "ui_GBDeviceStatusDlg.h"
#include "MySipInfo.h"

class GBDeviceStatusDlg : public QWidget
{
	Q_OBJECT

public:
	GBDeviceStatusDlg(QWidget *parent = nullptr);
	~GBDeviceStatusDlg();

	void AddDeviceStatusData(const CMyDeviceStatus& devicestatus);

signals:
	void sigQueryDeviceStatus(const QString& deviceID);

private:
	Ui::GBDeviceStatusDlg ui;
};
