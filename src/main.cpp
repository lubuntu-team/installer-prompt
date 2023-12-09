/*
 * Copyright (C) 2022-2023 Lubuntu Developers <lubuntu-devel@lists.ubuntu.com>
 * Authored by: Simon Quigley <tsimonq2@lubuntu.me>
 *              Aaron Rainbolt <arraybolt3@lubuntu.me>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

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
