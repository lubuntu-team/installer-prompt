# Lubuntu Installer Prompt

This project is in beta. It presents a "Try or Install Lubuntu" screen. Eventually we want to extend this to support multiple flavors.

Releases are signed with Simon Quigley's GPG key: 5C7ABEA20F8630459CC8C8B5E27F2CF8458C2FA4

All art assets are (C) 2010-2018 Rafael Laguna <rafaellaguna@gmail.com> (GPL-2.0+). Otherwise, the source is (C) 2022-2023 Lubuntu Developers <lubuntu-devel@lists.ubuntu.com>.

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

## Translations

Run the `gen_ts.sh` script after making any code modifications to ensure that the translations files are up-to-date for translators to work on.

To add a new language to be translated:

* Open the `gen_ts.sh` script and add the locale code for the new language to the `langList` array.
* Run the script after doing this - a new template .ts file will be generated under `src/translations/`.
* Next, add the new template file to the `TS_FILES` list in `CMakeLists.txt` - it will be named `src/translations/lubuntu-installer-prompt-locale_CODE.ts`, where `locale_CODE` is the locale code of the added language.
* Finally, add a line in the src/translations.qrc resource file to include the new translation file. The line should look like `<file alias="locale_CODE">lubuntu-installer-prompt_locale_CODE.qm</file>`, where `locale_CODE` is the locale code of the added language. This line should go inside the `<qresource>` tag.

For instance, if I were to add Chinese to the list of languages that could be translated into, I would do this:

    vim gen_ts.sh
    # add this code to the langList array:
    #    'zh_CN'
    ./gen_ts.sh
    vim CMakeLists.txt
    # add this line to the TS_FILES list:
    #    src/translations/lubuntu-installer-prompt_zh_CN.ts
    vim src/translations.qrc
    # add this line to the list of file resources:
    #    <file alias="zh_CN">lubuntu-installer-prompt_zh_CN.qm</file>

The program will now pick up the added language at build time. Any translations added to the newly created .ts file will be shown to program users who select the new language.
