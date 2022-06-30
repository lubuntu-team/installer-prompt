#include "installerprompt.h"
#include "./ui_installerprompt.h"

InstallerPrompt::InstallerPrompt(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::InstallerPrompt)
{
    ui->setupUi(this);

    // Set the background image, and let it change with the release
    QPixmap bg("../img/background.png");
    bg = bg.scaled(this->size(), Qt::IgnoreAspectRatio);
    QPalette palette;
    palette.setBrush(QPalette::Window, bg);
    this->setPalette(palette);

    // Set the button colors
    QString css = "background-color: rgba(0, 104, 200, 100); color: white; border-radius: 15px;";
    ui->tryLubuntu->setAttribute(Qt::WA_TranslucentBackground);
    ui->tryLubuntu->setStyleSheet(css);
    ui->installLubuntu->setAttribute(Qt::WA_TranslucentBackground);
    ui->installLubuntu->setStyleSheet(css);

    // Slots and signals
    connect(ui->tryLubuntu, &QAbstractButton::clicked, this, &InstallerPrompt::tryLubuntu);
}

void InstallerPrompt::tryLubuntu()
{
    QApplication::quit();
}

InstallerPrompt::~InstallerPrompt()
{
    delete ui;
}
