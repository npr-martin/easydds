#include "easyddstest.h"
#include "ui_easyddstest.h"

#include <QDebug>
#include <QMessageBox>
#include <QFormLayout>
#include <QTimer>
#include <QFileDialog>
#include <QTextStream>
#include <QEventLoop>
#include <QStandardPaths>

#include <thread>
#include <iostream>
#include "qosdialog.h"

#include "fastdds/dds/log/Log.hpp"
#include "fastdds/dds/log/FileConsumer.hpp"

#include <unistd.h>

EasyDDSTest::EasyDDSTest(const QString& type, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::EasyDDSTest)
{
    QIcon appIcon(":/EDTest.png");

    // 设置应用程序图标
    this->setWindowIcon(appIcon);
    ui->setupUi(this);

    initInvisible();

    buffer = std::make_shared<qtStreamBuf>(this);
    new (&std::cout) std::ostream(buffer.get());

    QStringList labels({"编号", "主题", "状态", "频率", "源大小"});
    ui->tableWidget->setColumnCount(labels.size());
    ui->tableWidget->setHorizontalHeaderLabels(labels);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    ui->tableWidget->verticalHeader()->setVisible(false);

    qRegisterMetaType<std::shared_ptr<easyddsClientPublisherApp>>("std::shared_ptr<easyddsClientPublisherApp>");
    qRegisterMetaType<std::string>("std::string");
    connect(this, &EasyDDSTest::setCoutText, this, &EasyDDSTest::addText);
    connect(this, &EasyDDSTest::sendTextSignal, this, & EasyDDSTest::sendText, Qt::QueuedConnection);

    // First remove previous executions file
#if 0
    pid_t pid = getpid();

    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    std::string logName = homePath.toStdString() + "/append_" + std::to_string(pid) + ".log";

    std::remove(logName.c_str());

    Log::ClearConsumers();
    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(new FileConsumer(logName)));
    Log::ReportFunctions(true);
    Log::SetVerbosity(Log::Info);
#endif

    if(type == "subscriber")
    {
        ui->comboBox->setCurrentText(type);
    }
    else if(type.startsWith("ds_"))
    {
        ui->radioButton_2->click();
        ui->comboBox->setCurrentText(type.mid(3));
    }
}

EasyDDSTest::~EasyDDSTest()
{
    delete ui;
}

void EasyDDSTest::Append(const QString &text)
{
    //    static QString textbuf;
    //    textbuf+=text;

    emit setCoutText(text);
}

void EasyDDSTest::on_pushButton_clicked()
{
    int num = ui->sbNum->value();
    m_kindName = ui->comboBox->currentText().toStdString();

    if(m_kindName != "publisher")
    {
        QStringList labels({"编号", "主题", "状态"});
        ui->tableWidget->setColumnCount(labels.size());
        ui->tableWidget->setHorizontalHeaderLabels(labels);
    }

    if(num == 1)
    {
        QString leTopicName = ui->leTopic->text();
        QString cmbTopicName = ui->cmbTopic->currentText();
        createApp(0, m_kindName == "monitor" ? cmbTopicName : leTopicName,
                  ui->sbFrequency->value(), m_source);
    }
    else
    {
        for(int i = 0; i < num; ++i)
        {
            QString topicName = QString("%1_%2").arg(ui->leTopic->text()).arg(i);
            createApp(0, topicName, ui->sbFrequency->value(), m_source);
        }
    }
}

