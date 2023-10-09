#pragma once

#include <QWidget>
#include <QPushButton>
#include <QAbstractItemView>
#include <qmessagebox.h>
#include "ui_GBRecordInfoResultDlg.h"
#include "MySipInfo.h"

class GBRecordInfoResultDlg : public QWidget
{
	Q_OBJECT

public:
	GBRecordInfoResultDlg(QWidget *parent = Q_NULLPTR);
	~GBRecordInfoResultDlg();

	void AddRecordInfo(const CMyRecordInfo& recordInfo);

private:
	void InitUI();

signals:
	void sigRecordInfo(const QString& gbid, const QString& startTime, const QString& endTime);

private:
	Ui::GBRecordInfoResultDlg ui;

	int m_row = 0;
};
