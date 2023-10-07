﻿#include "GB28181Client.h"
#include <thread>

static void MyGBMsgCB(int type, void* user, void* data)
{
	assert(user);
	GB28181Client* cli = (GB28181Client*)user;
	cli->HandleGBMsgCB(type, data);
}

static void MyGBDataCB(int avtype, void* data, int dataLen, void* user)
{
	assert(user);
	GB28181Client* cli = (GB28181Client*)user;
	cli->HandleGBDataCB(avtype, data, dataLen);
}

int GBClientDataWorkerThread(void* param, const std::string& localip, const std::string& localport)
{
	assert(param);
	GB28181Client* client = (GB28181Client*)param;
	client->GBClientDataWorker(localip, localport);
	return 0;
}

GB28181Client::GB28181Client(QWidget *parent)
    : QMainWindow(parent)
{
    InitUi();
	InitAction();
}

GB28181Client::~GB28181Client()
{
}

void GB28181Client::InitUi()
{
    ui.setupUi(this);
	resize(1200, 800);
	setWindowTitle(QString::fromLocal8Bit("GB28181客户端"));

	// 注册与注销
	m_GBRegisterDlg = new(std::nothrow) GBRegisterDlg();
	if (nullptr != m_GBRegisterDlg)
	{
		connect(m_GBRegisterDlg, &GBRegisterDlg::sigSipConnect, this, &GB28181Client::Start);
		connect(m_GBRegisterDlg, &GBRegisterDlg::sigSipDisConnect, this, &GB28181Client::Stop);
	}

	// 设备目录
	m_GBCataLogDlg = new(std::nothrow) GBCataLogDlg();
	if (m_GBCataLogDlg)
	{
		connect(m_GBCataLogDlg, &GBCataLogDlg::sigQueryCatalog, [=]() {
			if (!m_gbid.empty())
			{
				if (0 != GB_QueryNetDeviceInfo(Type_RecvCatalog, m_gbid))
				{
					QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("查询设备目录失败"), QMessageBox::Ok);
				}
			}
			});
	}
	// 设备信息
	m_GBDeviceInfoDlg = new(std::nothrow) GBDeviceInfoDlg();
	if (m_GBDeviceInfoDlg)
	{
		connect(m_GBDeviceInfoDlg, &GBDeviceInfoDlg::sigQueryDeviceInfo, [=]() {
			if (!m_gbid.empty())
			{
				if (0 != GB_QueryNetDeviceInfo(Type_RecvDeviceInfo, m_gbid))
				{
					QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("查询设备信息失败"), QMessageBox::Ok);
				}
			}
			});
	}
	// 设备状态
	m_GBDeviceStatusDlg = new(std::nothrow) GBDeviceStatusDlg();
	if (m_GBDeviceStatusDlg)
	{
		connect(m_GBDeviceStatusDlg, &GBDeviceStatusDlg::sigQueryDeviceStatus, [=]() {
			if (!m_gbid.empty())
			{
				if (0 != GB_QueryNetDeviceInfo(Type_RecvDeviceStatus, m_gbid))
				{
					QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("查询设备状态失败"), QMessageBox::Ok);
				}
			}
			});
	}

	// 视频点播
	m_GBVideoPlayDlg = new(std::nothrow) GBVideoPlayDlg();
	if (m_GBVideoPlayDlg)
	{
		connect(m_GBVideoPlayDlg, &GBVideoPlayDlg::sigStartVideoPlay, this, &GB28181Client::slotStartVideoPlay);
		connect(m_GBVideoPlayDlg, &GBVideoPlayDlg::sigStopVideoPlay, this, &GB28181Client::slotStopVideoPlay);
	}

	// 视音频文件检索
	m_GBRecordInfoDlg = new(std::nothrow) GBRecordInfoDlg();
	if (m_GBRecordInfoDlg)
	{
		connect(m_GBRecordInfoDlg, &GBRecordInfoDlg::sigRecordInfo, this, &GB28181Client::slotQueryRecordInfo);
	}

	m_tabWidget = new(std::nothrow) QTabWidget();
	if (m_tabWidget)
	{
		m_tabWidget->setTabsClosable(false);
		connect(m_tabWidget, &QTabWidget::tabBarClicked, this, [=](int index) {
			if (5 == index)
				centralWidget()->hide();
			else
				centralWidget()->show();

			m_tabWidget->setCurrentIndex(index);
			});
		m_tabWidget->addTab(m_GBRegisterDlg, QString::fromLocal8Bit("注册与注销"));
		m_tabWidget->addTab(m_GBCataLogDlg, QString::fromLocal8Bit("设备目录"));
		m_tabWidget->addTab(m_GBDeviceInfoDlg, QString::fromLocal8Bit("设备信息"));
		m_tabWidget->addTab(m_GBDeviceStatusDlg, QString::fromLocal8Bit("设备状态"));
		m_tabWidget->addTab(m_GBVideoPlayDlg, QString::fromLocal8Bit("视频点播"));
		m_tabWidget->addTab(m_GBRecordInfoDlg, QString::fromLocal8Bit("视音频文件检索"));
	}

	// 添加组织界面
	m_addOrgDlg = new(std::nothrow) AddOrgDlg();
	if (m_addOrgDlg)
	{
		connect(m_addOrgDlg, SIGNAL(sigOrgName(const QString&)), this, SLOT(slotAddOrg(const QString&)));
	}
	// 添加设备界面
	m_addDeviceDlg = new(std::nothrow) AddDeviceDlg();
	if (m_addDeviceDlg)
	{
		connect(m_addDeviceDlg, SIGNAL(sigAddDevice(const QString&)), this, SLOT(slotAddDevice(const QString&)));
	}
	m_addChannelDlg = new(std::nothrow) AddChannelDlg();
	if (m_addChannelDlg)
	{
		connect(m_addChannelDlg, SIGNAL(sigAddChannel(const QString&)), this, SLOT(slotAddChannel(const QString&)));
	}

	// 视频展示区(待完善)
	m_playWidget = new(std::nothrow) PlayWidget();
	m_playWidget->Init();
	setCentralWidget(m_playWidget);

	// 设备/通道树展示栏
	m_treeWidget = new(std::nothrow) QTreeWidget();
	if (m_treeWidget)
	{
		m_treeWidget->header()->setVisible(false);
		m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);// 开启右键单击目录
		m_treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
		m_treeWidget->header()->setStretchLastSection(false);
		connect(m_treeWidget, &QTreeWidget::customContextMenuRequested, this, &GB28181Client::slotContextMenu);
		connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &GB28181Client::slotItemDoubleClick);
	}

	// 工具操作日志展示栏
	m_listWidget = new(std::nothrow) QListWidget();
	if (m_listWidget)
	{
		m_listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(m_listWidget, &QListWidget::customContextMenuRequested, this, [&](const QPoint&) {
			QMenu* popMenu = new QMenu();
			QAction* clearAll = new QAction(QString::fromLocal8Bit("清空"), popMenu);
			popMenu->addAction(clearAll);
			connect(clearAll, &QAction::triggered, this, [&]() {
				m_listWidget->clear();
				});
			popMenu->exec(QCursor::pos());
			});
		m_listWidget->hide();
	}

	// 设备/通道树区
	if (nullptr == m_projManagerView)
	{
		m_projManagerView = new QDockWidget(this);
		m_projManagerView->setFeatures(QDockWidget::DockWidgetMovable);
		m_projManagerView->setWindowTitle(QString::fromLocal8Bit("设备/通道树展示"));
		addDockWidget(Qt::LeftDockWidgetArea, m_projManagerView, Qt::Orientation::Vertical);
		m_projManagerView->setWidget(m_treeWidget);
		m_projManagerView->hide();
	}

	// 日志展示区
	if (nullptr == m_logView)
	{
		m_logView = new QDockWidget(this);
		m_logView->setFeatures(QDockWidget::DockWidgetMovable);
		m_logView->setWindowTitle(QString::fromLocal8Bit("日志输出"));
		addDockWidget(Qt::BottomDockWidgetArea, m_logView, Qt::Orientation::Vertical);
		m_logView->setWidget(m_listWidget);
		m_logView->hide();
	}

	// 操作区
	if (nullptr == m_operView)
	{
		m_operView = new(std::nothrow) QDockWidget(this);
		m_operView->setFeatures(QDockWidget::DockWidgetMovable);
		m_operView->setWindowTitle(QString::fromLocal8Bit("GB28181操作"));
		this->addDockWidget(Qt::TopDockWidgetArea, m_operView, Qt::Orientation::Vertical);
		m_operView->setWidget(m_tabWidget);
	}

	QTimer* timer = new QTimer();
	timer->start(5000);
	connect(timer, &QTimer::timeout, this, &GB28181Client::slotCatalogTimer);
}

