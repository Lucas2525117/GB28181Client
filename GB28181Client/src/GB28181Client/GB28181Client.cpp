#include "GB28181Client.h"
#include "mpeg-ps.h"
#include <thread>

static void MyGBMsgCB(int type, void* user, void* data)
{
	assert(user);
	GB28181Client* cli = (GB28181Client*)user;
	cli->HandleGBMsgCB(type, data);
}

static void VideoDataCB(int avtype, void* data, int dataLen, void* user)
{
	assert(user);
	GB28181Client* cli = (GB28181Client*)user;
	cli->HandleVideoDataCB(avtype, data, dataLen);
}

static void AudioDataCB(int avtype, void* data, int dataLen, void* user)
{
	assert(user);
	GB28181Client* cli = (GB28181Client*)user;
	cli->HandleAudioDataCB(avtype, data, dataLen);
}

int VideoDataThread(void* param, const std::string& localip, const std::string& localport)
{
	assert(param);
	GB28181Client* client = (GB28181Client*)param;
	client->VidioDataWorker(localip, localport);
	return 0;
}

int AudioDataThread(void* param, const std::string& localip, const std::string& localport)
{
	assert(param);
	GB28181Client* client = (GB28181Client*)param;
	client->AudioDataWorker(localip, localport);
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
		connect(m_GBCataLogDlg, &GBCataLogDlg::sigQueryCatalog, [=](const QString& deviceID) {
			if (!deviceID.isEmpty())
			{
				if (0 != GB_QueryNetDeviceInfo(Type_RecvCatalog, deviceID.toStdString().c_str()))
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
		connect(m_GBDeviceInfoDlg, &GBDeviceInfoDlg::sigQueryDeviceInfo, [=](const QString& deviceID) {
			if (!deviceID.isEmpty())
			{
				if (0 != GB_QueryNetDeviceInfo(Type_RecvDeviceInfo, deviceID.toStdString().c_str()))
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
		connect(m_GBDeviceStatusDlg, &GBDeviceStatusDlg::sigQueryDeviceStatus, [=](const QString& deviceID) {
			if (!deviceID.isEmpty())
			{
				if (0 != GB_QueryNetDeviceInfo(Type_RecvDeviceStatus, deviceID.toStdString().c_str()))
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

	// PTZ控制界面
	m_ptzControlDlg = new(std::nothrow) PTZControlDlg();
	if (m_ptzControlDlg)
	{
		connect(m_ptzControlDlg, &PTZControlDlg::sigPTZCmd, this, &GB28181Client::slotPTZControl);
	}

	// 订阅界面
	m_GBSubscribeDlg = new(std::nothrow) GBSubscribeDlg();
	if (m_GBSubscribeDlg)
	{
		connect(m_GBSubscribeDlg, &GBSubscribeDlg::sigSubscribe, this, &GB28181Client::slotSubscribe);
	}

	// 语音对讲界面
	m_GBTalkDlg = new(std::nothrow) GBTalkDlg();
	if (m_GBTalkDlg)
	{
		connect(m_GBTalkDlg, &GBTalkDlg::sigSTalk, this, &GB28181Client::slotStartTalk);
		connect(m_GBTalkDlg, &GBTalkDlg::sigStopTalk, this, &GB28181Client::slotStopTalk);
	}

	// 语音广播
	m_GBVoiceBroadcastDlg = new(std::nothrow) GBVoiceBroadcastDlg();
	if (m_GBVoiceBroadcastDlg)
	{
		connect(m_GBVoiceBroadcastDlg, &GBVoiceBroadcastDlg::sigVoiceBroadcast, [=](const QString& sourceID, const QString& targetID) {
			if (!m_gbid.empty() && !sourceID.isEmpty() || !targetID.isEmpty())
			{
				if (0 != GB_VoiceBroadcast(sourceID.toStdString().c_str(), m_gbid.c_str(), targetID.toStdString().c_str()))
				{
					QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("开启语音广播失败"), QMessageBox::Ok);
					return;
				}
			}
			});
	}

	m_tabWidget = new(std::nothrow) QTabWidget();
	if (m_tabWidget)
	{
		m_tabWidget->setTabsClosable(false);
		connect(m_tabWidget, &QTabWidget::tabBarClicked, this, [=](int index) {
			if (5 == index)
			{
				m_stackedWidget->setCurrentIndex(1);
			}
			else if (8 == index)
			{
				m_stackedWidget->setCurrentIndex(2);
			}
			else
			{
				m_stackedWidget->setCurrentIndex(0);
			}

			m_tabWidget->setCurrentIndex(index);
			});

		m_tabWidget->addTab(m_GBRegisterDlg, QString::fromLocal8Bit("注册与注销"));
		m_tabWidget->addTab(m_GBCataLogDlg, QString::fromLocal8Bit("设备目录"));
		m_tabWidget->addTab(m_GBDeviceInfoDlg, QString::fromLocal8Bit("设备信息"));
		m_tabWidget->addTab(m_GBDeviceStatusDlg, QString::fromLocal8Bit("设备状态"));
		m_tabWidget->addTab(m_GBVideoPlayDlg, QString::fromLocal8Bit("视频点播"));
		m_tabWidget->addTab(m_GBRecordInfoDlg, QString::fromLocal8Bit("视音频文件检索"));
		m_tabWidget->addTab(m_ptzControlDlg, QString::fromLocal8Bit("控制(PTZ控制)"));
		m_tabWidget->addTab(m_GBSubscribeDlg, QString::fromLocal8Bit("订阅与通知"));
		m_tabWidget->addTab(m_GBTalkDlg, QString::fromLocal8Bit("语音对讲"));
		m_tabWidget->addTab(m_GBVoiceBroadcastDlg, QString::fromLocal8Bit("语音广播"));
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
	// 添加通道界面
	m_addChannelDlg = new(std::nothrow) AddChannelDlg();
	if (m_addChannelDlg)
	{
		connect(m_addChannelDlg, SIGNAL(sigAddChannel(const QString&)), this, SLOT(slotAddChannel(const QString&)));
	}

	// 全局配置界面
	m_globalConfigDlg = new(std::nothrow) GlobalConfigDlg();
	if (m_globalConfigDlg)
	{
		connect(m_globalConfigDlg, &GlobalConfigDlg::sigGlobalConfig, this, &GB28181Client::slotSetGlobalParam);
	}

	m_stackedWidget = new(std::nothrow) QStackedWidget();
	setCentralWidget(m_stackedWidget);

	// 视频展示区
	m_playWidget = new(std::nothrow) PlayWidget();
	if(m_playWidget)
		m_playWidget->Init();

	// 音频播放区
	m_audioPlayWidget = new(std::nothrow) AudioPlayWidget();
	if (m_audioPlayWidget)
		m_audioPlayWidget->Init();

	m_GBRecordInfoResultDlg = new(std::nothrow) GBRecordInfoResultDlg();
	m_stackedWidget->insertWidget(0, m_playWidget);
	m_stackedWidget->insertWidget(1, m_GBRecordInfoResultDlg);
	m_stackedWidget->insertWidget(2, m_audioPlayWidget);

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
		//m_listWidget->hide();
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
		m_logView->setWindowTitle(QString::fromLocal8Bit("报警输出"));
		addDockWidget(Qt::BottomDockWidgetArea, m_logView, Qt::Orientation::Vertical);
		m_logView->setWidget(m_listWidget);
		//m_logView->hide();
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
}

void GB28181Client::InitAction()
{
	QMenuBar* menuBar = new QMenuBar();
	setMenuBar(menuBar);

	QMenu* menuConnect = new QMenu(QString::fromLocal8Bit("开始"), this);
	QMenu* menuOper = new QMenu(QString::fromLocal8Bit("操作"), this);
	QMenu* menuConfig = new QMenu(QString::fromLocal8Bit("配置"), this);
	QMenu* menuHelp = new QMenu(QString::fromLocal8Bit("帮助"), this);
	menuBar->addMenu(menuConnect);
	menuBar->addMenu(menuOper);
	menuBar->addMenu(menuConfig);
	menuBar->addMenu(menuHelp);

	QAction* actAddOrg = new QAction(QString::fromLocal8Bit("添加组织"), this);
	QAction* actClose = new QAction(QString::fromLocal8Bit("关闭"), this);
	menuConnect->addAction(actAddOrg);
	menuConnect->addAction(actClose);

	QAction* actShowOper = new QAction(QString::fromLocal8Bit("显示操作区"), this);
	QAction* actHideOper = new QAction(QString::fromLocal8Bit("隐藏操作区"), this);
	QAction* actShowAlarm = new QAction(QString::fromLocal8Bit("显示报警展示区"), this);
	QAction* actHideAlarm = new QAction(QString::fromLocal8Bit("隐藏报警展示区"), this);
	menuOper->addAction(actShowOper);
	menuOper->addAction(actHideOper);
	menuOper->addAction(actShowAlarm);
	menuOper->addAction(actHideAlarm);

	QAction* actGlobalConfig = new QAction(QString::fromLocal8Bit("全局配置"), this);
	menuConfig->addAction(actGlobalConfig);

	QAction* actVersion = new QAction(QString::fromLocal8Bit("版本信息"), this);
	menuHelp->addAction(actVersion);

	connect(actAddOrg, &QAction::triggered, [=]() {
		if (nullptr != m_addOrgDlg)
			m_addOrgDlg->show();
		});

	connect(actShowOper, &QAction::triggered, [=]() {
		if (nullptr != m_operView)
			m_operView->show();
		});

	connect(actHideOper, &QAction::triggered, [=]() {
		if (nullptr != m_operView)
			m_operView->hide();
		});

	connect(actShowAlarm, &QAction::triggered, [=]() {
		if (nullptr != m_logView)
			m_logView->show();
		});

	connect(actHideAlarm, &QAction::triggered, [=]() {
		if (nullptr != m_logView)
			m_logView->hide();
		});

	connect(actClose, &QAction::triggered, [=]() {
		exit(0);
		});

	connect(actGlobalConfig, &QAction::triggered, [=]() {
		if (m_globalConfigDlg)
			m_globalConfigDlg->show();
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
	//if (nullptr != m_listWidget)
	//	m_listWidget->addItem(strText);
}

void GB28181Client::AddAlarm(const QString& strAlarmInfo)
{
	if (nullptr != m_listWidget)
		m_listWidget->addItem(strAlarmInfo);
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
		char* token = nullptr;
		GB_Invite(mediaContext, (GB_TOKEN*)&token);
		m_token = token;
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
	std::string localcontact = "SIP:" + gbid +"@" + ip + ":" + std::to_string(sipport);
	if (!GB_Init(localcontact.c_str(), 3))
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("SIP初始化失败"), QMessageBox::Ok);
		return;
	}

	m_gbid = gbid;
	m_serverStart = true;
	GB_RegisterHandler(Type_Register, MyGBMsgCB, this);
	GB_RegisterHandler(Type_KeepAlive, MyGBMsgCB, this);
	GB_RegisterHandler(Type_RecvCatalog, MyGBMsgCB, this);
	GB_RegisterHandler(Type_RecvRecordInfo, MyGBMsgCB, this);
	GB_RegisterHandler(Type_RecvDeviceInfo, MyGBMsgCB, this);
	GB_RegisterHandler(Type_RecvDeviceStatus, MyGBMsgCB, this);
	GB_RegisterHandler(Type_Alarm, MyGBMsgCB, this);
	GB_RegisterHandler(Type_VoiceBroadcast, MyGBMsgCB, this);
	GB_RegisterHandler(Type_Invite, MyGBMsgCB, this);
	GB_RegisterHandler(Type_VideoInvite, MyGBMsgCB, this);
	GB_RegisterHandler(Type_Bye, MyGBMsgCB, this);
}

void GB28181Client::Stop()
{
	if (!m_serverStart)
		return;

	m_serverStart = false;
	if (!GB_UnInit())
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("注销失败"), QMessageBox::Ok);
		return;
	}

	if (m_GBRegisterDlg)
		m_GBRegisterDlg->SetRegisterResult(QString::fromLocal8Bit("设备注销成功"));
}

void GB28181Client::closeEvent(QCloseEvent* event)
{
	exit(0);
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
	mediaContext.SetStreamType(StreamType_RealStream);
	mediaContext.SetStreamTransMode((StreamTransMode)m_globalParam.streamTransMode);
	char* token = nullptr;
	if (!GB_Invite(mediaContext, (GB_TOKEN*)&token))
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("开始视频点播失败"), QMessageBox::Ok);

	m_localIP = localIP;
	m_token = token;

	if (2 != m_globalParam.streamTransMode)
	{
		std::thread th(VideoDataThread, this, localIP.toStdString(), localRecvPort.toStdString());
		th.detach();
	}
}

void GB28181Client::slotStopVideoPlay()
{
	if (m_token.empty() || !m_receiver)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请先开始视频点播"), QMessageBox::Ok);
		return;
	}

	if (m_audioPlayWidget)
		m_audioPlayWidget->Stop();
	
	m_receiver->Stop();
	m_receiver->DeleteThis();
	m_receiver = nullptr;
	GB_Bye(m_token.c_str());
}

void GB28181Client::slotStartTalk(const QString& gbid, const QString& deviceIP, const QString& gbPort, const QString& localIP, const QString& localRecvPort)
{
	if (m_audioReceiver)
	{
		QMessageBox::information(this, QString::fromLocal8Bit("通知"), QString::fromLocal8Bit("当前正在对讲"), QMessageBox::Ok);
		return;
	}

	std::string requestUrl = "sip:" + gbid.toStdString() + "@" + deviceIP.toStdString() + ":" + gbPort.toStdString();
	GB28181MediaContext mediaContext(requestUrl);
	mediaContext.SetRecvAddress(localIP.toStdString());
	mediaContext.SetRecvPort(localRecvPort.toInt());
	mediaContext.SetStreamType(StreamType_Audio);
	char* token = nullptr;
	if (!GB_Invite(mediaContext, (GB_TOKEN*)&token))
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("开始视频点播失败"), QMessageBox::Ok);
		return;
	}	

	m_audioToken = token;
	std::thread th(AudioDataThread, this, localIP.toStdString(), localRecvPort.toStdString());
	th.detach();
}

void GB28181Client::slotStopTalk()
{
	if (m_audioToken.empty() || !m_audioReceiver)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请先开始视频点播"), QMessageBox::Ok);
		return;
	}

	m_audioReceiver->Stop();
	m_audioReceiver->DeleteThis();
	m_audioReceiver = nullptr;
	GB_Bye(m_audioToken.c_str());
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
	GB_QueryRecordInfo(gbid.toStdString().c_str(), mediaContext);
}

void GB28181Client::slotPTZControl(const QString& gbid, int type, int paramValue)
{
	int ret = GB_PTZControl(gbid.toStdString().c_str(), (PTZControlType)type, paramValue);
	if (0 != ret)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("PTZ执行失败"), QMessageBox::Ok);
		return;
	}
	
	// 停止
	ret = GB_PTZControl(gbid.toStdString().c_str(), PTZ_CTRL_HALT, 0);
	if(0 != ret)
		QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("PTZ停止失败"), QMessageBox::Ok);
}

