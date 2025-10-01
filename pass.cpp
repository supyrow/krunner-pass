/******************************************************************************
 *  Copyright (C) 2017 by Lukas Fürmetz <fuermetz@mailbox.org>                *
 *                                                                            *
 *  This library is free software; you can redistribute it and/or modify      *
 *  it under the terms of the GNU General Public License as published         *
 *  by the Free Software Foundation; either version 3 of the License or (at   *
 *  your option) any later version.                                           *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Library General Public License for more details.                          *
 *                                                                            *
 *  You should have received a copy of the GNU General Public License         *
 *  along with this library; see the file LICENSE.                            *
 *  If not, see <http://www.gnu.org/licenses/>.                               *
 *****************************************************************************/
#include <KSharedConfig>
#include <KLocalizedString>
#include <KNotification>
#include <QIcon>
#include <QAction>
#include <QDirIterator>
#include <QProcess>
#include <QRegularExpression>
#include <QTimer>
#include <QMessageBox>
#include <QClipboard>
#include <KSystemClipboard>
#include <QMimeData>
#include <QDebug>
#include <QApplication>
#include <cstdlib>

#include "pass.h"
#include "config.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <KRunner/Action>
#endif

using namespace std;

K_PLUGIN_CLASS_WITH_JSON(Pass, "pass.json")

Pass::Pass(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    : KRunner::AbstractRunner(metaData, parent)
#else
    : KRunner::AbstractRunner(parent, metaData)
#endif
{
    Q_UNUSED(args)
    setObjectName(QStringLiteral("Pass"));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    setPriority(HighestPriority);
#endif
}

Pass::~Pass() = default;

void Pass::reloadConfiguration()
{
    orderedActions.clear();
    KConfigGroup cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"))->group("Runners").group("Pass");

    bool enableOtp = cfg.readEntry(Config::enableOtp, true);
    if (enableOtp) {
        #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            auto* otpAction = new QAction(QIcon::fromTheme("dialog-password"), i18n("Copy OTP"), this);
            otpAction->setData("copy-otp");
            this->orderedActions << otpAction;
        #else
            this->orderedActions << KRunner::Action(QStringLiteral("copy-otp"), QStringLiteral("dialog-password"), i18n("Copy OTP"));
        #endif
    }

    bool showActions = cfg.readEntry(Config::showActions, false);
    if (showActions) {
        const auto configActions = cfg.group(Config::Group::Actions);
        for (const auto &name : configActions.groupList()) {
            auto group = configActions.group(name);
            auto passAction = PassAction::fromConfig(group);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            auto *act = new QAction(QIcon::fromTheme(passAction.icon, QIcon::fromTheme("object-unlocked")), passAction.name, this);
            act->setData(passAction.regex);
            this->orderedActions << act;
#else
            this->orderedActions << KRunner::Action(passAction.regex, QIcon::hasThemeIcon(passAction.icon) ? passAction.icon : QStringLiteral("object-unlocked"), passAction.name);
#endif
        }
    }

    if (cfg.readEntry(Config::showFileContentAction, false)) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        auto *act = new QAction(QIcon::fromTheme("document-new"), i18n("Show password file contents"), this);
        act->setData(Config::showFileContentAction);
        this->orderedActions << act;
#else
        this->orderedActions << KRunner::Action(Config::showFileContentAction, "document-new", i18n("Show password file contents"));
#endif
    }

    addSyntax(KRunner::RunnerSyntax(QString("pass "), i18n("Looks for a password matching the query.")));
}

void Pass::init()
{
    reloadConfiguration();
    this->baseDir = QDir(QDir::homePath() + "/.password-store");
    if (const auto _baseDir = getenv("PASSWORD_STORE_DIR")) {
        this->baseDir = QDir(_baseDir);
    }
    this->timeout = 45;
    if (const auto _timeout = getenv("PASSWORD_STORE_CLIP_TIME")) {
        bool ok;
        int timeoutParsed = QString::fromUtf8(_timeout).toInt(&ok);
        if (ok) {
            this->timeout = timeoutParsed;
        }
    }

    watcher = new QFileSystemWatcher(this);

    initPasswords();
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, &Pass::reinitPasswords);
}

void Pass::initPasswords()
{

    if (!watcher->files().isEmpty() || !watcher->directories().isEmpty()) {
        watcher->removePaths(watcher->files() + watcher->directories());
    }

    passwords.clear();

    watcher->addPath(this->baseDir.absolutePath());
    QDirIterator it(this->baseDir, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        const auto fileInfo = it.fileInfo();
        if (fileInfo.isFile() && fileInfo.suffix() == QLatin1String("gpg")) {
            QString password = this->baseDir.relativeFilePath(fileInfo.absoluteFilePath());
            password.chop(4);
            passwords.append(password);
        } else if (fileInfo.isDir() && it.fileName() != "." && it.fileName() != "..") {
            watcher->addPath(it.filePath());
        }
    }
}

