#pragma once

#include <QWidget>
#include <qmessagebox.h>
#include "ui_GBVideoPlayDlg.h"
#include "MySipInfo.h"
#include "MySipHeader.h"

class GBVideoPlayDlg : public QWidget
{
	Q_OBJECT

public:
	GBVideoPlayDlg(QWidget *parent = nullptr);
	~GBVideoPlayDlg();

signals:
	void sigStartVideoPlay(const QString& gbid, const QString& deviceIP, const QString& gbPort, const QString& localIP, const QString& localRecvPort);
	void sigStopVideoPlay();

private:
	Ui::GBVideoPlayDlg ui;
};
