#include "easyddstest.h"
#include "ui_easyddstest.h"

#include <QDebug>
#include <QMessageBox>
#include <QFormLayout>
#include <QTimer>
#include <QFileDialog>
#include <QTextStream>
#include <QEventLoop>
#include <thread>
#include <iostream>

EasyDDSTest::EasyDDSTest(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::EasyDDSTest)
{
    ui->setupUi(this);

    ui->lblIP->setVisible(false);
    ui->lblPort->setVisible(false);
    ui->leIP->setVisible(false);
    ui->sbPort->setVisible(false);

    buffer = std::make_shared<qtStreamBuf>(this);
    new (&std::cout) std::ostream(buffer.get());

    QStringList labels({"主题", "状态", "频率", "源大小"});
    ui->tableWidget->setColumnCount(labels.size());
    ui->tableWidget->setHorizontalHeaderLabels(labels);

    ui->tableWidget->verticalHeader()->setVisible(false);

    qRegisterMetaType<std::shared_ptr<easyddsPublisherApp>>("std::shared_ptr<easyddsPublisherApp>");
    qRegisterMetaType<std::string>("std::string");
    connect(this, &EasyDDSTest::setCoutText, this, &EasyDDSTest::addText);
    connect(this, &EasyDDSTest::sendTextSignal, this, & EasyDDSTest::sendText, Qt::QueuedConnection);
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
    int num = ui->spinBox->value();
    m_kindName = ui->comboBox->currentText().toStdString();

    if("publisher" == m_kindName)
    {
        if(num == 1)
        {
            createPubliserApp(0, ui->lineEdit->text(), ui->spinBox_2->value(), m_source);
        }
        else
        {
            for(int i = 0; i < num; ++i)
            {
                QString topicName = QString("%1_%2").arg(ui->lineEdit->text()).arg(i);
                createPubliserApp(0, topicName, ui->spinBox_2->value(), m_source);
            }
        }
    }
    else if("subscriber" == m_kindName)
    {
        if(num == 1)
        {
            createSubscriberApp(0, ui->lineEdit->text());
        }
        else
        {
            for(int i = 0; i < num; ++i)
            {
                QString topicName = QString("%1_%2").arg(ui->lineEdit->text()).arg(i);
                createSubscriberApp(0, topicName);
            }
        }
    }
}

void EasyDDSTest::addInfoTab(const QString &topicName, int frequency, std::string source)
{
    int rowNum = ui->tableWidget->rowCount();

    QLabel* lblTopic = new QLabel(topicName, ui->tableWidget);
    QLabel* lblFrequency = new QLabel(QString::number(frequency), ui->tableWidget);
    QLabel* lblSize = new QLabel(QString::number(source.size()), ui->tableWidget);

    QPushButton* btn = new QPushButton("stop", ui->tableWidget);

    connect(btn, &QPushButton::clicked, ui->tableWidget, [=]{
        QString btnText = btn->text();
        int curRow = getWidgetRow(lblTopic, 0);
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
                if("publisher" == m_kindName)
                {
                    createPubliserApp(0, topicName, ui->spinBox_2->value(), m_source, curRow, false);
                }
                else
                {
                    createSubscriberApp(0, topicName, curRow, false);
                }
            }
        }
    });

    ui->tableWidget->setRowCount(rowNum + 1);
    ui->tableWidget->setCellWidget(rowNum, 0, lblTopic);
    ui->tableWidget->setCellWidget(rowNum, 1, btn);
    ui->tableWidget->setCellWidget(rowNum, 2, lblFrequency);
    ui->tableWidget->setCellWidget(rowNum, 3, lblSize);
    ui->tableWidget->setItem(rowNum, 0, new QTableWidgetItem());
    ui->tableWidget->setItem(rowNum, 1, new QTableWidgetItem());
    ui->tableWidget->setItem(rowNum, 2, new QTableWidgetItem());
    ui->tableWidget->setItem(rowNum, 3, new QTableWidgetItem());
}