void GB28181Client::slotSubscribe(const QString& gbid, const QString& ipp, const QString& startTime, const QString& endTime, int subType, int expires)
{
	auto it = m_mapSubscribeObjs.find(subType);
	if (it != m_mapSubscribeObjs.end() && expires > 0)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("当前订阅已开启"), QMessageBox::Ok);
		return;
	}

	GBSubscribeContext subContext;
	std::string requestUrl = "sip:" + gbid.toStdString() + "@" + ipp.toStdString();
	subContext.SetRequestUrl(requestUrl);
	subContext.SetDeviceId(gbid.toStdString().c_str());
	subContext.SetSubscirbeType((SubscribeType)subType);
	subContext.SetExpires(expires);
	subContext.SetSubStartTime(startTime.toStdString());
	subContext.SetSubEndTime(endTime.toStdString());
	char* token = nullptr;
	if (!GB_Subscribe(subContext, (GB_TOKEN*)&token))
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("订阅失败"), QMessageBox::Ok);
		return;
	}
		
	if (expires > 0)
	{
		m_mapSubscribeObjs.insert(std::make_pair(subType, token));
	}
	else if (0 == expires)
	{
		m_mapSubscribeObjs.erase(it);
	}
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
	case Type_Alarm:
		HandleAlarmInfoData(data);
		break;
	case Type_VoiceBroadcast:
		HandleVoiceBroadcastData(data);
		break;
	case Type_VideoInvite:
		HandleVideoInviteData(data);
		break;
	default:
		break;
	}
}

