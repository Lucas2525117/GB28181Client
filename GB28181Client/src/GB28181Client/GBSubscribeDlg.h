#pragma once

#include <QWidget>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include "MySipHeader.h"
#include "ui_GBSubscribeDlg.h"

class GBSubscribeDlg : public QWidget
{
	Q_OBJECT

public:
	GBSubscribeDlg(QWidget *parent = nullptr);
	~GBSubscribeDlg();

signals:
	void sigSubscribe(const QString& gbid, const QString& ipp, const QString& startTime, const QString& endTime, int subType, int expires);

private:
	Ui::GBSubscribeDlg ui;
};
