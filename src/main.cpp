#include "installerprompt.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    InstallerPrompt w;
    w.showFullScreen();
    return a.exec();
}
