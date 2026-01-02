#include "connectwindow.h"
#include <QApplication>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 加载样式表
    QFile styleFile(":/style.qss");  // 如果放在资源文件中

    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        a.setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qDebug() << "无法加载样式表文件";
    }

    ConnectWindow w;
    w.show();
    return a.exec();
}
