QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    easyddstest.cpp \
    qosdialog.cpp \
    qtstreambuf.cpp

HEADERS += \
    easyddstest.h \
    qosdialog.h \
    qtstreambuf.h

FORMS += \
    easyddstest.ui \
    qosdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../easydds/build/release/ -lEasyDDS
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../easydds/build/debug/ -lEasyDDS
else:unix: LIBS += -L$$PWD/../../easydds/build/ -lEasyDDS

INCLUDEPATH += $$PWD/../../easydds/include
DEPENDPATH += $$PWD/../../easydds/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../install/lib/release/ -lfastcdr
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../install/lib/debug/ -lfastcdr
else:unix: LIBS += -L$$PWD/../../install/lib/ -lfastcdr

INCLUDEPATH += $$PWD/../../install/include
DEPENDPATH += $$PWD/../../install/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../install/lib/release/ -lfastdds
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../install/lib/debug/ -lfastdds
else:unix: LIBS += -L$$PWD/../../install/lib/ -lfastdds

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../install/lib/release/ -lfoonathan_memory-0.7.3
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../install/lib/debug/ -lfoonathan_memory-0.7.3
else:unix: LIBS += -L$$PPWD/../../install/lib/ -lfoonathan_memory-0.7.3

RESOURCES += \
    icon.qrc
