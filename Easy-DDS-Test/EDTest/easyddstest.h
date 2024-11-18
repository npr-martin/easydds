#ifndef EASYDDSTEST_H
#define EASYDDSTEST_H

#include <QMainWindow>
#include <QMap>
#include "easyddsApplication.hpp"
#include "qtstreambuf.h"

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

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_ckb_all_toggled(bool checked);

    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

private:
    void addInfoTab(const QString& topicName, int frequency = 500, std::string source = "");

    std::shared_ptr<easyddsApplication> createApp(int domain_id, const QString& topicName,
                                                  const QString& kindName,
                                                  int frequency = 500, std::string source = "",
                                                  int curRow = 0,
                                                  bool addRow = true);
    void stopApp(int curRow);

    int getWidgetRow(QWidget* widget, int column);

    void multiOp(const QString& op);
    void insertMonitorQos();

    void addText(const QString& text);

private:
    Ui::EasyDDSTest *ui;

    int m_index = 0;
    QVector< std::shared_ptr<easyddsApplication> > m_infos;

    std::shared_ptr<qtStreamBuf> buffer;

    std::string m_source;

    std::string m_monitorTopic;

    bool m_useCDR;

//    TextEditStreamBuf streamBuffer;
};
#endif // EASYDDSTEST_H
