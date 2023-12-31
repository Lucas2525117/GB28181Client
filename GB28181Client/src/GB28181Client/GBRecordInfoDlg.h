#pragma once

#include <QWidget>
#include <QPushButton>
#include <QAbstractItemView>
#include <qmessagebox.h>
#include "ui_GBRecordInfoDlg.h"
#include "MySipInfo.h"

class GBRecordInfoDlg : public QWidget
{
	Q_OBJECT

public:
	GBRecordInfoDlg(QWidget *parent = Q_NULLPTR);
	~GBRecordInfoDlg();

private:
	void InitUI();
	void InitAction();

signals:
	void sigRecordInfo(const QString& gbid, const QString& startTime, const QString& endTime);

private:
	Ui::GBRecordInfoDlg ui;

	int m_row = 0;
};