void EasyDDSTest::addInfoTab(const QString &topicName, int frequency, const QString& source, const QString& guid)
{
    int rowNum = ui->tableWidget->rowCount();

    auto splitResult = splitGUID(guid);
    ui->lblList->setText(QString("%1 列表 [进程标识：%2]").arg(m_kindName.c_str()).arg(splitResult.first));

    QLabel* lblGUID = new QLabel(splitResult.second, ui->tableWidget);
    QLabel* lblTopic = new QLabel(topicName, ui->tableWidget);
    QLabel* lblFrequency = new QLabel(QString::number(frequency), ui->tableWidget);
    QLabel* lblSize = new QLabel(QString::number(source.size()), ui->tableWidget);

    lblGUID->setAlignment(Qt::AlignCenter);
    lblTopic->setAlignment(Qt::AlignCenter);
    lblFrequency->setAlignment(Qt::AlignCenter);
    lblSize->setAlignment(Qt::AlignCenter);

    QPushButton* btn = new QPushButton("stop", ui->tableWidget);

    connect(btn, &QPushButton::clicked, ui->tableWidget, [=]{
        QString btnText = btn->text();
        int curRow = getWidgetRow(lblGUID, 0);
        if(-1 == curRow)
        {
            qDebug() << "get wrong Row for current info.";
        }
        else
        {
            if("stop" == btnText)
            {
                stopApp(curRow);
                std::cout << topicName.toStdString() << "'s "
                          << ui->comboBox->currentText().toStdString()
                          << " has stopped." << std::endl;
                btn->setText("start");
            }
            else
            {
                btn->setText("stop");
                QString newGUID = createApp(0, topicName, ui->sbFrequency->value(), m_source, curRow, false);

                lblGUID->setText(splitGUID(newGUID).second);
            }
        }
    });

    ui->tableWidget->setRowCount(rowNum + 1);
    ui->tableWidget->setCellWidget(rowNum, 0, lblGUID);
    ui->tableWidget->setCellWidget(rowNum, 1, lblTopic);
    ui->tableWidget->setCellWidget(rowNum, 2, btn);
    ui->tableWidget->setCellWidget(rowNum, 3, lblFrequency);
    ui->tableWidget->setCellWidget(rowNum, 4, lblSize);
    ui->tableWidget->setItem(rowNum, 0, new QTableWidgetItem());
    ui->tableWidget->setItem(rowNum, 1, new QTableWidgetItem());
    ui->tableWidget->setItem(rowNum, 2, new QTableWidgetItem());
    ui->tableWidget->setItem(rowNum, 3, new QTableWidgetItem());
    ui->tableWidget->setItem(rowNum, 4, new QTableWidgetItem());
}

QString EasyDDSTest::createApp(int domain_id, const QString &topicName, int frequency, const QString& source, int curRow, bool addRow)
{
    if("publisher" == m_kindName)
    {
        return createPubliserApp(domain_id, topicName, frequency, source, curRow, addRow);
    }
    else if("subscriber" == m_kindName)
    {
        return createSubscriberApp(domain_id, topicName, curRow, addRow);
    }
    else if("server" == m_kindName)
    {
        return createServer(domain_id, curRow, addRow);
    }
    else
    {
        return createMonitor(domain_id, topicName, curRow, addRow);
    }

//    EPROSIMA_LOG_ERROR("type",m_kindName);
    return QString();
}

QString EasyDDSTest::createPubliserApp(int domain_id, const QString &topicName, int frequency,
                                    const QString& source, int curRow, bool addRow)
{
    QString guid;

    std::shared_ptr<easyddsClientPublisherApp> app = nullptr;
    if(!topicName.isEmpty())
    {
        app = easyddsApplication::createClientPublisher(topicName.toStdString(),
                                                        domain_id, getEasyConfig());

        std::thread thread(&easyddsApplication::run, app);
        thread.detach();

        std::cout << topicName.toStdString()
                  << "'s publisher running. "
                     "Please press stop Button to stop the publisher at any time." << std::endl;

        guid = QString::fromStdString(app->getGUID());

        if(addRow)
        {
            addInfoTab(topicName, frequency, source, guid);
            m_pubInfos.append(app);
        }
        else
        {
            if(curRow < m_pubInfos.size())
            {
                m_pubInfos[curRow] = app;
            }
            else
            {
                qDebug() << "Wrong curRow for info buffer.(curRow = " << curRow << ", bufferSize = " << m_pubInfos.size();
            }
        }

        ui->comboBox->setEnabled(false);
        emit sendTextSignal(app, topicName, frequency, source);
    }
    else
    {
        QMessageBox::warning(this, "Topic Error", "Publisher Topic can't be empty.");
    }

    //    sendText(app, frequency, source);

    return guid;
}