std::shared_ptr<easyddsPublisherApp> EasyDDSTest::createPubliserApp(int domain_id, const QString &topicName, int frequency, std::string source, int curRow, bool addRow)
{
    std::shared_ptr<easyddsPublisherApp> app = nullptr;
    if(!topicName.isEmpty())
    {

        app = easyddsApplication::createPublisher(topicName.toStdString()
                                                  , domain_id
                                                  , qos_profile_default
                                                  , ui->ckb_all->isChecked()
                                                  , static_cast<MONITOR_TOPIC::monitorItems>(insertMonitorQos()));

        std::thread thread(&easyddsApplication::run, app);
        thread.detach();

        std::cout << topicName.toStdString() << "'s publisher running. Please press stop Button to stop the publisher at any time." << std::endl;

        if(addRow)
        {
            QString newName = QString("%1_%2").arg("publisher").arg(m_index++);
            addInfoTab(topicName, frequency, source);
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
                qDebug() << "wrong curRow for info buffer.(curRow = " << curRow << ", bufferSize = " << m_pubInfos.size();
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
    return app;
}

std::shared_ptr<easyddsSubscriberApp> EasyDDSTest::createSubscriberApp(int domain_id, const QString &topicName, int curRow, bool addRow)
{

    std::shared_ptr<easyddsSubscriberApp> app = nullptr;
    if(!topicName.isEmpty())
    {
        app = easyddsApplication::createSubscriber(topicName.toStdString()
                                                   , domain_id
                                                   , qos_profile_default
                                                   , ui->ckb_all->isChecked()
                                                   , static_cast<MONITOR_TOPIC::monitorItems>(insertMonitorQos()));

        std::thread thread(&easyddsApplication::run, app);
        thread.detach();

        std::cout << topicName.toStdString() << "'s subscriber running. Please press stop Button to stop the subscriber at any time." << std::endl;

        if(addRow)
        {
            QString newName = QString("%1_%2").arg("subscriber").arg(m_index++);
            addInfoTab(topicName);
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
                qDebug() << "wrong curRow for info buffer.(curRow = " << curRow << ", bufferSize = " << m_subInfos.size();
            }
        }

        ui->comboBox->setEnabled(false);
    }
    else
    {
        QMessageBox::warning(this, "Topic Error", "Publisher Topic can't be empty.");
    }

    app->onMessageReceived(printRecvMsg);
    return app;
}

void EasyDDSTest::stopApp(int curRow)
{
    if("publisher" == m_kindName)
    {
        if(curRow < m_pubInfos.size())
        {
            m_pubInfos[curRow]->stop();
            m_pubInfos[curRow] = nullptr;
            //        m_infos.remove(curRow);
        }
    }
    else
    {
        if(curRow < m_subInfos.size())
        {
            m_subInfos[curRow]->stop();
            m_subInfos[curRow] = nullptr;
            //        m_infos.remove(curRow);
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
    //    qDebug() << "items size = " << items.size();
    QSet<int> selectedRows;
    for(auto item : items)
    {
        selectedRows.insert(item->row());
    }
    //    qDebug() << selectedRows;
    for(int row : selectedRows)
    {
        QPushButton* btn = qobject_cast<QPushButton*>(ui->tableWidget->cellWidget(row, 1));
        if(btn && btn->text() == op)
        {
            btn->clicked();
        }
        else
        {
            qDebug() << "do not find btn.";
        }
    }
}

uint64_t EasyDDSTest::insertMonitorQos()
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
                monitorItems |= MONITOR_TOPIC::GAP_COUNT_TOPIC;
            }
            else if("RTPS_LOST" == text)
            {
                monitorItems |= MONITOR_TOPIC::RTPS_LOST_TOPIC;
            }
            else if("RTPS_SENT" == text)
            {
                monitorItems |= MONITOR_TOPIC::RTPS_SENT_TOPIC;
            }
            else if("DATA_COUNT" == text)
            {
                monitorItems |= MONITOR_TOPIC::DATA_COUNT_TOPIC;
            }
            else if("EDP_PACKETS" == text)
            {
                monitorItems |= MONITOR_TOPIC::EDP_PACKETS_TOPIC;
            }
            else if("PDP_PACKETS" == text)
            {
                monitorItems |= MONITOR_TOPIC::PDP_PACKETS_TOPIC;
            }
            else if("RESENT_DATAS" == text)
            {
                monitorItems |= MONITOR_TOPIC::RESENT_DATAS_TOPIC;
            }
            else if("SAMPLE_DATAS" == text)
            {
                monitorItems |= MONITOR_TOPIC::SAMPLE_DATAS_TOPIC;
            }
            else if("ACKNACK_COUNT" == text)
            {
                monitorItems |= MONITOR_TOPIC::ACKNACK_COUNT_TOPIC;
            }
            else if("PHYSICAL_DATA" == text)
            {
                monitorItems |= MONITOR_TOPIC::PHYSICAL_DATA_TOPIC;
            }
            else if("NACKFRAG_COUNT" == text)
            {
                monitorItems |= MONITOR_TOPIC::NACKFRAG_COUNT_TOPIC;
            }
            else if("HEARTBEAT_COUNT" == text)
            {
                monitorItems |= MONITOR_TOPIC::HEARTBEAT_COUNT_TOPIC;
            }
            else if("HISTORY_LATENCY" == text)
            {
                monitorItems |= MONITOR_TOPIC::HISTORY_LATENCY_TOPIC;
            }
            else if("NETWORK_LATENCY" == text)
            {
                monitorItems |= MONITOR_TOPIC::NETWORK_LATENCY_TOPIC;
            }
            else if("PUB_THROUGHPUT" == text)
            {
                monitorItems |= MONITOR_TOPIC::PUBLICATION_THROUGHPUT_TOPIC;
            }
            else if("SUB_THROUGHPUT" == text)
            {
                monitorItems |= MONITOR_TOPIC::SUBSCRIPTION_THROUGHPUT_TOPIC;
            }
        }
    }
    return monitorItems;
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

void EasyDDSTest::sendText(const std::shared_ptr<easyddsPublisherApp> &app, QString topicName, int frequency, std::string source)
{
    std::thread t([=]() {
        int sampleCount = 0;
        while(!app->getIsStopped())
        {
            app->send(source);
            std::string printInfo = "Send [topic: " + topicName.toStdString() + "] Sample: " + std::to_string(sampleCount++) + "  \n";
            std::cout << printInfo;
            QApplication::processEvents(QEventLoop::AllEvents, frequency);
            std::this_thread::sleep_for(std::chrono::milliseconds(frequency));
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
    QString file = QFileDialog::getOpenFileName(this, "Source", "", "*.txt");
    if(!file.isEmpty())
    {
        ui->lineEdit_2->setText(file);

        QFile f(file);
        if(f.open(QFile::ReadOnly))
        {
            m_source = QString(f.readAll()).toStdString();
            // std::cout << "source is " << m_source << std::endl;
            ui->label_8->setText(QString::number(m_source.size()));
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

void EasyDDSTest::on_comboBox_currentIndexChanged(int index)
{
    bool enable = index ==0 ? true : false;
    ui->spinBox_2->setEnabled(enable);
    ui->lineEdit_2->setEnabled(enable);
    ui->pushButton_4->setEnabled(enable);
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

}
