#ifndef INSTALLERPROMPT_H
#define INSTALLERPROMPT_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class InstallerPrompt; }
QT_END_NAMESPACE

class InstallerPrompt : public QMainWindow
{
    Q_OBJECT

public:
    InstallerPrompt(QWidget *parent = nullptr);
    ~InstallerPrompt();

public slots:
    void tryLubuntu();

private:
    Ui::InstallerPrompt *ui;
};
#endif
