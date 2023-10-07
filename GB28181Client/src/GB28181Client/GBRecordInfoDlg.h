#pragma once

#include <QWidget>
#include <QPushButton>
#include "ui_GBRecordInfoDlg.h"
#include "MySipInfo.h"

class GBRecordInfoDlg : public QWidget
{
	Q_OBJECT

public:
	GBRecordInfoDlg(QWidget *parent = Q_NULLPTR);
	~GBRecordInfoDlg();

	void AddRecordInfo(const CMyRecordInfo& recordInfo);

private:
	void InitUI();
	void InitAction();

signals:
	void sigRecordInfo(const QString& gbid, const QString& startTime, const QString& endTime);

private:
	Ui::GBRecordInfoDlg ui;
};
