#include "easyddstest.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EasyDDSTest w;
    w.show();
    return a.exec();
}
