#pragma once

#include <QWidget>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include "ui_GBDeviceInfoDlg.h"
#include "MySipInfo.h"

class GBDeviceInfoDlg : public QWidget
{
	Q_OBJECT

public:
	GBDeviceInfoDlg(QWidget *parent = nullptr);
	~GBDeviceInfoDlg();

	void AddDeviceInfoData(const CMyDeviceInfo& deviceinfo);

signals:
	void sigQueryDeviceInfo(const QString& deviceID);

private:
	Ui::GBDeviceInfoDlg ui;
};