void GB28181Client::InitAction()
{
	QMenuBar* menuBar = new QMenuBar();
	setMenuBar(menuBar);

	QMenu* menuConnect = new QMenu(QString::fromLocal8Bit("开始"), this);
	QMenu* menuHelp = new QMenu(QString::fromLocal8Bit("帮助"), this);
	menuBar->addMenu(menuConnect);
	menuBar->addMenu(menuHelp);

	QAction* actAddOrg = new QAction(QString::fromLocal8Bit("添加组织"), this);
	QAction* actClose = new QAction(QString::fromLocal8Bit("关闭"), this);
	menuConnect->addAction(actAddOrg);
	menuConnect->addAction(actClose);

	QAction* actVersion = new QAction(QString::fromLocal8Bit("版本信息"), this);
	menuHelp->addAction(actVersion);

	connect(actAddOrg, &QAction::triggered, [=]() {
		if (nullptr != m_addOrgDlg)
			m_addOrgDlg->show();
		});

	connect(actClose, &QAction::triggered, [=]() {
		exit(0);
		});

	connect(actVersion, &QAction::triggered, [=]() {
		
		});
}

QTreeWidgetItem* GB28181Client::SearchChild(QTreeWidgetItem* item, const QString& strMsg)
{
	for (int i = 0; i < item->childCount(); ++i)
	{
		QTreeWidgetItem* curChildItem = item->child(i);
		QString childMsg = curChildItem->toolTip(0);
		if (childMsg == strMsg)
		{
			return curChildItem;
		}
	}
	return nullptr;
}

