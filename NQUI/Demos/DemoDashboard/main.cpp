#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QStringLiteral("NDashboard Demo"));
    DashboardDemoWidget w;
    w.resize(1220, 800);
    w.show();
    return a.exec();
}
