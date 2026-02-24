#ifndef TOOLBARPANEL_H
#define TOOLBARPANEL_H

#include "IActionProvider.h"
#include "UserAction.h"
#include <QWidget>
#include <QVector>
#include <QPointer>

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QVBoxLayout;

class ToolbarPanel : public QWidget {
    Q_OBJECT
public:
    explicit ToolbarPanel(QWidget *parent = nullptr);
    void addProvider(IActionProvider *provider);
    void removeProvider(IActionProvider *provider);
    void refreshActions();

signals:
    void addFolderRequested();
    void addExecutableRequested();
    void addScriptRequested();
    void actionRunRequested(const QString &providerId, const QString &actionId);

private:
    void rebuildActionList();
    void handleItemClicked(QListWidgetItem *item);
    void handleAddButtonClicked();
    void handleProviderActionsChanged();
    void showContextMenuForItem(const QPoint &listLocalPos);

    QListWidget *m_actionListView = nullptr;
    QPushButton *m_addShortcutButton = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
    QVector<QPointer<IActionProvider>> m_actionProviders;

    static const int kItemRoleProviderId = Qt::UserRole;
    static const int kItemRoleActionId   = Qt::UserRole + 1;
};

#endif // TOOLBARPANEL_H
