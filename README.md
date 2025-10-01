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
<img src="https://git.devel/su_pyrow/krunner-pass/raw/branch/master/screenshot/Screenshot_20250930_214432.png">
<br>
<hr>
<br>
<img src="https://git.devel/su_pyrow/krunner-pass/raw/branch/master/screenshot/Screenshot_20250930_214131.png">
<br>
<hr>
<br>
<img src="https://git.devel/su_pyrow/krunner-pass/raw/branch/master/screenshot/Screenshot_20250930_215154.png">
<br>
<hr>
<br>
<img src="https://git.devel/su_pyrow/krunner-pass/raw/branch/master/screenshot/Screenshot_20250930_215244.png">


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

