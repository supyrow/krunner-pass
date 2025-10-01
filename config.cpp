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
#include <KPluginFactory>
#include <krunner/abstractrunner.h>
#include <QToolButton>
#include <QtCore/QDir>

#include "kcmutils_version.h"
#include "config.h"

K_PLUGIN_FACTORY_WITH_JSON(PassConfigFactory, "kcm_krunner_pass.json", registerPlugin<PassConfig>();)

PassConfigForm::PassConfigForm(QWidget *parent)
        : QWidget(parent)
{
    setupUi(this);
    this->listSavedActions->setDragEnabled(true);
    this->listSavedActions->setDragDropMode(QAbstractItemView::InternalMove);
    this->listSavedActions->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(this->checkAdditionalActions, &QCheckBox::stateChanged, this, &PassConfigForm::updateUiState);

    connect(this->buttonAddAction, &QPushButton::clicked, [this]() {
        if (this->buttonAddAction->text() == tr("Add")) {
            this->addPassAction(this->lineName->text(), this->lineIcon->text(), this->lineRegEx->text(), true);
        } else {
            QListWidgetItem *currentItem = this->listSavedActions->currentItem();
            if (currentItem) {
                PassAction updatedAction;
                updatedAction.icon = this->lineIcon->text();
                updatedAction.regex = this->lineRegEx->text();

                currentItem->setData(Qt::UserRole, QVariant::fromValue(updatedAction));
                currentItem->setText(updatedAction.name);

                emit passActionUpdated();

                this->listSavedActions->clearSelection();
                this->clearInputsAndResetButton();
            }
        }
    });

    connect(this->lineIcon, &QLineEdit::textChanged, this, &PassConfigForm::validateAddButton);
    connect(this->lineName, &QLineEdit::textChanged, this, &PassConfigForm::validateAddButton);
    connect(this->lineRegEx, &QLineEdit::textChanged, this, &PassConfigForm::validateAddButton);
    validateAddButton();
}

void PassConfigForm::addPassAction(const QString &name, const QString &icon, const QString &regex, bool isNew)
{
    if (isNew) {
        for (const auto &act: this->passActions())
            if (act.name == name)
                return;
    }

    auto *listWidget = new QWidget(this);
    auto *layoutAction = new QHBoxLayout(listWidget);
    auto *buttonRemoveAction = new QToolButton(listWidget);

    buttonRemoveAction->setIcon(QIcon::fromTheme("delete"));
    layoutAction->setContentsMargins(0,0,0,0);
    layoutAction->addStretch();
    layoutAction->addWidget(buttonRemoveAction);
    listWidget->setLayout(layoutAction);

    auto *item = new QListWidgetItem(name + (isNew ? "*" : ""), this->listSavedActions);
    item->setData(Qt::UserRole, QVariant::fromValue(PassAction{name, icon, regex}));
    this->listSavedActions->setItemWidget(item, listWidget);

    clearInputsAndResetButton();
    if (isNew) {
        emit passActionAdded();
    }

    connect(buttonRemoveAction, &QToolButton::clicked, [=](bool clicked) {
        Q_UNUSED(clicked)
        delete this->listSavedActions->takeItem(this->listSavedActions->row(item));
        delete listWidget;
        emit passActionRemoved();
    });
}

QVector<PassAction> PassConfigForm::passActions()
{
    QVector<PassAction> passActions;
    for (int i = 0; i < listSavedActions->count(); ++i) {
        QListWidgetItem *item = this->listSavedActions->item(i);
        passActions << item->data(Qt::UserRole).value<PassAction>();
    }
    return passActions;
}

void PassConfigForm::clearPassActions()
{
    for (int i = 0; i < listSavedActions->count(); ++i) {
        QListWidgetItem *item = this->listSavedActions->item(i);
        delete this->listSavedActions->itemWidget(item);
    }
    this->listSavedActions->clear();
}

void PassConfigForm::clearInputsAndResetButton()
{
    this->lineName->clear();
    this->lineIcon->clear();
    this->lineRegEx->clear();

    this->buttonAddAction->setText(tr("Add"));
    this->lineName->setEnabled(true);
    validateAddButton();
}

void PassConfigForm::validateAddButton()
{
    this->buttonAddAction->setDisabled(this->lineIcon->text().isEmpty() ||
            this->lineName->text().isEmpty() ||
            this->lineRegEx->text().isEmpty());
}

void PassConfigForm::updateUiState()
{
    bool isChecked = this->checkAdditionalActions->isChecked();
    this->boxNewAction->setEnabled(isChecked);
    this->boxSavedActions->setEnabled(isChecked);
}

