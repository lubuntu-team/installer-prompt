#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Ipv6Setting>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessNetwork>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/Settings>
#include <QProcess>
#include <QScreen>
#include <QMessageBox>
#include <QUuid>
#include <QDBusPendingReply>
#include <KLed>
#include "installerprompt.h"
#include "./ui_installerprompt.h"

InstallerPrompt::InstallerPrompt(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::InstallerPrompt) {
    ui->setupUi(this);

    // Hide the Incorrect Password text
    ui->incorrectPassword->setVisible(false);

    // Set the background image and scale it
    QPixmap bg(":/background");
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

    // Initialize process for external app launch
    process = new QProcess(this);

    // Set up signal-slot connections for buttons
    connect(ui->tryLubuntu, &QAbstractButton::clicked, this, &InstallerPrompt::tryLubuntu);
    connect(ui->installLubuntu, &QAbstractButton::clicked, this, &InstallerPrompt::installLubuntu);
    connect(ui->connectWiFiButton, &QAbstractButton::clicked, this, &InstallerPrompt::onConnectWifiClicked);

    // Set up the language combo box with available languages
    initLanguageComboBox();

    // Connect the language combo box to the onLanguageChanged slot
    connect(ui->languageComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onLanguageChanged(int)));

    // Check initial network status and update UI
    updateConnectionStatus();

    // Find and store the Wi-Fi device
    foreach (const NetworkManager::Device::Ptr &device, NetworkManager::networkInterfaces()) {
        if (device->type() == NetworkManager::Device::Wifi) {
            wifiDevice = device.objectCast<NetworkManager::WirelessDevice>();
            connect(wifiDevice.data(), &NetworkManager::Device::stateChanged, this, &InstallerPrompt::handleWiFiConnectionChange);
            break;
        }
    }

    // Set up network manager signals for dynamic updates
    auto nm = NetworkManager::notifier();
    connect(nm, &NetworkManager::Notifier::deviceAdded, this, &InstallerPrompt::updateConnectionStatus);
    connect(nm, &NetworkManager::Notifier::deviceRemoved, this, &InstallerPrompt::updateConnectionStatus);
    connect(nm, &NetworkManager::Notifier::activeConnectionsChanged, this, &InstallerPrompt::updateConnectionStatus);
    connect(nm, &NetworkManager::Notifier::wirelessEnabledChanged, this, &InstallerPrompt::updateConnectionStatus);
    connect(nm, &NetworkManager::Notifier::activeConnectionAdded, this, &InstallerPrompt::updateConnectionStatus);
    connect(nm, &NetworkManager::Notifier::connectivityChanged, this, &InstallerPrompt::updateConnectionStatus);
    connect(nm, &NetworkManager::Notifier::primaryConnectionChanged, this, &InstallerPrompt::updateConnectionStatus);
}

