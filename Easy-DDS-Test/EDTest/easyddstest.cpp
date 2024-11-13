#include "easyddstest.h"
#include "ui_easyddstest.h"

#include <QDebug>
#include <QMessageBox>
#include <QFormLayout>
#include <QTimer>
#include <QFileDialog>
#include <QTextStream>
#include <thread>
#include <iostream>

EasyDDSTest::EasyDDSTest(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::EasyDDSTest)
{
    ui->setupUi(this);

    buffer = std::make_shared<qtStreamBuf>(this);
    new (&std::cout) std::ostream(buffer.get());

    QStringList labels({"topic", "status", "frequency", "source size"});
    ui->tableWidget->setColumnCount(labels.size());
    ui->tableWidget->setHorizontalHeaderLabels(labels);
    //    ui->tableWidget->horizontalHeader()->setVisible(false);
    ui->tableWidget->verticalHeader()->setVisible(false);

    connect(this, &EasyDDSTest::setCoutText, this, [=](const QString& text){
        ui->textEdit->setText(text);
        ui->textEdit->moveCursor(QTextCursor::End);
    });
}

EasyDDSTest::~EasyDDSTest()
{
    delete ui;
}

void EasyDDSTest::Append(const QString &text)
{
    static QString textbuf;
    textbuf+=text;

    emit setCoutText(textbuf);
}

void EasyDDSTest::on_pushButton_clicked()
{
    int num = ui->spinBox->value();
    if(num == 1)
    {
        createApp(0, ui->lineEdit->text(), ui->comboBox->currentText(), ui->spinBox_2->value(), m_source);
    }
    else
    {
        for(int i = 0; i < num; ++i)
        {
            QString topicName = QString("%1_%2").arg(ui->lineEdit->text()).arg(i);
            createApp(0, topicName, ui->comboBox->currentText(), ui->spinBox_2->value(), m_source);
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
                createApp(0, topicName, ui->comboBox->currentText(),
                          frequency, source, curRow, false);
                //                m_infos[curRow]->run();
                btn->setText("stop");
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

std::shared_ptr<easyddsApplication> EasyDDSTest::createApp(int domain_id, const QString &topicName,
                                                           const QString &kindName,
                                                           int frequency, std::string source,
                                                           int curRow, bool addRow)
{
    std::shared_ptr<easyddsApplication> app = nullptr;

    if(!topicName.isEmpty())
    {
        app = easyddsApplication::make_app(domain_id, topicName.toStdString(), kindName.toStdString(), frequency, source);
        std::thread thread(&easyddsApplication::run, app);

        thread.detach();

        std::cout << topicName.toStdString() << "'s " << kindName.toStdString() << " running. Please press stop Button to stop the "
                  << kindName.toStdString() << " at any time." << std::endl;

        if(addRow)
        {
            QString newName = QString("%1_%2").arg(kindName).arg(m_index++);
            addInfoTab(topicName, frequency, source);
            m_infos.append(app);
        }
        else
        {
            if(curRow < m_infos.size())
            {
                m_infos[curRow] = app;
            }
            else
            {
                qDebug() << "wrong curRow for info buffer.(curRow = " << curRow << ", bufferSize = " << m_infos.size();
            }
        }

        ui->comboBox->setEnabled(false);
    }
    else
    {
        QMessageBox::warning(this, "Topic Error", "Publisher Topic can't be empty.");
    }

    return app;
}

void EasyDDSTest::stopApp(int curRow)
{
    if(curRow < m_infos.size())
    {
        m_infos[curRow]->stop();
        m_infos[curRow] = nullptr;
        //        m_infos.remove(curRow);
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
            std::cout << "source is " << m_source << std::endl;
            ui->label_8->setText(QString::number(m_source.size()));
        }
        else
        {
            qDebug() << "Read File " << file << " error.";
        }
    }
}
