#pragma once

#include <QWidget>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include "ui_AddChannelDlg.h"

class AddChannelDlg : public QWidget
{
	Q_OBJECT

public:
	AddChannelDlg(QWidget *parent = nullptr);
	~AddChannelDlg();

signals:
	void sigAddChannel(const QString& channelNum);

private:
	Ui::AddChannelDlgClass ui;
};
