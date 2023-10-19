#pragma once

#include <QWidget>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include "ui_PTZControlDlg.h"
#include "MySipHeader.h"

class PTZControlDlg : public QWidget
{
	Q_OBJECT

public:
	PTZControlDlg(QWidget *parent = nullptr);
	~PTZControlDlg();

signals:
	void sigPTZCmd(const QString& gbid, int type, int paramValue);

private:
	Ui::PTZControlDlgClass ui;
};
