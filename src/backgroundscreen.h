#ifndef BACKGROUNDSCREEN_H
#define BACKGROUNDSCREEN_H

#include <QWidget>

class BackgroundScreen : public QWidget {
    Q_OBJECT

public:
    explicit BackgroundScreen(QWidget *parent = nullptr);
    virtual ~BackgroundScreen();
};

#endif // BACKGROUNDSCREEN_H