QString EasyDDSTest::createSubscriberApp(int domain_id, const QString &topicName, int curRow, bool addRow)
{
    QString guid;

    std::shared_ptr<easyddsClientSubscriberApp> app = nullptr;
    if(!topicName.isEmpty())
    {
        app = easyddsApplication::createClientSubscriber(topicName.toStdString(),
                                                         domain_id, getEasyConfig());

        std::thread thread(&easyddsApplication::run, app);
        thread.detach();

        std::cout << topicName.toStdString() << "'s subscriber running. "
                                                "Please press stop Button to stop the subscriber at any time." << std::endl;

        guid = QString::fromStdString(app->getGUID());

        if(addRow)
        {
            addInfoTab(topicName, 500, "", guid);
            m_subInfos.append(app);
        }
        else
        {
            if(curRow < m_subInfos.size())
            {
                m_subInfos[curRow] = app;
            }
            else
            {
                qDebug() << "Wrong curRow for info buffer.(curRow = " << curRow << ", bufferSize = " << m_subInfos.size();
            }
        }

        ui->comboBox->setEnabled(false);
    }
    else
    {
        QMessageBox::warning(this, "Topic Error", "Publisher Topic can't be empty.");
    }

    app->onMessageReceived(printRecvMsg);

    return guid;
}

QString EasyDDSTest::createMonitor(int domain_id, const QString &topicName, int curRow, bool addRow)
{
    QString guid;

    std::shared_ptr<easyddsMonitorSub> app = nullptr;

    app = easyddsApplication::createMoniterSubscriber(topicName.toStdString(), domain_id);

    std::thread thread(&easyddsApplication::run, app);
    thread.detach();

    std::cout << "Monitor " << topicName.toStdString()
              << " is running. Please press stop Button to stop the monitor at any time." << std::endl;

    guid = QString::fromStdString(app->getGUID());

    if(addRow)
    {
        addInfoTab(topicName, 500, "", guid);
        m_monitorInfos.append(app);
    }
    else
    {
        if(curRow < m_monitorInfos.size())
        {
            m_monitorInfos[curRow] = app;
        }
        else
        {
            qDebug() << "Wrong curRow for info buffer.(curRow = " << curRow << ", bufferSize = " << m_subInfos.size();
        }
    }

    ui->comboBox->setEnabled(false);

    return guid;
}

QString EasyDDSTest::createServer(int domain_id, int curRow, bool addRow)
{
    QString guid;

    std::shared_ptr<easyddsServerApp> app = nullptr;

    app = easyddsApplication::createServer(domain_id, getEasyServerConfig());

    std::thread thread(&easyddsApplication::run, app);
    thread.detach();

    std::cout << "Server is running. Please press stop Button to stop the server at any time." << std::endl;

    guid = QString::fromStdString(app->getGUID());

    if(addRow)
    {
        addInfoTab("server", 500, "", guid);
        m_serverInfos.append(app);
    }
    else
    {
        if(curRow < m_serverInfos.size())
        {
            m_serverInfos[curRow] = app;
        }
        else
        {
            qDebug() << "wrong curRow for info buffer.(curRow = " << curRow << ", bufferSize = " << m_subInfos.size();
        }
    }

    ui->comboBox->setEnabled(false);

    return guid;
}

void EasyDDSTest::stopApp(int curRow)
{
    if("publisher" == m_kindName)
    {
        if(curRow < m_pubInfos.size())
        {
            m_pubInfos[curRow]->stop();
            m_pubInfos[curRow] = nullptr;
        }
    }
    else if("subscriber" == m_kindName)
    {
        if(curRow < m_subInfos.size())
        {
            m_subInfos[curRow]->stop();
            m_subInfos[curRow] = nullptr;
        }
    }
    else if("server" == m_kindName)
    {
        if(curRow < m_serverInfos.size())
        {
            m_serverInfos[curRow]->stop();
            m_serverInfos[curRow] = nullptr;
        }
    }
    else
    {
        if(curRow < m_monitorInfos.size())
        {
            m_monitorInfos[curRow]->stop();
            m_monitorInfos[curRow] = nullptr;
        }
    }
}

int EasyDDSTest::getWidgetRow(QWidget* widget, int column)
{
    for(int i = 0; i < ui->tableWidget->rowCount(); ++i)
    {
        QWidget* itemWidget = ui->tableWidget->cellWidget(i, column);
        if(itemWidget == widget)
        {
            return i;
        }
    }

    return -1;
}

void EasyDDSTest::multiOp(const QString &op)
{
    auto items = ui->tableWidget->selectedItems();

    QSet<int> selectedRows;
    for(auto item : items)
    {
        selectedRows.insert(item->row());
    }

    for(int row : selectedRows)
    {
        QPushButton* btn = qobject_cast<QPushButton*>(ui->tableWidget->cellWidget(row, 2));
        if(btn && btn->text() == op)
        {
            btn->clicked();
        }
        else
        {
            qDebug() << "Do not find btn.";
        }
    }

    ui->tableWidget->clearSelection();
}