void GB28181Client::AddLog(const QString& strText)
{
	if (nullptr != m_listWidget)
		m_listWidget->addItem(strText);
}

void GB28181Client::slotItemDoubleClick(QTreeWidgetItem* item, int index)
{
	if (nullptr == item)
		return;

	int type = item->type();
	if (TypeChannel == type)
	{
		QString channleNum = item->toolTip(0);

		std::string requestUrl = "sip:" + m_gbid + "@" + "100.18.141.85:5060";
		GB28181MediaContext mediaContext(requestUrl);
		mediaContext.SetRecvAddress("100.18.141.86");
		mediaContext.SetRecvPort(5060);
		m_token = GB_Invite(mediaContext);
	}
}

void GB28181Client::slotContextMenu(const QPoint& pos)
{
	QTreeWidgetItem* item = m_treeWidget->currentItem();
	if (!item)
		return;
	int type = item->type();
	QString value = item->toolTip(0);
	if (TypeOrg == type)
	{
		QAction* pAddDeviceAction = new QAction(QString::fromLocal8Bit("添加设备"), this);
		connect(pAddDeviceAction, &QAction::triggered, [=]() {
			if (nullptr != m_addDeviceDlg)
				m_addDeviceDlg->show();
			});
	
		QMenu* popMenu = new QMenu(this);      
		popMenu->addAction(pAddDeviceAction);
		popMenu->exec(QCursor::pos());
	}
	else if (TypeDevice == type)
	{
		QAction* pAddChannelAction = new QAction(QString::fromLocal8Bit("添加通道"), this);
		connect(pAddChannelAction, &QAction::triggered, [=]() {
			if (nullptr != m_addDeviceDlg)
				m_addChannelDlg->show();
			});

		QMenu* popMenu = new QMenu(this);      
		popMenu->addAction(pAddChannelAction);
		popMenu->exec(QCursor::pos());
	}
}

