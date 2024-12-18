#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFile>
#include <QDebug>
#include <QStandardPaths>
#include <algorithm>
#include <functional>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->twSample->setColumnCount(2);
    ui->twSample->setHorizontalHeaderLabels({"序列号", "？"});
    ui->twSample->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->twSample->verticalHeader()->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpenLog_triggered()
{
    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    QString fileName = QFileDialog::getOpenFileName(this, "打开监控日志", homePath, "监控日志文件 (*.txt)");
    if(!fileName.isEmpty())
    {
        if(praseLog(fileName))
        {
            updateUI();
        }
    }
}

bool MainWindow::praseLog(const QString &fileName)
{
    QFile f(fileName);
    if(f.open(QIODevice::Text | QIODevice::ReadOnly))
    {
        int fileLine = m_mapFileIndex.value(fileName, 0);

        // 跳过之前已读取的监控内容行数
        for(int i = 0; i < fileLine; ++i)
        {
            f.readLine();
        }

        while(!f.atEnd())
        {
            QString line = f.readLine();
            ++fileLine;
            praseLine(line);
        }

        m_mapFileIndex[fileName] = fileLine;

        f.close();
        return true;
    }
    return false;
}

bool MainWindow::praseLine(const QString &line)
{
    QString strBegin = "sampleIdentity = ";
    QString strReaderMid = ", readerID = ";
    QString strWriterMid = ", msg = ";
    if(line.startsWith(strBegin))
    {
        // 移除前面的 strBegin
        QString strCutBegin = line.mid(strBegin.size());
        // Reader的信息
        if(strCutBegin.contains(strReaderMid))
        {
            QStringList splitList = strCutBegin.split(strReaderMid);
            if(splitList.size() == 2)
            {
                QString strSample = splitList[0];
                QString strReader = splitList[1];
                removeN(strReader);
                // seqNum,WriterID
                MapInfo si = praseSample(strSample);
                m_mapReader2Num[strReader].append(si);
                m_mapNumWriter2Reader[si].append(strReader);
                m_mapWriter2Reader[si.second].insert(strReader);
            }
            else
            {
                qDebug() << "Prase Reader Line Error: " << strCutBegin;
                return false;
            }
        }
        else
        {
            QStringList splitList = strCutBegin.split(strWriterMid);
            if(splitList.size() == 2)
            {
                QString strSample = splitList[0];
                QString strMessage = splitList[1];
                removeN(strMessage);
                MapInfo si = praseSample(strSample);
                int seqNum = si.first;
                QString strWriter = si.second;
                m_mapWriter2NumMsg[strWriter].append(MapInfo(seqNum, strMessage));
            }
            else
            {
                qDebug() << "Prase Writer Line Error: " << strCutBegin;
                return false;
            }
        }
        return true;
    }

    return false;
}

MapInfo MainWindow::praseSample(const QString &text)
{
    QStringList list = text.split("|");
    int seqNum = list.last().toInt();
    list.removeLast();
    QString strID = list.join("|");

    return MapInfo(seqNum, strID);
}

void MainWindow::updateUI()
{
    std::function<void(QStringList, QString, QListWidget*)> updateListData
            = [=](QStringList list, QString text, QListWidget* lw){
        for(int i = 0; i < list.size(); ++i)
        {
            QListWidgetItem* item = new QListWidgetItem(text + QString::number(i));
            item->setToolTip(list[i]);
            item->setStatusTip(list[i]);
            lw->addItem(item);
        }
    };

    updateListData(m_mapWriter2NumMsg.keys(), "writer", ui->lwWriter);
    updateListData(m_mapReader2Num.keys(), "reader", ui->lwReader);
}

void MainWindow::removeN(QString& text)
{
    if(text.endsWith("\n"))
    {
        text.chop(1);
    }
}

QStringList MainWindow::getSelectedWriters()
{
    auto items = ui->lwWriter->selectedItems();
    QStringList selectedWriters;
    for(auto item : items)
    {
        selectedWriters.append(item->toolTip());
    }

    return selectedWriters;
}

QStringList MainWindow::getSelectedReaders()
{
    auto items = ui->lwReader->selectedItems();
    QStringList selectedReaders;
    for(auto item : items)
    {
        selectedReaders.append(item->toolTip());
    }

    return selectedReaders;
}

