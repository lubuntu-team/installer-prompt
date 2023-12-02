#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessNetwork>
#include <QProcess>
#include <QScreen>
#include <QMessageBox>
#include <QLineEdit>
#include "installerprompt.h"
#include "./ui_installerprompt.h"

InstallerPrompt::InstallerPrompt(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::InstallerPrompt) {
    ui->setupUi(this);

    // Set the background image and scale it
    QPixmap bg("/usr/share/lubuntu/installer-prompt/background.png");
    if (bg.isNull()) {
        QMessageBox::warning(this, tr("Error"), tr("Background image cannot be loaded."));
        return;
    }

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    bg = bg.scaled(screenGeometry.size(), Qt::IgnoreAspectRatio);
    
    QPalette palette;
    palette.setBrush(QPalette::Window, bg);
    this->setPalette(palette);

    // Resize the layout widget to the screen size
    ui->gridLayoutWidget->resize(screenGeometry.size());

    // Initialize process for external app launch
    process = new QProcess(this);

    // Set up signal-slot connections for buttons
    connect(ui->tryLubuntu, &QAbstractButton::clicked, this, &InstallerPrompt::tryLubuntu);
    connect(ui->installLubuntu, &QAbstractButton::clicked, this, &InstallerPrompt::installLubuntu);

    // Set up the language combo box with available languages
    initLanguageComboBox();

    // Check initial network status and update UI
    updateConnectionStatus(checkInternetConnection());

    // Set up network manager signals for dynamic updates
    auto nm = NetworkManager::notifier();
    connect(nm, &NetworkManager::Notifier::deviceAdded, this, &InstallerPrompt::refreshNetworkList);
    connect(nm, &NetworkManager::Notifier::deviceRemoved, this, &InstallerPrompt::refreshNetworkList);
    connect(nm, &NetworkManager::Notifier::networkingEnabledChanged, this, &InstallerPrompt::refreshNetworkList);
}

bool InstallerPrompt::checkInternetConnection() {
    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
        if (device->type() == NetworkManager::Device::Wifi) {
            auto wifiDevice = device.staticCast<NetworkManager::WirelessDevice>();
            if (!wifiDevice->isActive()) {
                showWifiOptions();
            }
            return wifiDevice->isActive();
        }
    }
    return false;
}

void InstallerPrompt::updateConnectionStatus(bool online) {
    if (online) {
        ui->connectionStatusLabel->setText(tr("Connected to the internet"));
        ui->connectWifiButton->setVisible(false);
    } else {
        ui->connectionStatusLabel->setText(tr("Not connected to the internet"));
        ui->connectWifiButton->setVisible(true);
    }
}

void InstallerPrompt::onConnectWifiClicked() {
    QDialog *passwordDialog = new QDialog(this, Qt::Window | Qt::WindowStaysOnTopHint);
    QVBoxLayout *layout = new QVBoxLayout(passwordDialog);
    
    QLabel *label = new QLabel(tr("Enter Wi-Fi Password:"), passwordDialog);
    QLineEdit *lineEdit = new QLineEdit(passwordDialog);
    lineEdit->setEchoMode(QLineEdit::Password);
    QPushButton *button = new QPushButton(tr("Connect"), passwordDialog);
    
    layout->addWidget(label);
    layout->addWidget(lineEdit);
    layout->addWidget(button);
    
    passwordDialog->setLayout(layout);
    
    connect(button, &QPushButton::clicked, this, [this, lineEdit, passwordDialog]() {
        QString password = lineEdit->text();
        // Use the password to connect to the selected Wi-Fi network
        // Make sure to handle the password securely and do not store it in plain text
        passwordDialog->accept();
    });
    
    passwordDialog->exec();
}

void InstallerPrompt::showWifiOptions() {
    bool foundWifiDevice = false;
    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
        if (device->type() == NetworkManager::Device::Wifi) {
            foundWifiDevice = true;
            auto wifiDevice = device.staticCast<NetworkManager::WirelessDevice>();
            ui->networkComboBox->clear();  // Use the combo box from the UI file, clear existing items
            for (const NetworkManager::WirelessNetwork::Ptr &network : wifiDevice->networks()) {
                ui->networkComboBox->addItem(network->ssid());  // Add Wi-Fi networks to the combo box
            }
            break;  // Handle the first Wi-Fi device
        }
    }

    if (!foundWifiDevice) {
        QMessageBox::information(this, tr("WiFi Not Available"), tr("No WiFi devices were found on this system."));
    }
}

void InstallerPrompt::refreshNetworkList() {
    NetworkManager::WirelessDevice::Ptr wirelessDevice;

    // Iterate over network interfaces to find a wireless device
    const auto devices = NetworkManager::networkInterfaces();
    for (const auto &device : devices) {
        if (device->type() == NetworkManager::Device::Wifi) {
            wirelessDevice = device.staticCast<NetworkManager::WirelessDevice>();
            break; // Break after finding the first wireless device
        }
    }

    if (!wirelessDevice) {
        // No wireless device found, handle appropriately
        ui->networkComboBox->setVisible(false);
        connectWifiButton->setVisible(false);
        return;
    }

    // Get the list of available networks
    const auto networks = wirelessDevice->networks();
    ui->networkComboBox->clear();
    for (const auto &network : networks) {
        ui->networkComboBox->addItem(network->ssid());
    }

    // Adjust visibility based on whether any networks are found
    ui->networkComboBox->setVisible(!networks.isEmpty());
    connectWifiButton->setVisible(!networks.isEmpty());
}

void InstallerPrompt::initLanguageComboBox() {
    // This should populate the language combo box from the UI file, not create a new one
    QStringList languages = getAvailableLanguages();
    ui->languageComboBox->addItems(languages);  // Add items to the combo box

    int defaultIndex = ui->languageComboBox->findText(QLocale(QLocale::English, QLocale::UnitedStates).nativeLanguageName());
    if (defaultIndex != -1) {
        ui->languageComboBox->setCurrentIndex(defaultIndex);
    }
}

QStringList InstallerPrompt::getAvailableLanguages() const {
    QStringList languageList;
    for (int language = QLocale::Abkhazian; language <= QLocale::LastLanguage; ++language) {
        QLocale locale(static_cast<QLocale::Language>(language));
        QString languageName = locale.languageToString(locale.language());
        if (!languageName.isEmpty() && !languageList.contains(languageName)) {
            languageList.append(languageName);
        }
    }
    languageList.sort(Qt::CaseInsensitive);
    return languageList;
}

void InstallerPrompt::onLanguageChanged(int index) {
    // Placeholder for handling language change
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