void GB28181Client::slotAddOrg(const QString& orgName)
{
	if (orgName.isEmpty())
		return;

	QTreeWidgetItem* colonyItem = new QTreeWidgetItem(m_treeWidget, TypeOrg);
	if (nullptr != colonyItem)
	{
		colonyItem->setText(0, orgName);
		colonyItem->setData(0, Qt::UserRole, orgName);
		colonyItem->setToolTip(0, orgName);
		colonyItem->setExpanded(true);
		m_treeWidget->addTopLevelItem(colonyItem);//添加顶层节点
	}
}

void GB28181Client::slotAddDevice(const QString& deviceIp)
{
	// 非ip格式过滤掉
	if (!ipAddrIsOK(deviceIp) || deviceIp.isEmpty())
		return;

	QTreeWidgetItem* orgItem = m_treeWidget->currentItem();
	if (nullptr == orgItem)
		return;

	QString orgName = orgItem->text(0);
	QList<QTreeWidgetItem*> orgItems = m_treeWidget->findItems(orgName, Qt::MatchFlag::MatchExactly, TypeOrg);
	if (orgItems.size() <= 0)
		return;

	QTreeWidgetItem* serverIpItem = SearchChild(orgItems.at(0), deviceIp);
	if (nullptr == serverIpItem)
	{
		QTreeWidgetItem* serverIpItem = new QTreeWidgetItem(orgItems.at(0), TypeDevice);
		serverIpItem->setText(0, deviceIp);
		serverIpItem->setData(0, Qt::UserRole, deviceIp);
		serverIpItem->setToolTip(0, deviceIp);
		serverIpItem->setExpanded(true);
		serverIpItem->addChild(orgItems.at(0));
	}
}

void GB28181Client::slotAddChannel(const QString& channelNum)
{
	if (channelNum.isEmpty())
		return;

	QTreeWidgetItem* deviceItem = m_treeWidget->currentItem();
	if (nullptr == deviceItem)
		return;

	QTreeWidgetItem* channelItem = SearchChild(deviceItem, channelNum);
	if (nullptr == channelItem && ipAddrIsOK(deviceItem->toolTip(0)))
	{
		QTreeWidgetItem* channelItem = new QTreeWidgetItem(deviceItem, TypeChannel);
		channelItem->setText(0, QString::fromLocal8Bit("通道%1").arg(channelNum));
		channelItem->setData(0, Qt::UserRole, channelNum);
		channelItem->setToolTip(0, channelNum);
		channelItem->setExpanded(false);
		channelItem->addChild(deviceItem);
	}
}

void GB28181Client::Start(const std::string& gbid, const std::string& ip, int sipport)
{
	// 初始化协议栈并启动
	const auto localcontact = "SIP:" + gbid +"@" + ip + ":" + std::to_string(sipport);
	if (!GB_Init(localcontact, 3))
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("SIP初始化失败"), QMessageBox::Ok);
		return;
	}

	m_gbid = gbid;
	GB_RegisterHandler(Type_Register, MyGBMsgCB, this);
	GB_RegisterHandler(Type_KeepAlive, MyGBMsgCB, this);
	GB_RegisterHandler(Type_RecvCatalog, MyGBMsgCB, this);
	GB_RegisterHandler(Type_RecvRecordInfo, MyGBMsgCB, this);
	GB_RegisterHandler(Type_RecvDeviceInfo, MyGBMsgCB, this);
	GB_RegisterHandler(Type_RecvDeviceStatus, MyGBMsgCB, this);
}

void GB28181Client::Stop()
{

}

void GB28181Client::closeEvent(QCloseEvent* event)
{
	exit(0);
}

void GB28181Client::slotCatalogTimer()
{
	if (m_GBCataLogDlg)
	{
		m_GBCataLogDlg->AddCatalogData(m_catalog);
		slotAddDevice(m_catalog.PlatformAddr.c_str());
	}

	if (m_GBDeviceInfoDlg)
	{
		m_GBDeviceInfoDlg->AddDeviceInfoData(m_deviceinfo);
		slotAddChannel(m_deviceinfo.channel.c_str());
	}

	if (m_GBDeviceStatusDlg)
	{
		m_GBDeviceStatusDlg->AddDeviceStatusData(m_deviceStatus);
	}

	if (0 == m_registerCBMsg.compare("register ok"))
	{
		if (m_GBRegisterDlg)
			m_GBRegisterDlg->SetRegisterResult(QString::fromLocal8Bit("注册成功"));
	}
}

