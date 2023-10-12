#pragma once

#include <QWidget>
#include <QPushButton>
#include <QAbstractItemView>
#include <qmessagebox.h>
#include <thread>
#include <mutex>
#include <set>
#include "ui_GBRecordInfoResultDlg.h"
#include "PlayBackDlg.h"
#include "MySipInfo.h"
#include "public.h"

class GBRecordInfoResultDlg : public QWidget
{
	Q_OBJECT

public:
	GBRecordInfoResultDlg(QWidget *parent = Q_NULLPTR);
	~GBRecordInfoResultDlg();

	void AddRecordInfo(const QString& gbid, CMyRecordInfo* recordInfo);

	void OnWorkThread();

private:
	void Init();
	void InitUI();

private:
	void Add_(CMyRecordInfo* recordInfo);

private slots:
	void slotMenuShow(QPoint point);
	void slotStartPlayBack();

signals:
	void sigRecordInfo(const QString& gbid, const QString& startTime, const QString& endTime);

private:
	Ui::GBRecordInfoResultDlg ui;
	QMenu* m_tabMenu = nullptr;
	PlayBackDlg* m_playBackDlg = nullptr;

	int m_row = 0;

	bool m_running = true;
	std::thread m_thread;

	typedef std::lock_guard<std::recursive_mutex> RecursiveGuard;
	std::recursive_mutex m_recursive_mutex;

	typedef std::set<CMyRecordInfo*> RecordInfoSet;
	typedef std::multimap<QString, CMyRecordInfo*> RecordInfoMap;
	RecordInfoSet m_setRecordInfo;
	RecordInfoMap m_mapRecordInfo;
};