void MainWindow::updateSampleTable()
{
    QStringList selectedWriters = getSelectedWriters();
    QStringList selectedReaders = getSelectedReaders();

    QVector<MapInfo> infos;
    QVector<MapInfo> unRecvInfos;
    QVector<QStringList> readerInfos;

    bool showDetails = false;
    bool onlyReader = false;

    if(selectedReaders.isEmpty())
    {
        ui->twSample->setColumnCount(3);
        ui->twSample->setHorizontalHeaderLabels({"序列号", "信息内容", "读取者列表"});
        showDetails = true;
        for(auto key : m_mapWriter2NumMsg.keys())
        {
            if(selectedWriters.contains(key))
            {
                auto numMsgs = m_mapWriter2NumMsg.value(key);
                infos.append(numMsgs);
                for(auto info : numMsgs)
                {
                    readerInfos.append(m_mapNumWriter2Reader.value(MapInfo(info.first, key)).toList());
                }
            }
        }
    }
    else if(selectedWriters.isEmpty())
    {
        onlyReader = true;
        ui->twSample->setColumnCount(2);
        ui->twSample->setHorizontalHeaderLabels({"序列号", "写入者编号"});
        for(auto key : m_mapReader2Num.keys())
        {
            if(selectedReaders.contains(key))
            {
                infos.append(m_mapReader2Num.value(key));
            }
        }
    }
    else
    {
        ui->twSample->setColumnCount(2);
        ui->twSample->setHorizontalHeaderLabels({"序列号", "信息内容"});
        for(auto writerID : m_mapWriter2NumMsg.keys())
        {
            if(selectedWriters.contains(writerID))
            {
                QVector<MapInfo> vecWriterMsgs = m_mapWriter2NumMsg.value(writerID);
                for(MapInfo writerMsg : vecWriterMsgs)
                {
                    auto vecRecvs = m_mapNumWriter2Reader.value(MapInfo(writerMsg.first, writerID));
                    // 如果当前所选信息的接受者内包含所选项
                    if(anyInVecs(vecRecvs, selectedReaders))
                    {
                        infos.append(writerMsg);
                    }
                    // 检查Writer与Reader是否匹配
                    else if(anyInSet(m_mapWriter2Reader.value(writerID), selectedReaders))
                    {
                        unRecvInfos.append(writerMsg);
                    }
                }
            }
        }
    }


    ui->twSample->setRowCount(infos.size() + unRecvInfos.size());
    int row = 0;
    // 已接收的消息
    for(; row < infos.size(); ++row)
    {
        ui->twSample->setItem(row, 0, new QTableWidgetItem(QString::number(infos[row].first)));
        ui->twSample->setItem(row, 1, new QTableWidgetItem(onlyReader ? transWriter(infos[row].second) : infos[row].second));
        if(showDetails)
        {
            ui->twSample->setItem(row, 2, new QTableWidgetItem(transReader(readerInfos[row])));
        }
    }
    // 未接收的消息
    for(int i = 0; i < unRecvInfos.size(); ++i)
    {
        auto itemSeq = new QTableWidgetItem(QString::number(unRecvInfos[i].first));
        auto itemMsg = new QTableWidgetItem(unRecvInfos[i].second);
        itemSeq->setTextColor(Qt::gray);
        itemMsg->setTextColor(Qt::gray);
        ui->twSample->setItem(row + i, 0, itemSeq);
        ui->twSample->setItem(row + i, 1, itemMsg);
    }
}

bool MainWindow::anyInVecs(const QVector<QString> &vecBase, const QStringList &vecCheck)
{
    auto iter = std::find_if(vecBase.begin(), vecBase.end(), [=](QString info){
        return vecCheck.contains(info);
    });
    return (iter != vecBase.end());
}

bool MainWindow::anyInSet(const QSet<QString> &setBase, const QStringList &vecCheck)
{
    auto iter = std::find_if(setBase.begin(), setBase.end(), [=](QString info){
        return vecCheck.contains(info);
    });
    return (iter != setBase.end());
}

QString MainWindow::transReader(const QStringList &list)
{
    QStringList strReaderList;
    QStringList readerList = m_mapReader2Num.keys();
    for(auto readerID : list)
    {
        int index = readerList.indexOf(readerID);
        if(index != -1)
        {
            strReaderList.append("reader" + QString::number(index));
        }
        else
        {
            qDebug() << "Wrong index for readerID: " << readerID;
        }
    }

    std::sort(strReaderList.begin(), strReaderList.end());

    return strReaderList.join(", ");
}

QString MainWindow::transWriter(const QString &writerID)
{
    QStringList writerList = m_mapWriter2NumMsg.keys();
    int index = writerList.indexOf(writerID);
    if(index != -1)
    {
        return "reader" + QString::number(index);
    }
    else
    {
        qDebug() << "Wrong index for writerID: " << writerID;
    }

    return writerID;
}

void MainWindow::on_lwWriter_itemSelectionChanged()
{
    updateSampleTable();
}

void MainWindow::on_lwReader_itemSelectionChanged()
{
    updateSampleTable();
}
