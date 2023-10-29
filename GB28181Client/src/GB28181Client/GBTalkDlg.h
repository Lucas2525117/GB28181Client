#pragma once

#include <QWidget>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include "ui_GBTalkDlg.h"

class GBTalkDlg : public QWidget
{
	Q_OBJECT

public:
	GBTalkDlg(QWidget *parent = nullptr);
	~GBTalkDlg();

signals:
	void sigSTalk(const QString& gbid, const QString& deviceIP, const QString& gbPort, const QString& localIP, const QString& localRecvPort);
	void sigStopTalk();

private:
	Ui::GBTalkDlg ui;
};