EASYDDS::monitorItems EasyDDSTest::insertMonitorQos()
{
    uint64_t monitorItems = 0;
    auto children = ui->wgt_monitor->findChildren<QCheckBox*>();
    for(auto ckb : children)
    {
        if(ckb->isChecked())
        {
            QString text = ckb->text();
            if("GAP_COUNT" == text)
            {
                monitorItems |= EASYDDS::GAP_COUNT_TOPIC;
            }
            else if("RTPS_LOST" == text)
            {
                monitorItems |= EASYDDS::RTPS_LOST_TOPIC;
            }
            else if("RTPS_SENT" == text)
            {
                monitorItems |= EASYDDS::RTPS_SENT_TOPIC;
            }
            else if("DATA_COUNT" == text)
            {
                monitorItems |= EASYDDS::DATA_COUNT_TOPIC;
            }
            else if("EDP_PACKETS" == text)
            {
                monitorItems |= EASYDDS::EDP_PACKETS_TOPIC;
            }
            else if("PDP_PACKETS" == text)
            {
                monitorItems |= EASYDDS::PDP_PACKETS_TOPIC;
            }
            else if("RESENT_DATAS" == text)
            {
                monitorItems |= EASYDDS::RESENT_DATAS_TOPIC;
            }
            else if("SAMPLE_DATAS" == text)
            {
                monitorItems |= EASYDDS::SAMPLE_DATAS_TOPIC;
            }
            else if("ACKNACK_COUNT" == text)
            {
                monitorItems |= EASYDDS::ACKNACK_COUNT_TOPIC;
            }
            else if("PHYSICAL_DATA" == text)
            {
                monitorItems |= EASYDDS::PHYSICAL_DATA_TOPIC;
            }
            else if("NACKFRAG_COUNT" == text)
            {
                monitorItems |= EASYDDS::NACKFRAG_COUNT_TOPIC;
            }
            else if("HEARTBEAT_COUNT" == text)
            {
                monitorItems |= EASYDDS::HEARTBEAT_COUNT_TOPIC;
            }
            else if("HISTORY_LATENCY" == text)
            {
                monitorItems |= EASYDDS::HISTORY_LATENCY_TOPIC;
            }
            else if("NETWORK_LATENCY" == text)
            {
                monitorItems |= EASYDDS::NETWORK_LATENCY_TOPIC;
            }
            else if("PUB_THROUGHPUT" == text)
            {
                monitorItems |= EASYDDS::PUBLICATION_THROUGHPUT_TOPIC;
            }
            else if("SUB_THROUGHPUT" == text)
            {
                monitorItems |= EASYDDS::SUBSCRIPTION_THROUGHPUT_TOPIC;
            }
            else if("SENT_DATA" == text)
            {
                monitorItems |= EASYDDS::SENT_DATA_TOPIC;
            }
            else if("RECEIVED_DATA" == text)
            {
                monitorItems |= EASYDDS::RECEIVED_DATA_TOPIC;
            }
        }
    }
    return static_cast<EASYDDS::monitorItems>(monitorItems);
}

TransportKind EasyDDSTest::getCurKind()
{
    QString kind = ui->cmbTransKind->currentText();
    if("SHM" == kind)
    {
        return TransportKind::SHM;
    }
    else if("TCPv4" == kind)
    {
        return TransportKind::TCPv4;
    }
    else if("TCPv6" == kind)
    {
        return TransportKind::TCPv6;
    }
    else if("UDPv4" == kind)
    {
        return TransportKind::UDPv4;
    }
    else if("UDPv6" == kind)
    {
        return TransportKind::UDPv6;
    }
    else if("DATA_SHARING" == kind)
    {
        return TransportKind::DATA_SHARING;
    }
    else if("LARGE_DATA" == kind)
    {
        return TransportKind::LARGE_DATA;
    }

    return TransportKind::DEFAULT;
}

easyddsClientConfig EasyDDSTest::getEasyConfig()
{
    easyddsClientConfig cc;
    cc.qosProfile = m_qosProfile;
    cc.open_monitor = ui->ckb_all->isChecked();
    cc.items = insertMonitorQos();
    cc.clientConfig = getClientConfig();
    cc.useDiscoveryServer = ui->radioButton_2->isChecked();

    return cc;
}

client_config EasyDDSTest::getClientConfig()
{
    client_config cc;
    cc.connection_address = ui->leIP->text().toStdString();
    cc.connection_port = ui->sbPort->value();
    cc.transport_kind = getCurKind();

    return cc;
}