void GB28181Client::HandleVideoDataCB(int avtype, void* data, int dataLen)
{
	if (m_playWidget)
		m_playWidget->AddData(CODEC_VIDEO_H264, data, dataLen);
}

void GB28181Client::HandleAudioDataCB(int avtype, void* data, int dataLen)
{
	if (m_audioPlayWidget)
		m_audioPlayWidget->AddData(CODEC_AUDIO_G711, data, dataLen);
}

void GB28181Client::VidioDataWorker(const std::string& localip, const std::string& localport)
{
	std::string gburl = "";
	if(0 == m_globalParam.streamTransMode)          // udp
		gburl = "gbudp://" + localip + ":" + localport;
	else if(1 == m_globalParam.streamTransMode)     // tcp被动
		gburl = "gbtcps://" + localip + ":" + localport;
	else if (2 == m_globalParam.streamTransMode)    // tcp主动
		gburl = "gbtcpc://" + localip + ":" + QString::number(m_videoInviteInfo.transport).toStdString().c_str();

	m_receiver = GB_CreateStreamReceiver(gburl.c_str(), VideoDataCB, this);
	if(m_receiver)
		m_receiver->Start(0);
}

void GB28181Client::AudioDataWorker(const std::string& localip, const std::string& localport)
{
	std::string gburl = "gbudp://" + localip + ":" + localport;
	m_audioReceiver = GB_CreateStreamReceiver(gburl.c_str(), AudioDataCB, this);
	if(m_audioReceiver)
		m_audioReceiver->Start(1);
}