void InstallerPrompt::updateConnectionStatus() {
    auto status = NetworkManager::status();
    bool online = false;
    QString statusText;

    switch (status) {
        case NetworkManager::Status::Disconnected:
        case NetworkManager::ConnectedLinkLocal:
        case NetworkManager::Asleep:
            statusText = tr("Not Connected");
            ui->connectionLED->setColor(Qt::red);
            ui->connectionLED->setState(KLed::Off);
            break;
        case NetworkManager::Status::Connected:
            online = true;
            statusText = tr("Connected");
            ui->connectionLED->setColor(Qt::green);
            ui->connectionLED->setState(KLed::On);
            break;
        case NetworkManager::Status::Connecting:
            statusText = tr("Connecting...");
            ui->connectionLED->setColor(Qt::yellow);
            ui->connectionLED->setState(KLed::On);
            break;
        case NetworkManager::Status::Disconnecting:
            statusText = tr("Disconnecting...");
            ui->connectionLED->setColor(Qt::yellow);
            ui->connectionLED->setState(KLed::On);
            break;
        default:
            qDebug() << "Unknown status:" << status;
            statusText = tr("Unknown Status");
            ui->connectionLED->setColor(Qt::gray);
            ui->connectionLED->setState(KLed::Off);
            break;
    }

    ui->connectionStatusLabel->setText(statusText);
 
    const auto devices = NetworkManager::networkInterfaces();
    bool wifiEnabled = false;
    if (NetworkManager::isNetworkingEnabled()) {
      for (const auto &device : devices) {
          if (device->type() == NetworkManager::Device::Wifi && NetworkManager::isWirelessEnabled()) wifiEnabled = true;
      }
    }

    bool connectable = !online && wifiEnabled;
    if (connectable) refreshNetworkList();

    ui->connectWiFiButton->setVisible(connectable);
    ui->WiFiLabel->setVisible(connectable);
    ui->networkComboBox->setVisible(connectable);
    ui->WiFiInfoLabel->setVisible(connectable);
    ui->WiFiSpacer->changeSize(connectable ? 40 : 0, connectable ? 20 : 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void InstallerPrompt::handleWiFiConnectionChange(NetworkManager::Device::State newstate, NetworkManager::Device::State oldstate, NetworkManager::Device::StateChangeReason reason)
{
    QMutexLocker locker(&wifiChangeMutex);
    if (reason == NetworkManager::Device::NoSecretsReason && !wifiWrongHandling) {
        wifiWrongHandling = true;
        qDebug() << wifiSSID;
        
        foreach (const NetworkManager::Connection::Ptr &connection, NetworkManager::listConnections()) {
            if (connection->settings()->connectionType() == NetworkManager::ConnectionSettings::Wireless) {
                auto wirelessSetting = connection->settings()->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
                if (wirelessSetting && wirelessSetting->ssid() == wifiSSID) {
                    qDebug() << "Wiping connection with wrong password: " << wifiSSID;
                    // Show the Incorrect Password text
                    ui->incorrectPassword->setVisible(true);
                    QDBusPendingReply removeReply = connection->remove();
                    removeReply.waitForFinished();
                }
            }
        }
        wifiWrongHandling = false;
    }
}

NetworkManager::Connection::Ptr InstallerPrompt::findConnectionBySsid(const QString &ssid) {
    foreach (const NetworkManager::Connection::Ptr &connection, NetworkManager::listConnections()) {
        if (connection->settings()->connectionType() == NetworkManager::ConnectionSettings::Wireless) {
            auto wirelessSetting = connection->settings()->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
            if (wirelessSetting && wirelessSetting->ssid() == ssid) {
                return connection;
            }
        }
    }
    return NetworkManager::Connection::Ptr(); // Return null pointer if not found
}

QString InstallerPrompt::promptForWifiPassword(const QString &ssid, bool isWrongPassword) {
    QDialog passwordDialog(this);
    passwordDialog.setModal(true);
    passwordDialog.setWindowTitle(tr("Wi-Fi Password Required"));
    passwordDialog.setWindowIcon(QIcon::fromTheme("network-wireless"));
    passwordDialog.setStyleSheet("QLabel { color: black; } ");
    passwordDialog.setMinimumWidth(250);
    passwordDialog.setMinimumHeight(120);
    passwordDialog.setMaximumWidth(5000);
    passwordDialog.setMaximumHeight(500);

    QVBoxLayout layout;
    QLabel passwordLabel(tr("Enter password for \"%1\":").arg(ssid), &passwordDialog);
    QLineEdit passwordLineEdit(&passwordDialog);
    QPushButton passwordButton(tr("Connect"), &passwordDialog);

    passwordLineEdit.setEchoMode(QLineEdit::Password);
    layout.addWidget(&passwordLabel);
    layout.addWidget(&passwordLineEdit);
    layout.addWidget(&passwordButton);

    passwordDialog.setLayout(&layout);

    // Connect with a lambda function for inline validation
    connect(&passwordLineEdit, &QLineEdit::textChanged, this, [&passwordLineEdit](const QString &text) {
        int minLength = 8;
        int maxLength = 64;
        bool isValid = text.length() >= minLength && text.length() <= maxLength;
        passwordLineEdit.setStyleSheet(isValid ? "" : "border: 1px solid red;");
    });

    connect(&passwordButton, &QPushButton::clicked, &passwordDialog, &QDialog::accept);

    if (passwordDialog.exec() == QDialog::Accepted) {
        return passwordLineEdit.text();
    }

    return QString();
}

void InstallerPrompt::handleWifiConnection(const QString &ssid, bool recoverFromWrongPassword) {
    ui->incorrectPassword->setVisible(false);
    ui->networkComboBox->setEnabled(false);
    wifiSSID = ssid;
    qDebug() << "Attempting to find connection for SSID:" << ssid;

    // Check for existing connection
    NetworkManager::Connection::Ptr connection = findConnectionBySsid(ssid);
    if (connection && !recoverFromWrongPassword) {
        qDebug() << "Using existing connection for:" << ssid;
        NetworkManager::activateConnection(connection->path(), wifiDevice->uni(), QString());
        ui->networkComboBox->setEnabled(true);
        return;
    }

    // Prompt for Wi-Fi password
    QString password = promptForWifiPassword(ssid);
    if (password.isEmpty()) {
        ui->networkComboBox->setEnabled(true);
        return;
    }

    // Create new Wi-Fi connection
    NMVariantMapMap settings = createSettingsBySSID(ssid);
    if (settings.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to create Wi-Fi settings."));
        ui->networkComboBox->setEnabled(true);
        return;
    }

    // Update the wireless security settings
    QVariantMap wirelessSecurity;
    wirelessSecurity["key-mgmt"] = "wpa-psk";
    wirelessSecurity["psk"] = password;
    settings["802-11-wireless-security"] = wirelessSecurity;

    // Add the new connection
    QDBusPendingReply<QDBusObjectPath> reply = NetworkManager::addConnection(settings);
    reply.waitForFinished();
    if (reply.isError()) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to add Wi-Fi connection."));
        ui->networkComboBox->setEnabled(true);
        return;
    }

    // Activate the new connection
    QDBusObjectPath path = reply.value();
    NetworkManager::activateConnection(path.path(), wifiDevice->uni(), QString());
    ui->networkComboBox->setEnabled(true);
}

NMVariantMapMap InstallerPrompt::createSettingsBySSID(const QString &ssid) {
    NMVariantMapMap convertedSettings;

    if (!wifiDevice) {
        qWarning() << "Wi-Fi device not found. Unable to set interface name.";
        return convertedSettings;
    }

    NetworkManager::ConnectionSettings::Ptr newConnectionSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    newConnectionSettings->setId(ssid);
    newConnectionSettings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
    newConnectionSettings->setInterfaceName(wifiDevice->interfaceName());

    // Configure wireless settings
    QVariantMap wirelessSetting;
    wirelessSetting.insert("ssid", ssid.toUtf8());
    convertedSettings.insert("802-11-wireless", wirelessSetting);

    // Convert other settings
    const auto settingsMap = newConnectionSettings->toMap();
    for (const auto &key : settingsMap.keys()) {
        QVariant value = settingsMap.value(key);
        convertedSettings.insert(key, value.toMap());
    }

    return convertedSettings;
}

void InstallerPrompt::onConnectWifiClicked() {
    QString selectedSSID = ui->networkComboBox->currentText();
    handleWifiConnection(selectedSSID);
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
        ui->connectWiFiButton->setVisible(false);
        return;
    }

    QMap<QString, NetworkManager::WirelessNetwork::Ptr> tempWifiNetworkMap;
    QStringList ssidList;

    const auto networks = wirelessDevice->networks();
    for (const auto &network : networks) {
        QString ssid = network->ssid();
        tempWifiNetworkMap[ssid] = network;
        ssidList.append(ssid);
    }

    // Update the main map and combo box only after the new list is ready
    ui->networkComboBox->clear();
    ui->networkComboBox->addItems(ssidList);

    // Adjust visibility
    ui->networkComboBox->setVisible(!networks.isEmpty());
    ui->connectWiFiButton->setVisible(!networks.isEmpty());
}

