QT += core gui widgets network charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    quizwindow.cpp \
    quizmanager.cpp \
    apimanager.cpp \
    timerwidget.cpp \
    chartwidget.cpp \
    customquizmaker.cpp \
    quizquestion.cpp

HEADERS += \
    mainwindow.h \
    quizwindow.h \
    quizmanager.h \
    apimanager.h \
    timerwidget.h \
    chartwidget.h \
    customquizmaker.h \
    quizquestion.h

# FORMS removed - UI created programmatically

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# macOS specific
macx {
    ICON = app_icon.icns
    QMAKE_INFO_PLIST = Info.plist
} 