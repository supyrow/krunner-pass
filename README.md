Krunner pass
============

Integrates [krunner](https://userbase.kde.org/Plasma/Krunner) with [pass](https://www.passwordstore.org).

## Use with [pass-otp](https://github.com/tadfisher/pass-otp)


This plugin supports `pass-otp` via a KRunner Action.

To copy an OTP code, type pass then what your searching for, once you see results, highlight the one you want (e.g., `Shift+Enter`) or click `Copy OTP` the `Lock Icon`.

This works for any entry that has OTP embedded in the file(the normal way) without needing to rename files.

this also adds the option to only search when the chosen keyword is used. `pass` is default.  This option now has a checkbox in the `Configuration` for `Krunner` under `Pass` (default is `Enabled`)

<br>
<br>
To create an action that copies `my.email@gmail.com`:

1.  Go to **System Settings -> Search -> KRunner -> Pass -> Configure**.
2.  Check the box for **"Show saved actions"**.
3.  In the **"New Action"** section, fill out the fields:
    *   **Name:** `Copy Username`
    *   **Icon:** `user-identity`
    *   **RegEx:** `Username: (.*)`
4.  Click **"Add"**, then **"Apply"**.
5.  Restart the plugin (uncheck and re-check it in the settings list).

Now, when you search for `pass google.com`, you can highlight result and select "Copy Username" to copy your email to the clipboard. The text inside the `()` in the RegEx is what gets copied.
<br>
<hr>
<img width="1242" height="242" alt="Plasma_Search__System_Settings" src="https://github.com/user-attachments/assets/0d714a2b-ee87-4821-a7f2-f7db576ca236" />
<img width="545" height="708" alt="Configure__System_Settings" src="https://github.com/user-attachments/assets/75fd371b-ee4c-4701-a432-971919d86db4" />
<img width="535" height="110" alt="Screenshot_20251001_145542" src="https://github.com/user-attachments/assets/7a58e6b3-a35a-49d3-addb-84fe1d269479" />

<br>
<hr>
<br>



<img width="535" height="110" alt="Screenshot_20251001_145456" src="https://github.com/user-attachments/assets/20829c27-63b0-4d67-8ba2-3646db812fbf" />
<br>
<hr>
<br>
<img width="535" height="110" alt="Screenshot_20251001_145614" src="https://github.com/user-attachments/assets/e56e098b-7371-4bcf-a7be-ebaf0acf6439" />

<br>
<hr>
<br>
<img width="532" height="115" alt="Screenshot_20250930_214131" src="https://github.com/user-attachments/assets/9723eb60-9a35-40b4-8dbf-ad4a2e50a588" />
<img width="551" height="65" alt="Screenshot_20250930_214432" src="https://github.com/user-attachments/assets/9f8a9d40-1f34-4d92-a405-7e45e2e14702" />


Build and Installation
======================

The provided `install.sh` script is the recommended way to build and install. It automatically detects if you are on a KF5 or KF6 system and runs the correct commands.

```
$ ./install.sh
```
Manual Build

The instructions below are for advanced users. Note that they differ for KF5 and KF6 systems.
For KF6 / Plasma 6:
```
$ mkdir -p build && cd build
$ cmake .. -DQT_MAJOR_VERSION=6 -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
$ make
```
For KF5 / Plasma 5 (Original Instructions):

```
$ mkdir -p build && cd build
$ cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_INSTALL_PREFIX=`kf5-config --prefix`
$ make
```
Dependencies:
The dependency lists below are for KF5. For a KF6 build, you will need the equivalent kf6- and qt6- packages (e.g., libkf6runner-dev, qt6-base-dev).
For debian (>=9) you will need the following build dependencies:
```
apt-get install build-essential cmake extra-cmake-modules gettext \
  qtdeclarative5-dev \
  libkf5i18n-dev \
  libkf5service-dev \
  libkf5runner-dev \
  libkf5textwidgets-dev \
  libkf5notifications-dev \
  libkf5kcmutils-dev
```

For Fedora (>=23) you will need the following build dependencies:

```
dnf install @development-tools cmake extra-cmake-modules gettext \
   qt5-qtdeclarative-devel \
   kf5-ki18n-devel \
   kf5-kservice-devel \
   kf5-krunner-devel \
   kf5-ktextwidgets-devel \
   kf5-knotifications-devel \
   kf5-kconfigwidgets-devel \
```


### Applying Configuration Changes

After making ANY changes in the "Configure" window (adding a new RegEx action), hit `Save` then you must restart the plugin for the changes to take effect.

The easiest way to do this is from the KRunner settings (**System Settings -> Search**):
```
1.  **Uncheck** the box next to the "Pass" plugin to disable it.
2. hit `Apply`
3.  **Check** the box again to re-enable it.
4.hit `Apply`
```
The plugin will now be running with your new configuration.
<img width="1057" height="678" alt="image" src="https://github.com/user-attachments/assets/b2b7a806-8ac7-4488-aa75-cf800860f2fc" />