void PassConfigForm::on_listSavedActions_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (current) {
        PassAction action = current->data(Qt::UserRole).value<PassAction>();
        this->lineName->setText(action.name);
        this->lineIcon->setText(action.icon);
        this->lineRegEx->setText(action.regex);

        this->lineName->setEnabled(false);
        this->buttonAddAction->setText(tr("Update"));
        validateAddButton();
    } else {
        clearInputsAndResetButton();
    }
}

PassConfig::PassConfig(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
        :
        KCModule(parent, data)
{
    Q_UNUSED(args)
    this->ui = new PassConfigForm(this->widget());
    QGridLayout *layout = new QGridLayout(this->widget());
    layout->addWidget(ui, 0, 0);
#if KCMUTILS_VERSION >= QT_VERSION_CHECK(5, 64, 0)
    const auto changedSlotPointer = &PassConfig::markAsChanged;
#else
    const auto changedSlotPointer = static_cast<void (PassConfig::*)()>(&PassConfig::changed);
#endif
    connect(this->ui, &PassConfigForm::passActionAdded, this, changedSlotPointer);
    connect(this->ui, &PassConfigForm::passActionRemoved, this, changedSlotPointer);
    connect(this->ui, &PassConfigForm::passActionUpdated, this, changedSlotPointer);
    connect(this->ui->lineTriggerWord, &QLineEdit::textChanged, this, changedSlotPointer);
    connect(this->ui->checkEnableOtp, &QCheckBox::stateChanged, this, changedSlotPointer);
    connect(this->ui->checkAdditionalActions, &QCheckBox::stateChanged, this, changedSlotPointer);
    connect(this->ui->checkShowFileContentAction, &QCheckBox::stateChanged, this, changedSlotPointer);

void PassConfig::load()
{
    KCModule::load();
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    KConfigGroup passCfg = cfg->group("Runners").group("Pass");

    QString triggerWord = passCfg.readEntry(Config::triggerWord, "pass");
    bool enableOtp = passCfg.readEntry(Config::enableOtp, true);
    bool showActions = passCfg.readEntry(Config::showActions, false);
    bool showFileContentAction = passCfg.readEntry(Config::showFileContentAction, false);

    this->ui->lineTriggerWord->setText(triggerWord);
    this->ui->checkEnableOtp->setChecked(enableOtp);
    this->ui->checkAdditionalActions->setChecked(showActions);
    this->ui->checkShowFileContentAction->setChecked(showFileContentAction);

    this->ui->updateUiState();

    this->ui->clearPassActions();
    const auto actionGroup = passCfg.group(Config::Group::Actions);
    for (const auto &name: actionGroup.groupList()) {
        auto group = actionGroup.group(name);
        auto passAction = PassAction::fromConfig(group);
        this->ui->addPassAction(passAction.name, passAction.icon, passAction.regex, false);
    }
}

void PassConfig::save()
{
    KCModule::save();
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QStringLiteral("krunnerrc"));
    KConfigGroup passCfg = cfg->group("Runners").group("Pass");

    auto triggerWord = this->ui->lineTriggerWord->text();
    auto enableOtp = this->ui->checkEnableOtp->isChecked();
    auto showActions = this->ui->checkAdditionalActions->isChecked();
    auto showFileContentAction = this->ui->checkShowFileContentAction->isChecked();

    passCfg.writeEntry(Config::triggerWord, triggerWord);
    passCfg.writeEntry(Config::enableOtp, enableOtp);
    passCfg.writeEntry(Config::showActions, showActions);
    passCfg.writeEntry(Config::showFileContentAction, showFileContentAction);

    passCfg.deleteGroup(Config::Group::Actions);
    int i = 0;
    for (PassAction &act: this->ui->passActions()) {
        auto group = passCfg.group(Config::Group::Actions).group(QString::number(i++));
        act.writeToConfig(group);
    }
}

void PassConfig::defaults()
{
    KCModule::defaults();

    ui->lineTriggerWord->setText("pass");
    ui->checkEnableOtp->setChecked(true);
    ui->checkAdditionalActions->setChecked(false);
    ui->checkShowFileContentAction->setChecked(false);
    ui->clearPassActions();
    ui->clearInputsAndResetButton();
    ui->updateUiState();

#if KCMUTILS_VERSION >= QT_VERSION_CHECK(5, 64, 0)
    markAsChanged();
#else
    emit changed(true);
#endif
}

#include "config.moc"
