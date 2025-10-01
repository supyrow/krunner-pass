cmake_minimum_required(VERSION 3.16.0)

project(RunnerPass)

set(KPLUGIN_ID "krunner_pass")
set(KCM_PLUGIN_ID "kcm_krunner_pass")

find_package(ECM 5.12.0 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH
     ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_MODULE_PATH}
)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/VERSION PROJECT_VERSION_STRING)
string(STRIP ${PROJECT_VERSION_STRING} PROJECT_VERSION_STRING)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Copies a password from the password store (https://www.passwordstore.org/) to the clipboard")
set(CPACK_PACKAGE_NAME ${KPLUGIN_ID})
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_STRING}")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "alex1701c")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/postinst")
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/postinst")

INCLUDE(CPack)

if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_MIN_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
    set(KPLUGIN_ID_LINE "")
else()
  set(KF_MAJOR_VERSION "5")
  set(KPLUGIN_ID_LINE "\"Id\": \"${KPLUGIN_ID}\",")
endif()

set(KCM_CONFIG_MODULE_PATH "kf${KF_MAJOR_VERSION}/krunner/kcms/${KCM_PLUGIN_ID}")


find_package (Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION}
  REQUIRED CONFIG COMPONENTS
  Widgets
  Core
  Quick
  DBus
)
find_package (KF${KF_MAJOR_VERSION}
  REQUIRED COMPONENTS
  I18n
  Service
  Runner
  TextWidgets
  ConfigWidgets
  Notifications
  Auth
  KCMUtils
  GuiAddons
)

if(KF_MAJOR_VERSION STREQUAL 5)
  find_package(KF${KF_MAJOR_VERSION}
    REQUIRED COMPONENTS
    PlasmaQuick
    Plasma
  )
else()
  find_package(PlasmaQuick REQUIRED)
  find_package(Plasma)
endif()

set(krunner_pass_SRCS
    pass.cpp
)

set(kcm_krunner_pass_SRCS
    config.cpp
)

ki18n_wrap_ui(kcm_krunner_pass_SRCS config.ui)

add_library(${KCM_PLUGIN_ID} MODULE ${kcm_krunner_pass_SRCS})
target_link_libraries(${KCM_PLUGIN_ID}
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    KF${KF_MAJOR_VERSION}::CoreAddons
    KF${KF_MAJOR_VERSION}::ConfigCore
    KF${KF_MAJOR_VERSION}::ConfigWidgets
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::Runner
    KF${KF_MAJOR_VERSION}::KCMUtils
    )
kcmutils_generate_desktop_file(${KCM_PLUGIN_ID})

add_library(${KPLUGIN_ID} MODULE ${krunner_pass_SRCS})
target_link_libraries(${KPLUGIN_ID} KF${KF_MAJOR_VERSION}::Runner Qt${QT_MAJOR_VERSION}::Widgets
                      KF${KF_MAJOR_VERSION}::I18n
                      KF${KF_MAJOR_VERSION}::Service
                      KF${KF_MAJOR_VERSION}::ConfigWidgets
                      KF${KF_MAJOR_VERSION}::Notifications
                      KF${KF_MAJOR_VERSION}::KCMUtils
                      KF${KF_MAJOR_VERSION}::GuiAddons)
if(KF_MAJOR_VERSION STREQUAL 5)
  target_link_libraries(${KPLUGIN_ID}
    KF${KF_MAJOR_VERSION}::Plasma
  )
else()
   target_link_libraries(${KPLUGIN_ID}
     Plasma::Plasma
   )
endif()

add_dependencies(${KPLUGIN_ID} ${KCM_PLUGIN_ID})

configure_file(pass.json.in ${CMAKE_CURRENT_BINARY_DIR}/pass.json)
install(TARGETS ${KCM_PLUGIN_ID} DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/kf${KF_MAJOR_VERSION}/krunner/kcms)
install(TARGETS ${KPLUGIN_ID} DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/kf${KF_MAJOR_VERSION}/krunner)
install(FILES krunner_pass.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