easyddsServerConfig EasyDDSTest::getEasyServerConfig()
{
    easyddsServerConfig sc;
    sc.qosProfile = m_qosProfile;
    sc.serverConfig = getServerConfig();
    sc.open_monitor = ui->ckb_all->isChecked();
    sc.items = insertMonitorQos();
    sc.useDiscoveryServer = ui->radioButton_2->isChecked();

    return sc;
}

server_config EasyDDSTest::getServerConfig()
{
    server_config sc;
    sc.listening_address = ui->leListenIP->text().toStdString();
    sc.listening_port = ui->sbListenPort->value();
    sc.transport_kind = getCurKind();

    return sc;
}

void EasyDDSTest::addText(const QString &text)
{
    QTextDocument* doc = ui->textEdit->document();
    QTextCursor cursor = ui->textEdit->textCursor();
    Qt::TextFormat format = Qt::AutoText;

    QTextCursor tmp(doc);
    tmp.beginEditBlock();
    tmp.movePosition(QTextCursor::End);

    if (!doc->isEmpty())
        tmp.insertBlock(cursor.blockFormat(), cursor.charFormat());
    else
        tmp.setCharFormat(cursor.charFormat());

    // 删除默认的换行
    tmp.movePosition(QTextCursor::End);
    tmp.deletePreviousChar();

    // preserve the char format
    QTextCharFormat oldCharFormat = cursor.charFormat();

#ifndef QT_NO_TEXTHTMLPARSER
    if (format == Qt::RichText || (format == Qt::AutoText && Qt::mightBeRichText(text))) {
        tmp.insertHtml(text);
    } else {
        tmp.insertText(text);
    }
#else
    Q_UNUSED(format);
    tmp.insertText(text);
#endif // QT_NO_TEXTHTMLPARSER
    if (!cursor.hasSelection())
        cursor.setCharFormat(oldCharFormat);

    tmp.endEditBlock();

    // 添加文本后移动光标至末尾
    ui->textEdit->moveCursor(QTextCursor::End);
}

void EasyDDSTest::setWidgetsVisible(const QVector<QWidget *> widgets, bool visible)
{
    for(QWidget* widget : widgets)
    {
        widget->setVisible(visible);
    }
}

QPair<QString, QString> EasyDDSTest::splitGUID(QString guid)
{
    QStringList entityList = guid.split("|");
    if(entityList.size() == 2)
    {
        QStringList guidList = entityList.first().split(".");
        if(guidList.size() == 12)
        {
            QStringList hostAndProcess = guidList.mid(0, 8);
            QStringList uniID = guidList.mid(8);
            return QPair<QString, QString>(hostAndProcess.join("."), uniID.join("."));
        }
        else
        {
            qDebug() << "GUID ID error. GUID = " << guid;
        }
    }
    else
    {
        qDebug() << "GUID Entity error. GUID = " << guid;
    }

    return QPair<QString, QString>();
}

void EasyDDSTest::sendText(const std::shared_ptr<easyddsClientPublisherApp> &app, QString topicName,
                           int frequency, const QString& source)
{
    std::thread t([=]() {
        int sampleCount = 0;
        while(!app->getIsStopped())
        {
            try
            {
                if(app->send(source.toStdString()))
                {
                    std::string printInfo = "Send [topic: " + topicName.toStdString() + "] Sample: "
                            + std::to_string(sampleCount++) + "  \n";
                    std::cout << printInfo;
                }

                QApplication::processEvents(QEventLoop::AllEvents, frequency);
                std::this_thread::sleep_for(std::chrono::milliseconds(frequency));
            }
            catch (std::exception e)
            {
                std::cout << "Send Data Error: " << e.what();
                break;
            }
        }
    });
    t.detach();

}

void EasyDDSTest::printRecvMsg(std::string message)
{
    std::cout << message;
}

void EasyDDSTest::on_pushButton_2_clicked()
{
    multiOp("start");
}

void EasyDDSTest::on_pushButton_3_clicked()
{
    multiOp("stop");
}

