#include "GBRecordInfoResultDlg.h"

GBRecordInfoResultDlg::GBRecordInfoResultDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	InitUI();
}

GBRecordInfoResultDlg::~GBRecordInfoResultDlg()
{
}

void GBRecordInfoResultDlg::InitUI()
{
	ui.tableWidget->clear();
	ui.tableWidget->setRowCount(0);

	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->setColumnCount(8);

	QStringList headers;
	headers << QStringLiteral("�豸ID") << QStringLiteral("�豸��") << QStringLiteral("�ļ�·��")
		<< QStringLiteral("��ַ") << QStringLiteral("��ʼʱ��") << QStringLiteral("����ʱ��")
		<< QStringLiteral("�ļ���С") << QStringLiteral("¼�񲥷�");
	ui.tableWidget->setHorizontalHeaderLabels(headers);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	for(int i = 0; i < 7; ++i)
		ui.tableWidget->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Interactive);
}

void GBRecordInfoResultDlg::AddRecordInfo(const CMyRecordInfo& recordInfo)
{
	ui.tableWidget->setItem(m_row, 0, new QTableWidgetItem(QString::fromLocal8Bit(recordInfo.deviceID.c_str())));
	ui.tableWidget->setItem(m_row, 1, new QTableWidgetItem(QString::fromLocal8Bit(recordInfo.deviceName.c_str())));
	ui.tableWidget->setItem(m_row, 2, new QTableWidgetItem(QString::fromLocal8Bit(recordInfo.filePath.c_str())));
	ui.tableWidget->setItem(m_row, 3, new QTableWidgetItem(QString::fromLocal8Bit(recordInfo.address.c_str())));
	ui.tableWidget->setItem(m_row, 4, new QTableWidgetItem(QString::fromLocal8Bit(recordInfo.startTime.c_str())));
	ui.tableWidget->setItem(m_row, 5, new QTableWidgetItem(QString::fromLocal8Bit(recordInfo.endTime.c_str())));
	ui.tableWidget->setItem(m_row, 6, new QTableWidgetItem(QString::fromLocal8Bit(recordInfo.fileSize.c_str())));
	QPushButton* btnPlay = new QPushButton();
	connect(btnPlay, &QPushButton::clicked, [=] {
		
		});
	btnPlay->setText(QString::fromLocal8Bit("¼�񲥷�"));
	ui.tableWidget->setCellWidget(m_row, 7, btnPlay);

	m_row++;
}
