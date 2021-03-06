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
    bg = bg.scaled(width, height, Qt::KeepAspectRatio);

    QPalette palette;
    palette.setBrush(QPalette::Window, bg);
    this->setPalette(palette);

    // Resize the layout widget to the screen size.
    ui->gridLayoutWidget->resize(width, height);

    // Set the button colors
    QString css = "background-color: rgba(0, 104, 200, 100); color: white; border-radius: 15px;";
    ui->tryLubuntu->setAttribute(Qt::WA_TranslucentBackground);
    ui->tryLubuntu->setStyleSheet(css);
    ui->installLubuntu->setAttribute(Qt::WA_TranslucentBackground);
    ui->installLubuntu->setStyleSheet(css);

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
    QProcess *calamares = new QProcess(this);
    calamares->start("/usr/bin/lubuntu-installer");
}

InstallerPrompt::~InstallerPrompt()
{
    delete ui;
}
