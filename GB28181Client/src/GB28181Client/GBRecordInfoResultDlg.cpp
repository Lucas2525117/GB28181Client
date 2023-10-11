#include "GBRecordInfoResultDlg.h"
#include <qthread.h>
#include <qmenu.h>
#include <qaction.h>

static int DetectThread(void* param)
{
	GBRecordInfoResultDlg* p = (GBRecordInfoResultDlg*)param;
	if (p)
	{
		p->OnWorkThread();
	}
	return 0;
}

GBRecordInfoResultDlg::GBRecordInfoResultDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	Init();
	InitUI();
}

GBRecordInfoResultDlg::~GBRecordInfoResultDlg()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();

	for (auto it : m_mapRecordInfo)
	{
		if (it.second)
		{
			delete it.second;
			it.second = nullptr;
		}
	}
}

void GBRecordInfoResultDlg::Init()
{
	m_thread = std::thread(DetectThread, this);
}

void GBRecordInfoResultDlg::InitUI()
{
	ui.tableWidget->clear();
	ui.tableWidget->setRowCount(500);

	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->setColumnCount(7);

	QStringList headers;
	headers << QStringLiteral("设备ID") << QStringLiteral("设备名") << QStringLiteral("文件路径")
		<< QStringLiteral("地址") << QStringLiteral("开始时间") << QStringLiteral("结束时间")
		<< QStringLiteral("文件大小");
	ui.tableWidget->setHorizontalHeaderLabels(headers);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	for(int i = 0; i < 6; ++i)
		ui.tableWidget->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Interactive);

	ui.tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	m_tabMenu = new QMenu(ui.tableWidget);
	QAction* action = new QAction(QString::fromLocal8Bit("录像回放"), this);
	m_tabMenu->addAction(action);
	connect(ui.tableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotMenuShow(QPoint)));
	connect(action, SIGNAL(triggered(bool)), this, SLOT(slotStartPlayBack()));
}

void GBRecordInfoResultDlg::Add_(CMyRecordInfo* recordInfo)
{
}

void GBRecordInfoResultDlg::slotMenuShow(QPoint point)
{
	if(m_tabMenu)
		m_tabMenu->exec(QCursor::pos());
}

void GBRecordInfoResultDlg::slotStartPlayBack()
{
	QStringList tableDataList;
	QList<QTableWidgetItem*> items = ui.tableWidget->selectedItems(); 
	int count = items.count();  // 列数
	for (int i = 0; i < count; i++)
	{
		int row = ui.tableWidget->row(items.at(i));    //获取选中的行
		QTableWidgetItem* item = ui.tableWidget->item(row, i);    //获取（row,i）处的单元格 
		tableDataList.append(item->text()); //将单元格的文本存入到列表中
	}

	if (0 == tableDataList.size())
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("未选中数据"), QMessageBox::Ok);
		return;
	}

	std::string deviceID = tableDataList[0].toStdString();
	time_t sTime = StringToDatetime(tableDataList[4].toStdString().c_str())*1000;
	time_t eTime = StringToDatetime(tableDataList[5].toStdString().c_str())*1000;
}

void GBRecordInfoResultDlg::AddRecordInfo(const QString& gbid, CMyRecordInfo* recordInfo)
{
	/*if (nullptr == recordInfo)
		return;

	RecursiveGuard mtx(m_recursive_mutex);
	m_setRecordInfo.insert(recordInfo);
	m_mapRecordInfo.insert(std::make_pair(gbid, recordInfo));*/

	ui.tableWidget->setItem(m_row, 0, new QTableWidgetItem(QString::fromLocal8Bit(recordInfo->deviceID.c_str())));
	ui.tableWidget->setItem(m_row, 1, new QTableWidgetItem(QString::fromLocal8Bit(recordInfo->deviceName.c_str())));
	ui.tableWidget->setItem(m_row, 2, new QTableWidgetItem(QString::fromLocal8Bit(recordInfo->filePath.c_str())));
	ui.tableWidget->setItem(m_row, 3, new QTableWidgetItem(QString::fromLocal8Bit(recordInfo->address.c_str())));

	QString sTime = recordInfo->startTime.c_str();
	if (sTime.contains("T"))
		sTime.replace('T', ' ');
	ui.tableWidget->setItem(m_row, 4, new QTableWidgetItem(QString::fromLocal8Bit(sTime.toStdString().c_str())));

	QString eTime = recordInfo->endTime.c_str();
	if (eTime.contains("T"))
		eTime.replace('T', ' ');
	ui.tableWidget->setItem(m_row, 5, new QTableWidgetItem(QString::fromLocal8Bit(eTime.toStdString().c_str())));
	ui.tableWidget->setItem(m_row, 6, new QTableWidgetItem(QString::fromLocal8Bit(recordInfo->fileSize.c_str())));

	m_row++;
}

void GBRecordInfoResultDlg::OnWorkThread()
{
	while (m_running)
	{
		RecordInfoSet recordInfoSet;
		{
			RecursiveGuard mtx(m_recursive_mutex);
			if (m_setRecordInfo.empty())
			{
				QThread::sleep(2);
				continue;
			}

			recordInfoSet = m_setRecordInfo;
		}

		for (auto it : recordInfoSet)
		{
			Add_(it);
		}

		QThread::sleep(2);
	}
}