void GB28181Client::HandleCataLogData(void* data)
{
	if (!data)
		return;

	memcpy(&m_catalog, data, sizeof(CMyCatalogInfo));

	if (m_GBCataLogDlg)
	{
		m_GBCataLogDlg->AddCatalogData(m_catalog);
	}
}

void GB28181Client::HandleRegisterData(void* data)
{
	if (!data)
		return;

	m_registerCBMsg = (char*)data;

	if (0 == m_registerCBMsg.compare("register ok"))
	{
		if (m_GBRegisterDlg)
			m_GBRegisterDlg->SetRegisterResult(QString::fromLocal8Bit("设备注册成功"));
	}
	else if (0 == m_registerCBMsg.compare("unregister ok"))
	{
		if (m_GBRegisterDlg)
			m_GBRegisterDlg->SetRegisterResult(QString::fromLocal8Bit("设备注销成功"));

		if (m_serverStart)
		{
			if (!GB_UnInit())
				return;
			m_serverStart = false;
		}
	}
}

void GB28181Client::HandleDeviceInfoData(void* data)
{
	if (!data)
		return;

	memcpy(&m_deviceinfo, data, sizeof(CMyDeviceInfo));

	if (m_GBDeviceInfoDlg)
	{
		m_GBDeviceInfoDlg->AddDeviceInfoData(m_deviceinfo);
	}
}

