#include <QProcess>
#include <QScreen>
#include "installerprompt.h"
#include "./ui_installerprompt.h"

InstallerPrompt::InstallerPrompt(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::InstallerPrompt)
{
    ui->setupUi(this);

    // Set the background image and scale it
    QPixmap bg("/usr/share/lubuntu/installer-prompt/background.png");
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    int height = screenGeometry.height();
    int width = screenGeometry.width();
    bg = bg.scaled(width, height, Qt::IgnoreAspectRatio);

    QPalette palette;
    palette.setBrush(QPalette::Window, bg);
    this->setPalette(palette);

    // Resize the layout widget to the screen size.
    ui->gridLayoutWidget->resize(width, height);

    // Set the buttons to be translucent
    ui->tryLubuntu->setAttribute(Qt::WA_TranslucentBackground);
    ui->installLubuntu->setAttribute(Qt::WA_TranslucentBackground);

    // Slots and signals
    connect(ui->tryLubuntu, &QAbstractButton::clicked, this, &InstallerPrompt::tryLubuntu);
    connect(ui->installLubuntu, &QAbstractButton::clicked, this, &InstallerPrompt::installLubuntu);
}

void InstallerPrompt::tryLubuntu()
{
    QApplication::quit();
}

void InstallerPrompt::installLubuntu()
{
    ui->tryLubuntu->setVisible(false);
    ui->installLubuntu->setVisible(false);
    QProcess *calamares = new QProcess(this);
    QStringList args;
    calamares->start("/usr/libexec/lubuntu-installer", args);

    // If Calamares exits, it either crashed or the user cancelled the installation. Exit the installer prompt (and start LXQt).
    connect(calamares, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
        this, [this](int, QProcess::ExitStatus){ this->tryLubuntu(); });
}

InstallerPrompt::~InstallerPrompt()
{
    delete ui;
}
