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
#include <QLineEdit>
#include <QDBusPendingReply>
#include "installerprompt.h"
#include "./ui_installerprompt.h"

InstallerPrompt::InstallerPrompt(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::InstallerPrompt) {
    ui->setupUi(this);

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

    // Resize the layout widget to the screen size
    ui->gridLayoutWidget->resize(screenGeometry.size());

    // Initialize process for external app launch
    process = new QProcess(this);

    // Set up signal-slot connections for buttons
    connect(ui->tryLubuntu, &QAbstractButton::clicked, this, &InstallerPrompt::tryLubuntu);
    connect(ui->installLubuntu, &QAbstractButton::clicked, this, &InstallerPrompt::installLubuntu);
    connect(ui->connectWiFiButton, &QAbstractButton::clicked, this, &InstallerPrompt::onConnectWifiClicked);

    // Set up the language combo box with available languages
    initLanguageComboBox();

    // Check initial network status and update UI
    updateConnectionStatus();

    // Find and store the Wi-Fi device
    foreach (const NetworkManager::Device::Ptr &device, NetworkManager::networkInterfaces()) {
        if (device->type() == NetworkManager::Device::Wifi) {
            wifiDevice = device.objectCast<NetworkManager::WirelessDevice>();
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
    QString statusText, statusIndicator;

    switch (status) {
        case NetworkManager::Status::Disconnected:
            statusText = tr("Not Connected");
            statusIndicator = "<span style=\"color: red;\">❌</span> " + statusText;
            break;
        case NetworkManager::Status::Connected:
            online = true;
            statusText = tr("Connected");
            statusIndicator = "<span style=\"color: green;\">🟢</span> " + statusText;
            break;
        case NetworkManager::Status::Connecting:
            statusText = tr("Connecting...");
            statusIndicator = "<span style=\"color: yellow;\">🟡</span> " + statusText;
            break;
        case NetworkManager::Status::Disconnecting:
            statusText = tr("Disconnecting...");
            statusIndicator = "<span style=\"color: yellow;\">🟡</span> " + statusText;
            break;
        default:
            statusText = tr("Unknown Status");
            statusIndicator = "<span style=\"color: grey;\">⚪</span> " + statusText;
    }
    ui->connectionStatusLabel->setText(statusIndicator);   
 
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

void InstallerPrompt::handleWifiConnection(const QString &ssid) {
    qDebug() << "Attempting to find connection for SSID:" << ssid;
    foreach (const NetworkManager::Connection::Ptr &connection, NetworkManager::listConnections()) {
        if (connection->settings()->connectionType() == NetworkManager::ConnectionSettings::Wireless) {
            auto wirelessSetting = connection->settings()->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
            if (wirelessSetting && wirelessSetting->ssid() == ssid) {
                qDebug() << "Attempting to use existing connection:" << ssid;
                NetworkManager::activateConnection(connection->path(), wifiDevice->uni(), QString());
                qDebug() << "Successfully connected:" << ssid;
                return;
            }
        }
    }

    QMap<QString, QVariant> fullSettings = createSettingsBySSID(ssid);
    NMVariantMapMap nmMap;

    for (const auto &key : fullSettings.keys()) {
        nmMap[key] = fullSettings[key].toMap();
    }

    NetworkManager::ConnectionSettings::Ptr newConnectionSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    newConnectionSettings->fromMap(nmMap);
    QVariantMap wirelessSecurity = fullSettings.value("802-11-wireless-security").toMap();

    //bool isOpenNetwork = wirelessSecurity.isEmpty() || wirelessSecurity.value("key-mgmt").toString().isEmpty();
    //if (isOpenNetwork && wifiDevice && wifiDevice->isValid() && connection) {
    //    qDebug() << "Attempting to connect to open network: " << ssid;
    //    NetworkManager::activateConnection(connection->path(), wifiDevice->uni(), QString());
    //    return;
    //}

    // If the network is secured, display the password dialog
    QDialog passwordDialog(this);
    QVBoxLayout layout(&passwordDialog);
    QLabel label(tr("Enter Wi-Fi Password for %1:").arg(ssid), &passwordDialog);
    QLineEdit lineEdit(&passwordDialog);
    QPushButton button(tr("Connect"), &passwordDialog);

    lineEdit.setEchoMode(QLineEdit::Password);
    layout.addWidget(&label);
    layout.addWidget(&lineEdit);
    layout.addWidget(&button);

    connect(&button, &QPushButton::clicked, &passwordDialog, &QDialog::accept);

    for (int attempts = 0; attempts < 3; ++attempts) {
        if (passwordDialog.exec() == QDialog::Rejected) return;
        QString password = lineEdit.text();
        if (wifiDevice && wifiDevice->isValid()) {
            // Update the wireless security settings in the map
            wirelessSecurity["key-mgmt"] = "wpa-psk";
            wirelessSecurity["psk"] = password;
            fullSettings["802-11-wireless-security"] = wirelessSecurity;

            // Convert QMap<QString, QVariant> to NMVariantMapMap
            NMVariantMapMap nmMap;
            for (const auto &key : fullSettings.keys()) {
                nmMap[key] = fullSettings[key].toMap();
            }

            // Update the connection settings
            qDebug() << "Saving the connection...";
            NetworkManager::ConnectionSettings::Ptr newConnectionSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
            newConnectionSettings->fromMap(nmMap);
            QDBusPendingReply<QDBusObjectPath> addreply = NetworkManager::addConnection(nmMap);
            addreply.waitForFinished();
            if (addreply.isError()) {
                qDebug() << nmMap;
                qDebug() << "Unable to save the connection:" << addreply.error().message();
                return;
            }

            QString uuid = fullSettings.value("connection").toMap().value("uuid").toString();
            NetworkManager::Connection::Ptr connection = NetworkManager::findConnectionByUuid(uuid);
            if (!connection) {
                qDebug() << "Unable to retrieve the connection after saving:" << addreply.error().message();
                return;
            }

            QDBusPendingReply<QDBusObjectPath> reply = NetworkManager::activateConnection(connection->path(), wifiDevice->uni(), QString());
            reply.waitForFinished();
            if (reply.isError()) {
                qDebug() << "Unable to activate the connection:" << addreply.error().message();
                return;
            }

            NetworkManager::reloadConnections();
            qDebug() << "Successfully connected:" << ssid;
            return;
        }

        label.setStyleSheet("color: red;");
        label.setText(tr("Incorrect password. Please try again:"));
    }

    QMessageBox::warning(this, tr("Connection Failed"), tr("Unable to connect to the network."));
}

QMap<QString, QVariant> InstallerPrompt::createSettingsBySSID(const QString &ssid) {
    // Create new connection settings
    NetworkManager::ConnectionSettings::Ptr newConnectionSettings(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    newConnectionSettings->setId(ssid);
    newConnectionSettings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());

    // Set interface name from wifiDevice
    if (wifiDevice) {
        newConnectionSettings->setInterfaceName(wifiDevice->interfaceName());
    } else {
        qWarning() << "Wi-Fi device not found. Unable to set interface name.";
        return QMap<QString, QVariant>();
    }

    // Configure wireless settings
    QVariantMap wirelessSetting;
    wirelessSetting.insert("ssid", ssid.toUtf8());

    // Configure wireless security settings
    NetworkManager::WirelessSecuritySetting::Ptr wirelessSecuritySetting = newConnectionSettings->setting(NetworkManager::Setting::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();
    wirelessSecuritySetting->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaPsk);

    // Convert settings to QVariantMap
    QMap<QString, QVariant> convertedSettings;
    const auto settingsMap = newConnectionSettings->toMap();
    for (const auto &key : settingsMap.keys()) {
        convertedSettings[key] = QVariant::fromValue(settingsMap[key]);
    }
    convertedSettings.insert("802-11-wireless", wirelessSetting);

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
    wifiNetworkMap.swap(tempWifiNetworkMap);
    ui->networkComboBox->clear();
    ui->networkComboBox->addItems(ssidList);

    // Adjust visibility
    ui->networkComboBox->setVisible(!networks.isEmpty());
    ui->connectWiFiButton->setVisible(!networks.isEmpty());
}

void InstallerPrompt::initLanguageComboBox() {
    QStringList languages = getAvailableLanguages();
    ui->languageComboBox->addItems(languages);

    QString defaultLanguage = "English (United States)";
    int defaultIndex = ui->languageComboBox->findText(defaultLanguage);
    if (defaultIndex != -1) {
        ui->languageComboBox->setCurrentIndex(defaultIndex);
    }
}

QStringList InstallerPrompt::getAvailableLanguages() const {
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

    QStringList sortedLanguages = languageMap.keys();
    std::sort(sortedLanguages.begin(), sortedLanguages.end(), [](const QString &a, const QString &b) {
        return a.compare(b, Qt::CaseInsensitive) < 0;
    });

    return sortedLanguages;
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
