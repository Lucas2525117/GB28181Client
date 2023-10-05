#pragma once

#include <QWidget>
#include <qpushbutton.h>
#include <QRegExpValidator>
#include <qmessagebox.h>
#include "ui_AddDeviceDlg.h"
#include "public.h"

class AddDeviceDlg : public QWidget
{
	Q_OBJECT

public:
	AddDeviceDlg(QWidget *parent = nullptr);
	~AddDeviceDlg();

signals:
	void sigAddDevice(const QString& deviceIp);

private:
	Ui::AddDeviceDlgClass ui;
};
