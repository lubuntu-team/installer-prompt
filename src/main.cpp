#include "installerprompt.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    InstallerPrompt w;
    w.setWindowState(Qt::WindowFullScreen);
    w.show();
    return a.exec();
}
