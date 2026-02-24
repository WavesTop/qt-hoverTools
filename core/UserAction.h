#ifndef USERACTION_H
#define USERACTION_H

#include <QString>
#include <QIcon>

enum class UserActionType {
    Folder,
    Executable,
    Script
};

struct UserAction {
    QString id;
    QString displayName;
    QString path;
    UserActionType type = UserActionType::Folder;
    QIcon icon;
};

#endif // USERACTION_H
