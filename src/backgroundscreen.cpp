#include "backgroundscreen.h"

#include <QMessageBox>
#include <QGuiApplication>
#include <QScreen>

BackgroundScreen::BackgroundScreen(QWidget *parent)
    : QWidget(parent) {
    // Set the background image and scale it
    QPixmap bg(":/background");
    if (bg.isNull()) {
        // the user will see the warning message that the InstallerPrompt object pops up, no need to bombard them with one message per screen
        return;
    }

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    bg = bg.scaled(screenGeometry.size(), Qt::IgnoreAspectRatio);

    QPalette palette;
    palette.setBrush(QPalette::Window, bg);
    this->setPalette(palette);
}

BackgroundScreen::~BackgroundScreen()
{
}
