#include "installerprompt.h"
#include <QApplication>
#include <QScreen>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    QList<InstallerPrompt*> ws;

    // Iterate through all available screens
    for (QScreen *screen : QApplication::screens()) {
        InstallerPrompt *w = new InstallerPrompt();
        w->setGeometry(screen->geometry());
        w->show();
        ws.append(w);
    }

    for (InstallerPrompt *w : ws) {
        for (InstallerPrompt *otherWindow : ws) {
            if (w != otherWindow) {
                // Connect signals and slots for synchronization
                // Example: connect(ws.last(), &InstallerPrompt::someSignal, otherWindow, &InstallerPrompt::someSlot);
            }
        }
    }

    return app.exec();
}
