#ifndef USERSHORTCUTSPROVIDER_H
#define USERSHORTCUTSPROVIDER_H

#include "IActionProvider.h"
#include "UserAction.h"
#include <QVector>

class UserShortcutsProvider : public IActionProvider {
    Q_OBJECT
public:
    explicit UserShortcutsProvider(QObject *parent = nullptr);
    QString providerId() const override;
    QString displayName() const override;
    QVector<ActionItem> actions() const override;
    bool run(const QString &actionId) override;

    void loadFromConfig();
    void saveToConfig();
    void addAction(const UserAction &action);
    void removeAction(const QString &id);
    QVector<UserAction> userActions() const;

private:
    QString getConfigFilePath() const;
    static QString typeToConfigKey(UserActionType type);
    static UserActionType configKeyToType(const QString &key);

    QVector<UserAction> m_userActions;
};

#endif // USERSHORTCUTSPROVIDER_H