void Pass::reinitPasswords(const QString &path)
{
    Q_UNUSED(path)
    QWriteLocker locker(&lock);
    initPasswords();
}

void Pass::match(KRunner::RunnerContext &context)
{
    if (!context.isValid()) {
        return;
    }

    KConfigGroup cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"))->group("Runners").group("Pass");
    const QString triggerWord = cfg.readEntry(Config::triggerWord, "pass");

    const QString query = context.query();
    QString searchTerm;

    if (!triggerWord.isEmpty()) {
        const QString prefix = triggerWord + QLatin1String(" ");
        if (!query.startsWith(prefix, Qt::CaseInsensitive)) {
            return;
        }
        searchTerm = query.mid(prefix.length());
        if (searchTerm.isEmpty()) {
            return;
        }
    } else {
        if (query.size() < 1 && !context.singleRunnerQueryMode()) {
            return;
        }
        searchTerm = query;
    }

    QList<KRunner::QueryMatch> matches;
    QReadLocker locker(&lock);
    for (const auto &password : qAsConst(passwords)) {
        if (password.contains(searchTerm, Qt::CaseInsensitive)) {
            KRunner::QueryMatch match(this);
            match.setCategoryRelevance(KRunner::QueryMatch::CategoryRelevance::Highest);
            match.setIcon(QIcon::fromTheme("object-locked"));
            match.setText(password);
            #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                match.setActions(this->orderedActions);
            #endif
            matches.append(match);
        }
    }
    locker.unlock();
    context.addMatches(matches);
}

void Pass::clip(const QString &msg)
{
    auto md = new QMimeData;
    auto kc = KSystemClipboard::instance();
    md->setText(msg);
    md->setData(QStringLiteral("x-kde-passwordManagerHint"), "secret");
    kc->setMimeData(md, QClipboard::Clipboard);
    QTimer::singleShot(timeout * 1000, []() {
        KSystemClipboard::instance()->clear(QClipboard::Clipboard);
    });
}

void Pass::run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match)
{
    Q_UNUSED(context);

    QString selectedActionId;
    QString selectedActionText;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (!match.selectedAction().id().isEmpty()) {
        selectedActionId = match.selectedAction().id();
        selectedActionText = match.selectedAction().text();
    }
#else
    if (match.selectedAction()) {
        selectedActionId = match.selectedAction()->data().toString();
        selectedActionText = match.selectedAction()->text();
    }
#endif

    auto *pass = new QProcess();
    QStringList args;

    if (selectedActionId == QLatin1String("copy-otp")) {
        args << "otp" << match.text();
    } else {
        args << "show" << match.text();
    }

    pass->start("pass", args);

    connect(pass, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitStatus)

        if (exitCode == 0) {
            const auto output = pass->readAllStandardOutput();

            if (match.selectedAction()) {
                if (selectedActionId == Config::showFileContentAction) {
                    QMessageBox::information(nullptr, match.text(), output);
                } else if (selectedActionId == QLatin1String("copy-otp")) {
                    clip(QString::fromUtf8(output).trimmed());
                    this->showNotification(match.text(), i18n("OTP"));
                } else {
                    QRegularExpression re(selectedActionId, QRegularExpression::MultilineOption);
                    const auto matchre = re.match(output);

                    if (matchre.hasMatch()) {
                        clip(matchre.captured(1));
                        this->showNotification(match.text(), selectedActionText);
                    } else {
                        qInfo() << "Regexp: " << selectedActionId;
                    }
                }
            } else {
                const auto string = QString::fromUtf8(output.data());
                const auto lines = string.split('\n', Qt::SkipEmptyParts);
                if (!lines.isEmpty()) {
                    clip(lines[0]);
                    this->showNotification(match.text());
                }
            }
        }

        pass->close();
        pass->deleteLater();
    });
}
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QList<QAction *> Pass::actionsForMatch(const KRunner::QueryMatch &match)
{
    Q_UNUSED(match)
    return this->orderedActions;
}
#endif

void Pass::showNotification(const QString &text, const QString &actionName)
{
    const QString whatWasCopied = actionName.isEmpty() ? i18n("Password") : actionName;
    const QString msg = i18n("%1 for %2 copied to clipboard for %3 seconds", whatWasCopied, text, timeout);
    KNotification::event("password-unlocked", "Pass", msg,
                         "object-unlocked",
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                         nullptr,
#endif
                         KNotification::CloseOnTimeout,
                         "krunner_pass");
}

#include "pass.moc"