void GB28181Client::HandleDeviceStatusData(void* data)
{
	if (!data)
		return;

	memcpy(&m_deviceStatus, data, sizeof(CMyDeviceStatus));

	if (m_GBDeviceStatusDlg)
	{
		m_GBDeviceStatusDlg->AddDeviceStatusData(m_deviceStatus);
	}
}

void GB28181Client::HandleRecordInfoData(void* data)
{
	if (!data)
		return;

	CMyRecordInfo* recordInfo = new CMyRecordInfo();
	memcpy(recordInfo, data, sizeof(CMyRecordInfo));

	if (m_GBRecordInfoResultDlg)
	{
		m_GBRecordInfoResultDlg->AddRecordInfo(m_gbid.c_str(), recordInfo);
	}
}

void GB28181Client::HandleAlarmInfoData(void* data)
{
	if (!data)
		return;

	memcpy(&m_alarmInfo, data, sizeof(CMyAlarmInfo));

	QString strAlarmInfo = QString::fromLocal8Bit("接收到告警,报警编码:%1 ").arg(m_alarmInfo.deviceID.c_str());

	QString strAlarmProrityText;
	if (AlarmPriority_Undefined == m_alarmInfo.alarmPrority)
		strAlarmProrityText = QString::fromLocal8Bit("未定义(0)");
	else if(AlarmPriority_One == m_alarmInfo.alarmPrority)
		strAlarmProrityText = QString::fromLocal8Bit("一级警情(1)");
	else if (AlarmPriority_Two == m_alarmInfo.alarmPrority)
		strAlarmProrityText = QString::fromLocal8Bit("二级警情(2)");
	else if (AlarmPriority_Three == m_alarmInfo.alarmPrority)
		strAlarmProrityText = QString::fromLocal8Bit("三级警情(3)");
	else if (AlarmPriority_Three == m_alarmInfo.alarmPrority)
		strAlarmProrityText = QString::fromLocal8Bit("四级警情(4)");
	strAlarmInfo += QString::fromLocal8Bit("报警级别:%1 ").arg(strAlarmProrityText);

	QString strAlarmMethodText;
	if (Alarm_Undefined == m_alarmInfo.alarmMethod)
		strAlarmMethodText = QString::fromLocal8Bit("未定义(0)");
	else if (Alarm_Phone == m_alarmInfo.alarmMethod)
		strAlarmMethodText = QString::fromLocal8Bit("电话告警(1)");
	else if (Alarm_Device == m_alarmInfo.alarmMethod)
		strAlarmMethodText = QString::fromLocal8Bit("设备告警(2)");
	else if (Alarm_TextMessage == m_alarmInfo.alarmMethod)
		strAlarmMethodText = QString::fromLocal8Bit("短信告警(3)");
	else if (Alarm_GPS == m_alarmInfo.alarmMethod)
		strAlarmMethodText = QString::fromLocal8Bit("GPS告警(4)");
	else if (Alarm_Video == m_alarmInfo.alarmMethod)
		strAlarmMethodText = QString::fromLocal8Bit("视频告警(4)");
	else if (Alarm_DeviceBreak == m_alarmInfo.alarmMethod)
		strAlarmMethodText = QString::fromLocal8Bit("设备故障告警(4)");
	else if (Alarm_Other == m_alarmInfo.alarmMethod)
		strAlarmMethodText = QString::fromLocal8Bit("其他告警(4)");
	strAlarmInfo += QString::fromLocal8Bit("报警方式:%1 ").arg(strAlarmMethodText);

	strAlarmInfo += QString::fromLocal8Bit("报警时间:%1 ").arg(m_alarmInfo.alarmTime.c_str());
	strAlarmInfo += QString::fromLocal8Bit("报警描述:%1").arg(m_alarmInfo.alarmDescription.c_str());
	AddAlarm(strAlarmInfo);
}

void GB28181Client::HandleVoiceBroadcastData(void* data)
{
	if (!data)
		return;

	memcpy(&m_broadcastInfo, data, sizeof(CMyBroadcastInfo));

	if (m_GBVoiceBroadcastDlg)
		m_GBVoiceBroadcastDlg->SetBroadcastResult(m_broadcastInfo.result.c_str());
}

void GB28181Client::HandleVideoInviteData(void* data)
{
	if (!data)
		return;

	memcpy(&m_videoInviteInfo, data, sizeof(CMyVideoInviteInfo));
	
	QString deviceIP = m_videoInviteInfo.deviceIP.c_str();
	QString transPort = QString::number(m_videoInviteInfo.transport);
	std::thread th(VideoDataThread, this, deviceIP.toStdString(), transPort.toStdString().c_str());
	th.detach();
}