void InstallerPrompt::initLanguageComboBox() {
    languageLocaleMap.clear();
    QStringList languages = getAvailableLanguages();
    for (const auto &language : languages) {
        ui->languageComboBox->addItem(language);
    }

    QString defaultLanguage = "English (United States)";
    int defaultIndex = ui->languageComboBox->findText(defaultLanguage);
    if (defaultIndex != -1) {
        ui->languageComboBox->setCurrentIndex(defaultIndex);
    }
}

QStringList InstallerPrompt::getAvailableLanguages() {
    QMap<QString, QString> languageMap; // Default sorting by QString is case-sensitive

    auto sanitize = [](QString s) -> QString {
        s.replace("St.", "Saint", Qt::CaseInsensitive);
        s.replace("&", "and", Qt::CaseInsensitive);
        return s;
    };

    for (int language = QLocale::C; language <= QLocale::LastLanguage; ++language) {
        foreach (int country, QLocale::countriesForLanguage(static_cast<QLocale::Language>(language))) {
            QLocale locale(static_cast<QLocale::Language>(language), static_cast<QLocale::Country>(country));
            QString nativeName = locale.nativeLanguageName();
            if (nativeName.isEmpty()) continue;

            QString nativeCountryName = sanitize(locale.nativeCountryName());
            QString englishLanguageName = QLocale::languageToString(locale.language());
            QString englishCountryName = sanitize(QLocale::countryToString(locale.country()));

            // Rename "American English" to "English"
            if (locale.language() == QLocale::English) {
                nativeName = "English";
                englishLanguageName = "English";
            }

            QString displayName = nativeName + " (" + nativeCountryName + ")";
            if (nativeName.compare(englishLanguageName, Qt::CaseInsensitive) != 0 &&
                nativeCountryName.compare(englishCountryName, Qt::CaseInsensitive) != 0) {
                displayName += " - " + englishLanguageName + " (" + englishCountryName + ")";
            }

            languageMap.insert(displayName, locale.name());
        }
    }
    // Sort the language display names
    QStringList sortedLanguages = languageMap.keys();
    std::sort(sortedLanguages.begin(), sortedLanguages.end(), [](const QString &a, const QString &b) {
        return a.compare(b, Qt::CaseInsensitive) < 0;
    });

    // Clear the existing languageLocaleMap and repopulate it based on sortedLanguages
    languageLocaleMap.clear();
    for (const QString &languageName : sortedLanguages) {
        languageLocaleMap.insert(languageName, languageMap[languageName]);
    }

    return sortedLanguages;
}

void InstallerPrompt::onLanguageChanged(int index) {
    QString selectedLanguage = ui->languageComboBox->itemText(index);
    QString localeName = languageLocaleMap.value(selectedLanguage);
    qDebug() << selectedLanguage;
    qDebug() << index << languageLocaleMap;

    // Split the locale name to get language and country code
    QStringList localeParts = localeName.split('_');
    QString languageCode = localeParts.value(0);
    QString countryCode = localeParts.value(1);

    // Construct the command to run the script with parameters
    QString scriptPath = "/usr/libexec/change-system-language";  // Update with the actual path
    QStringList arguments;
    arguments << languageCode << countryCode;

    QProcess::execute(scriptPath, arguments);
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
