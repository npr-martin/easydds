#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QVector>
#include <QPair>
#include <QSet>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using MapInfo = QPair<int, QString>;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpenLog_triggered();

    void on_lwWriter_itemSelectionChanged();

    void on_lwReader_itemSelectionChanged();

private:
    bool praseLog(const QString& fileName);
    bool praseLine(const QString& line);
    MapInfo praseSample(const QString& text);

    void updateUI();

    void removeN(QString& text);

    QStringList getSelectedWriters();
    QStringList getSelectedReaders();

    void updateSampleTable();

    bool anyInVecs(const QVector<QString>& vecBase, const QStringList& vecCheck);
    bool anyInSet(const QSet<QString>& setBase, const QStringList& vecCheck);

    QString transReader(const QStringList& list);
    QString transReader(const QString& readerID);
    QString transWriter(const QString& writerID);

private:
    Ui::MainWindow *ui;

    QMap<QString, QVector<MapInfo> > m_mapWriter2NumMsg;    ///< WriterID->SequenceNum&Message
    QMap<QString, QVector<MapInfo> > m_mapReader2Num;       ///< ReaderID->SequenceNum&WriterID

    QMap<MapInfo, QVector<QString> > m_mapNumWriter2Reader; ///< SequenceNum&WriterID->ReaderID
    QMap<MapInfo, QString> m_mapNumWriter2Msg;              ///< SequenceNum&WriterID->Message
    QMap<QString, QSet<QString> > m_mapWriter2Reader;       ///< WriterID->ReaderID

    QMap<QString, int> m_mapFileIndex;

    int m_fileLine = 0;
};
#endif // MAINWINDOW_H
