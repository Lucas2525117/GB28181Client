#pragma once

#include <QtWidgets/QMainWindow>
#include <qtreewidget.h>
#include <qdockwidget.h>
#include <qtreeview.h>
#include <qheaderview.h>
#include <qlistwidget.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <QStackedWidget>
#include <string>
#include "ui_GB28181Client.h"
#include "GBRegisterDlg.h"
#include "GBCataLogDlg.h"
#include "GBDeviceInfoDlg.h"
#include "GBDeviceStatusDlg.h"
#include "GBVideoPlayDlg.h"
#include "GBRecordInfoDlg.h"
#include "GBRecordInfoResultDlg.h"
#include "AddOrgDlg.h"
#include "AddDeviceDlg.h"
#include "AddChannelDlg.h"
#include "PTZControlDlg.h"
#include "public.h"
#include "GB28181Server.h"
#include "MySipInfo.h"
#include "StreamReceiverInterface.h"
#include "PlayWidget.h"

class GB28181Client : public QMainWindow
{
    Q_OBJECT

public:
    GB28181Client(QWidget *parent = nullptr);
    ~GB28181Client();

    void HandleGBMsgCB(int type, void* data);
    void HandleGBDataCB(int avtype, void* data, int dataLen);

    void GBClientDataWorker(const std::string& localip, const std::string& localport);

private:
    void InitUi();
    void InitAction();
    QTreeWidgetItem* SearchChild(QTreeWidgetItem* item, const QString& strMsg);
    void AddLog(const QString& strText);
    void AddAlarm(const QString& strAlarmInfo);
    void HandleCataLogData(void* data);
    void HandleRegisterData(void* data);
    void HandleDeviceInfoData(void* data);
    void HandleDeviceStatusData(void* data);
    void HandleRecordInfoData(void* data);
    void HandleAlarmInfoData(void* data);

public slots:
    void slotItemDoubleClick(QTreeWidgetItem* item, int index);
    void slotContextMenu(const QPoint& pos);
    void slotAddOrg(const QString& orgName);
    void slotAddDevice(const QString& deviceIp);
    void slotAddChannel(const QString& channelNum);

    void Start(const std::string& gbid, const std::string& ip, int sipport);
    void Stop();

protected:
    virtual void closeEvent(QCloseEvent* event) override;

private slots:
    void slotCatalogTimer();
    void slotStartVideoPlay(const QString& gbid, const QString& deviceIP, const QString& gbPort, const QString& localIP, const QString& localRecvPort);
    void slotStopVideoPlay();
    void slotQueryRecordInfo(const QString& gbid, const QString& startTime, const QString& endTime);
    void slotPTZControl(const QString& gbid, int type, int paramValue);

private:
    Ui::GB28181ClientClass ui;

    QTreeWidget*       m_treeWidget      = nullptr;
    QDockWidget*       m_projManagerView = nullptr;
    QListWidget*       m_listWidget      = nullptr;
    QDockWidget*       m_logView         = nullptr;
    QDockWidget*       m_operView        = nullptr;
    GBRegisterDlg*     m_GBRegisterDlg   = nullptr;
    GBCataLogDlg*      m_GBCataLogDlg    = nullptr;
    GBDeviceStatusDlg* m_GBDeviceStatusDlg = nullptr;
    GBDeviceInfoDlg*   m_GBDeviceInfoDlg = nullptr;
    GBVideoPlayDlg*    m_GBVideoPlayDlg  = nullptr;
    GBRecordInfoDlg*   m_GBRecordInfoDlg = nullptr;
    GBRecordInfoResultDlg* m_GBRecordInfoResultDlg = nullptr;
    AddOrgDlg*         m_addOrgDlg       = nullptr;
    AddDeviceDlg*      m_addDeviceDlg    = nullptr;
    AddChannelDlg*     m_addChannelDlg   = nullptr;
    PTZControlDlg*     m_ptzControlDlg = nullptr;
    QTabWidget*        m_tabWidget       = nullptr;
    QWidget*           m_widget          = nullptr;
    IStreamReceiver*   m_receiver        = nullptr;
    PlayWidget*        m_playWidget      = nullptr;
    QStackedWidget*    m_stackedWidget   = nullptr;

    CMyCatalogInfo     m_catalog;
    CMyDeviceInfo      m_deviceinfo;
    CMyDeviceStatus    m_deviceStatus;
    CMyAlarmInfo       m_alarmInfo;
    std::string        m_registerCBMsg;
    std::string        m_gbid;
    std::string        m_token;
    
    bool               m_serverStart = false;
};
