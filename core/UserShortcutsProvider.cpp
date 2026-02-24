#include "UserShortcutsProvider.h"
#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QUuid>

UserShortcutsProvider::UserShortcutsProvider(QObject *parent) : IActionProvider(parent) {
    loadFromConfig();
}

QString UserShortcutsProvider::providerId() const {
    return QStringLiteral("user_shortcuts");
}

QString UserShortcutsProvider::displayName() const {
    return tr("我的快捷方式");
}

QString UserShortcutsProvider::getConfigFilePath() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    return configDir + QStringLiteral("/shortcuts.ini");
}

QString UserShortcutsProvider::typeToConfigKey(UserActionType type) {
    switch (type) {
    case UserActionType::Folder:
        return QStringLiteral("folder");
    case UserActionType::Executable:
        return QStringLiteral("exec");
    case UserActionType::Script:
        return QStringLiteral("script");
    }
    return QStringLiteral("folder");
}

UserActionType UserShortcutsProvider::configKeyToType(const QString &key) {
    if (key == QStringLiteral("exec")) {
        return UserActionType::Executable;
    }
    if (key == QStringLiteral("script")) {
        return UserActionType::Script;
    }
    return UserActionType::Folder;
}

void UserShortcutsProvider::loadFromConfig() {
    m_userActions.clear();
    QSettings settings(getConfigFilePath(), QSettings::IniFormat);
    int count = settings.beginReadArray(QStringLiteral("actions"));
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        UserAction action;
        action.id = settings.value(QStringLiteral("id")).toString();
        action.displayName = settings.value(QStringLiteral("name")).toString();
        action.path = settings.value(QStringLiteral("path")).toString();
        action.type = configKeyToType(settings.value(QStringLiteral("type")).toString());
        if (action.id.isEmpty()) {
            action.id = QUuid::createUuid().toString(QUuid::Id128);
        }
        m_userActions.append(action);
    }
    settings.endArray();
    emit actionsChanged();
}

void UserShortcutsProvider::saveToConfig() {
    QSettings settings(getConfigFilePath(), QSettings::IniFormat);
    settings.beginWriteArray(QStringLiteral("actions"));
    for (int i = 0; i < m_userActions.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue(QStringLiteral("id"), m_userActions[i].id);
        settings.setValue(QStringLiteral("name"), m_userActions[i].displayName);
        settings.setValue(QStringLiteral("path"), m_userActions[i].path);
        settings.setValue(QStringLiteral("type"), typeToConfigKey(m_userActions[i].type));
    }
    settings.endArray();
    emit actionsChanged();
}

void UserShortcutsProvider::addAction(const UserAction &action) {
    UserAction newAction = action;
    if (newAction.id.isEmpty()) {
        newAction.id = QUuid::createUuid().toString(QUuid::Id128);
    }
    m_userActions.append(newAction);
    saveToConfig();
}

void UserShortcutsProvider::removeAction(const QString &id) {
    for (int i = 0; i < m_userActions.size(); ++i) {
        if (m_userActions[i].id == id) {
            m_userActions.removeAt(i);
            saveToConfig();
            return;
        }
    }
}

QVector<UserAction> UserShortcutsProvider::userActions() const {
    return m_userActions;
}

QVector<ActionItem> UserShortcutsProvider::actions() const {
    QVector<ActionItem> result;
    QFileIconProvider iconProvider;
    for (const UserAction &userAction : m_userActions) {
        ActionItem item;
        item.id = userAction.id;
        item.displayName = userAction.displayName.isEmpty()
            ? QFileInfo(userAction.path).fileName()
            : userAction.displayName;
        if (userAction.icon.isNull()) {
            QFileInfo fileInfo(userAction.path);
            item.icon = fileInfo.isDir() ? iconProvider.icon(QFileIconProvider::Folder)
                                         : iconProvider.icon(fileInfo);
        } else {
            item.icon = userAction.icon;
        }
        item.toolTip = userAction.path;
        item.userData.insert(QStringLiteral("path"), userAction.path);
        item.userData.insert(QStringLiteral("type"), static_cast<int>(userAction.type));
        result.append(item);
    }
    return result;
}

bool UserShortcutsProvider::run(const QString &actionId) {
    for (const UserAction &userAction : m_userActions) {
        if (userAction.id != actionId) {
            continue;
        }
        QString path = QDir::fromNativeSeparators(userAction.path);
        if (path.isEmpty()) {
            return false;
        }
        switch (userAction.type) {
        case UserActionType::Folder: {
            return QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
        case UserActionType::Executable:
        case UserActionType::Script: {
            QFileInfo fileInfo(path);
#if defined(Q_OS_MAC)
            if (path.endsWith(QStringLiteral(".app"), Qt::CaseInsensitive)) {
                return QProcess::startDetached(QStringLiteral("open"), { path });
            }
#endif
            QString workDir = fileInfo.absolutePath();
            QString ext = fileInfo.suffix().toLower();
            if (ext == QStringLiteral("sh") || ext == QStringLiteral("bash")) {
                return QProcess::startDetached(QStringLiteral("/bin/sh"), QStringList() << path, workDir);
            }
            if (ext == QStringLiteral("py") || ext == QStringLiteral("pyw")) {
                return QProcess::startDetached(QStringLiteral("python3"), QStringList() << path, workDir);
            }
            return QProcess::startDetached(path, {}, workDir);
        }
        }
        return false;
    }
    return false;
}
