#ifndef EASYDDSTEST_H
#define EASYDDSTEST_H

#include <QMainWindow>
#include <QMap>
#include "easyddsApplication.hpp"
#include "qtstreambuf.h"
#include "easyddsPublisherApp.hpp"
#include "easyddsSubscriberApp.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class EasyDDSTest; }
QT_END_NAMESPACE

class EasyDDSTest : public QMainWindow
{
    Q_OBJECT

public:
    EasyDDSTest(QWidget *parent = nullptr);
    ~EasyDDSTest();

    void Append(const QString &text);

signals:
    void setCoutText(const QString& text);
    void sendTextSignal(const std::shared_ptr<easyddsPublisherApp> &app , QString topicName, int frequency, std::string source);
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_ckb_all_toggled(bool checked);

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_radioButton_2_toggled(bool checked);

    void on_comboBox_currentTextChanged(const QString &arg1);

private:
    void initInvisible();

    void addInfoTab(const QString& topicName, int frequency = 500, std::string source = "");

    std::shared_ptr<easyddsPublisherApp> createPubliserApp(int domain_id, const QString& topicName,
                                                           int frequency = 500, std::string source = "",
                                                           int curRow = 0,
                                                           bool addRow = true);
    std::shared_ptr<easyddsSubscriberApp> createSubscriberApp(int domain_id, const QString& topicName,
                                                              int curRow = 0,
                                                              bool addRow = true);
    void stopApp(int curRow);

    int getWidgetRow(QWidget* widget, int column);

    void multiOp(const QString& op);
    MONITOR_TOPIC::monitorItems insertMonitorQos();

    void addText(const QString& text);

    void setWidgetsVisible(const QVector<QWidget*> widgets, bool visible = true);
public slots:
    void sendText(const std::shared_ptr<easyddsPublisherApp> &app, QString topicName, int frequency = 500, std::string source = "");

   static void printRecvMsg(std::string message);
private:
    Ui::EasyDDSTest *ui;

    int m_index = 0;
    QVector< std::shared_ptr<easyddsPublisherApp> > m_pubInfos;
    QVector< std::shared_ptr<easyddsSubscriberApp> > m_subInfos;
    std::shared_ptr<qtStreamBuf> buffer;

    std::string m_source;

    std::string m_monitorTopic;
    std::string m_kindName;

    //    TextEditStreamBuf streamBuffer;
};
#endif // EASYDDSTEST_H
