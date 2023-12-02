#include "installerprompt.h"
#include <QApplication>
#include <QScreen>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QList<InstallerPrompt*> windows;

    // Iterate through all available screens
    for (QScreen *screen : QApplication::screens()) {
        InstallerPrompt *window = new InstallerPrompt();
        window->setGeometry(screen->geometry());
        window->show();
        windows.append(window);
    }

    // Connect signals and slots to synchronize state across windows
    for (InstallerPrompt *window : windows) {
        for (InstallerPrompt *otherWindow : windows) {
            if (window != otherWindow) {
                // Connect signals and slots for synchronization
                // Example: connect(window, &InstallerPrompt::someSignal, otherWindow, &InstallerPrompt::someSlot);
            }
        }
    }

    return app.exec();
}
