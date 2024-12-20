#include "easyddstest.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString type = "publisher";
    if(argc > 1)
    {
        type = argv[1];
    }
    EasyDDSTest w(type);
    w.show();
    return a.exec();
}
