#include "nappwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QStringLiteral("NApp"));

    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList iconCandidates = {
        QDir(appDir).filePath(QStringLiteral("images/app.png")),
        QDir(appDir).filePath(QStringLiteral("../images/app.png")),
        QDir(appDir).filePath(QStringLiteral("../../images/app.png"))
    };
    QIcon appIcon;
    for (const QString &iconPath : iconCandidates) {
        if (QFile::exists(iconPath)) {
            appIcon = QIcon(iconPath);
            break;
        }
    }
    if (!appIcon.isNull()) {
        a.setWindowIcon(appIcon);
    }

    NAppWindow w;
    if (!appIcon.isNull()) {
        w.setWindowIcon(appIcon);
    }
    w.resize(960, 600);
    w.show();
    return a.exec();
}