void GB28181Client::slotStartVideoPlay(const QString& gbid, const QString& deviceIP, const QString& gbPort, const QString& localIP, const QString& localRecvPort)
{
	if (m_receiver)
	{
		QMessageBox::information(this, QString::fromLocal8Bit("通知"), QString::fromLocal8Bit("当前正在点播"), QMessageBox::Ok);
		return;
	}

	std::string requestUrl = "sip:" + gbid.toStdString() + "@" + deviceIP.toStdString() + ":" + gbPort.toStdString();
	GB28181MediaContext mediaContext(requestUrl);
	mediaContext.SetRecvAddress(localIP.toStdString());
	mediaContext.SetRecvPort(localRecvPort.toInt());
	m_token = GB_Invite(mediaContext);
	if(m_token.empty())
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("开始视频点播失败"), QMessageBox::Ok);

	std::thread th(GBClientDataWorkerThread, this, localIP.toStdString(), localRecvPort.toStdString());
	th.detach();
}

void GB28181Client::slotStopVideoPlay()
{
	if (m_token.empty() || !m_receiver)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请先开始视频点播"), QMessageBox::Ok);
		return;
	}
	
	m_receiver->Stop();
	m_receiver->DeleteThis();
	m_receiver = nullptr;
	GB_Bye(m_token);
}

void GB28181Client::slotQueryRecordInfo(const QString& gbid, const QString& startTime, const QString& endTime)
{
	if (gbid.isEmpty() || startTime.isEmpty() || endTime.isEmpty())
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请输入正确的参数"), QMessageBox::Ok);
		return;
	}

	GB28181MediaContext mediaContext("");
	mediaContext.SetTime(startTime.toStdString(), endTime.toStdString());
	GB_QueryRecordInfo(gbid.toStdString(), mediaContext);
}

void GB28181Client::HandleGBMsgCB(int type, void* data)
{
	switch (type)
	{
	case Type_RecvCatalog:
		HandleCataLogData(data);
		break;
	case Type_Register:
		HandleRegisterData(data);
		break;
	case Type_RecvDeviceInfo:
		HandleDeviceInfoData(data);
		break;
	case Type_RecvDeviceStatus:
		HandleDeviceStatusData(data);
		break;
	case Type_RecvRecordInfo:
		HandleRecordInfoData(data);
		break;
	default:
		break;
	}
}

void GB28181Client::HandleGBDataCB(int avtype, void* data, int dataLen)
{
	if (m_playWidget)
		m_playWidget->AddData(CODEC_VIDEO_H264, data, dataLen);
}

void GB28181Client::GBClientDataWorker(const std::string& localip, const std::string& localport)
{
	std::string gburl = "gbudp://" + localip + ":" + localport;
	m_receiver = GB_CreateStreamReceiver(gburl.c_str(), MyGBDataCB, this);
	m_receiver->Start();
}

void GB28181Client::HandleCataLogData(void* data)
{
	if (!data)
		return;

	memcpy(&m_catalog, data, sizeof(CMyCatalogInfo));
}

void GB28181Client::HandleRegisterData(void* data)
{
	if (!data)
		return;

	m_registerCBMsg = (char*)data;
}

void GB28181Client::HandleDeviceInfoData(void* data)
{
	if (!data)
		return;

	memcpy(&m_deviceinfo, data, sizeof(CMyDeviceInfo));
}

void GB28181Client::HandleDeviceStatusData(void* data)
{
	if (!data)
		return;

	memcpy(&m_deviceStatus, data, sizeof(CMyDeviceStatus));
}

void GB28181Client::HandleRecordInfoData(void* data)
{
	if (!data)
		return;

	CMyRecordInfo recordInfo;
	memcpy(&recordInfo, data, sizeof(CMyRecordInfo));

	if (m_GBRecordInfoDlg)
		m_GBRecordInfoDlg->AddRecordInfo(recordInfo);
}
