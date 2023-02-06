# Lubuntu Installer Prompt

This project is in alpha. It may change drastically, or not. It presents a "Try or Install Lubuntu" screen. Eventually we want to extend this to support multiple flavors.

Releases are signed with Simon Quigley's GPG key: 5C7ABEA20F8630459CC8C8B5E27F2CF8458C2FA4

Licensing info:
 - Everything is GPL-3 by 2022 Lubuntu Developers <lubuntu-devel@lists.ubuntu.com> unless stated otherwise.
 - img/background.png is licensed CC-BY-4.0, authored by Aaron Rainbolt <arraybolt3@gmail.com>, copyright ownership the same as source

## Architecture

This section serves to explain how lubuntu-installer-prompt's various components work together to provide the Lubuntu ISO boot experience.

1. SDDM loads.
2. /etc/sddm.conf is read. This file has been generated (or modified?) by Casper to point to a lubuntu-live-environment.desktop X session.
3. lubuntu-live-environment.desktop executes /bin/start-lubuntu-live-env.
4. start-lubuntu-live-env starts Openbox and backgrounds it.
5. start-lubuntu-live-env starts /bin/lubuntu-installer-prompt but does not background it.
6. The installer prompt appears.
7. The user clicks "Install Lubuntu" or "Try Lubuntu".
8. If "Install Lubuntu" is clicked, the installer prompt executes /usr/libexec/lubuntu-installer.sh, which then executes Calamares. The installer prompt then removes the buttons from the screen.
9. The installer window appears on the user's screen.
10. If Calamares closes, the installer prompt detects this and exits.
11. When the installer prompt exits, start-lubuntu-live-env kills Openbox, then runs startlxqt.
12. If "Try Lubuntu" is clicked, steps 10 and 11 are executed immediately.