void EasyDDSTest::on_pushButton_4_clicked()
{
    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    QString file = QFileDialog::getOpenFileName(this, "Source", homePath, "*.txt");
    if(!file.isEmpty())
    {
        ui->lineEdit_2->setText(file);

        QFile f(file);
        if(f.open(QFile::ReadWrite))
        {
            try
            {
                m_source = f.readAll();
                ui->label_6->setText(QString("发送数据(size=%1)").arg(m_source.size()));
            }
            catch (std::exception e)
            {
                std::cout << "Read File " << file.toStdString() << " error: " << e.what();
            }

            f.close();
        }
        else
        {
            qDebug() << "Read File " << file << " error.";
        }
    }
}

void EasyDDSTest::on_ckb_all_toggled(bool checked)
{
    ui->wgt_monitor->setEnabled(checked);
    ui->pushButton_5->setEnabled(checked);
    ui->pushButton_6->setEnabled(checked);
}

void EasyDDSTest::on_pushButton_5_clicked()
{
    auto children = ui->wgt_monitor->findChildren<QCheckBox*>();
    for(auto ckb : children)
    {
        ckb->setChecked(true);
    }
}

void EasyDDSTest::on_pushButton_6_clicked()
{
    auto children = ui->wgt_monitor->findChildren<QCheckBox*>();
    for(auto ckb : children)
    {
        ckb->setChecked(false);
    }
}

void EasyDDSTest::on_radioButton_2_toggled(bool checked)
{
    QStringList serverList{"server"};
    QStringList deaultList{"DEFAULT","UDPv4","UDPv6","TCPv4",
                           "TCPv6","SHM","DATA_SHARING","LARGE_DATA"};
    QStringList discoverList{"UDPv4","UDPv6","TCPv4",
                             "TCPv6","SHM"};
    QString curTransKind = ui->cmbTransKind->currentText();

    std::function<void(QStringList, QString)> changeList =
            [=](QStringList list, QString curText)
    {
        ui->cmbTransKind->blockSignals(true);
        ui->cmbTransKind->clear();
        ui->cmbTransKind->addItems(list);
        if(list.contains(curText))
        {
            ui->cmbTransKind->setCurrentText(curText);
        }
        ui->cmbTransKind->blockSignals(false);
        ui->cmbTransKind->currentTextChanged(ui->cmbTransKind->currentText());
    };

    if(checked)
    {
        ui->comboBox->addItems(serverList);
        changeList(discoverList, curTransKind);
    }
    else
    {
        for(int i = 0; i < serverList.size(); ++i)
        {
            int serverIndex = ui->comboBox->findText("server");
            if(serverIndex != -1)
            {
                ui->comboBox->removeItem(serverIndex);
            }
        }
        initInvisible();
        changeList(deaultList, curTransKind);
    }
}

void EasyDDSTest::on_comboBox_currentTextChanged(const QString &arg1)
{
    bool vis = arg1.contains("publisher") ? true : false;
    setWidgetsVisible({ui->sbFrequency, ui->lineEdit_2, ui->pushButton_4,
                       ui->label_5, ui->label_6}, vis);

    bool isServer = arg1.contains("server");
    setWidgetsVisible({ui->lblTopic, ui->leTopic, ui->lblNum, ui->sbNum},
                      !isServer);

//    bool isMonitor = arg1.contains("monitor");
//    setWidgetsVisible({ui->cmbTopic}, isMonitor);
//    setWidgetsVisible({ui->leTopic, ui->lblQos, ui->pbQos,
//                      ui->lblTransKind, ui->cmbTransKind,
//                      ui->lblNum, ui->sbNum}, !isMonitor);

    if(ui->radioButton_2->isChecked())
    {
        setWidgetsVisible({ui->lblIP, ui->leIP, ui->lblPort, ui->sbPort},
                          !isServer);
        setWidgetsVisible({ui->lblListenIP, ui->sbListenPort,
                           ui->lblListenPort, ui->leListenIP,
                           ui->lblTimeout, ui->sbTimeout}, isServer);
    }
}

void EasyDDSTest::initInvisible()
{
    QVector<QWidget*> invisibles{ui->lblIP, ui->leIP,
                ui->lblPort, ui->sbPort,
                ui->lblListenIP, ui->sbListenPort,
                ui->lblListenPort, ui->leListenIP,
                ui->lblTimeout, ui->sbTimeout,
                                ui->cmbTopic};

    setWidgetsVisible(invisibles, false);
}

void EasyDDSTest::on_pbQos_clicked()
{
    QosDialog dialog(m_qosProfile, this);
    if(dialog.exec())
    {
        m_qosProfile = dialog.getQoSProfile();
    }
}
