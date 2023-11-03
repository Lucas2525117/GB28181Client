#pragma once

#include <QWidget>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include "ui_GBVoiceBroadcastDlg.h"

class GBVoiceBroadcastDlg : public QWidget
{
	Q_OBJECT

public:
	GBVoiceBroadcastDlg(QWidget *parent = nullptr);
	~GBVoiceBroadcastDlg();

	void SetBroadcastResult(const QString& result);

signals:
	void sigVoiceBroadcast(const QString& sourceID, const QString& targetID);

private:
	Ui::GBVoiceBroadcastDlgClass ui;
};
