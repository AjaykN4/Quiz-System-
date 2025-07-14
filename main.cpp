#include <QApplication>
#include <QStyle>
#include <QScreen>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Quiz System");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Quiz System");
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
} 