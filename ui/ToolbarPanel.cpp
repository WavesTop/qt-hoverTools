#include "ToolbarPanel.h"
#include "UserShortcutsProvider.h"
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMenu>
#include <QApplication>
#include <QFont>

ToolbarPanel::ToolbarPanel(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground, false);
    setMinimumWidth(260);
    setMaximumHeight(420);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(8);

    m_actionListView = new QListWidget(this);
    m_actionListView->setIconSize(QSize(40, 40));
    m_actionListView->setSpacing(6);
    m_actionListView->setUniformItemSizes(false);
    m_actionListView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_actionListView->setMouseTracking(false);
    m_actionListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_actionListView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_actionListView->setMinimumHeight(120);
    QFont listFont = m_actionListView->font();
    listFont.setPointSize(listFont.pointSize() + 1);
    m_actionListView->setFont(listFont);
    m_actionListView->setStyleSheet(
        QStringLiteral("QListWidget::item { min-height: 48px; padding: 6px 8px; }")
    );
    connect(m_actionListView, &QListWidget::itemClicked, this, &ToolbarPanel::handleItemClicked);
    connect(m_actionListView, &QListWidget::customContextMenuRequested, this, &ToolbarPanel::showContextMenuForItem);

    m_addShortcutButton = new QPushButton(tr("添加快捷方式…"), this);
    connect(m_addShortcutButton, &QPushButton::clicked, this, &ToolbarPanel::handleAddButtonClicked);

    m_mainLayout->addWidget(m_actionListView, 1);
    m_mainLayout->addWidget(m_addShortcutButton, 0);
}

void ToolbarPanel::addProvider(IActionProvider *provider) {
    if (!provider || m_actionProviders.contains(provider)) {
        return;
    }
    m_actionProviders.append(provider);
    connect(provider, &IActionProvider::actionsChanged, this, &ToolbarPanel::handleProviderActionsChanged);
    refreshActions();
}

void ToolbarPanel::removeProvider(IActionProvider *provider) {
    if (!provider) {
        return;
    }
    disconnect(provider, &IActionProvider::actionsChanged, this, &ToolbarPanel::handleProviderActionsChanged);
    m_actionProviders.removeAll(provider);
    refreshActions();
}

void ToolbarPanel::refreshActions() {
    rebuildActionList();
}

void ToolbarPanel::rebuildActionList() {
    m_actionListView->clear();
    for (IActionProvider *provider : m_actionProviders) {
        if (!provider) {
            continue;
        }
        for (const ActionItem &actionItem : provider->actions()) {
            QListWidgetItem *listItem = new QListWidgetItem(actionItem.icon, actionItem.displayName);
            listItem->setData(kItemRoleProviderId, provider->providerId());
            listItem->setData(kItemRoleActionId, actionItem.id);
            listItem->setToolTip(actionItem.toolTip);
            m_actionListView->addItem(listItem);
        }
    }
}

void ToolbarPanel::handleItemClicked(QListWidgetItem *item) {
    if (!item) {
        return;
    }
    QString providerId = item->data(kItemRoleProviderId).toString();
    QString actionId   = item->data(kItemRoleActionId).toString();
    for (IActionProvider *provider : m_actionProviders) {
        if (!provider || provider->providerId() != providerId) {
            continue;
        }
        provider->run(actionId);
        emit actionRunRequested(providerId, actionId);
        break;
    }
}

void ToolbarPanel::handleAddButtonClicked() {
    QMenu menu(this);
    QAction *actionFolder = menu.addAction(tr("添加文件夹"));
    QAction *actionExec   = menu.addAction(tr("添加可执行文件"));
    QAction *actionScript = menu.addAction(tr("添加脚本"));
    QAction *chosen = menu.exec(m_addShortcutButton->mapToGlobal(m_addShortcutButton->rect().bottomLeft()));
    if (chosen == actionFolder) {
        emit addFolderRequested();
    } else if (chosen == actionExec) {
        emit addExecutableRequested();
    } else if (chosen == actionScript) {
        emit addScriptRequested();
    }
}

void ToolbarPanel::showContextMenuForItem(const QPoint &listLocalPos) {
    QListWidgetItem *item = m_actionListView->itemAt(listLocalPos);
    if (!item) {
        return;
    }
    QString providerId = item->data(kItemRoleProviderId).toString();
    QString actionId   = item->data(kItemRoleActionId).toString();
    if (providerId != QStringLiteral("user_shortcuts")) {
        return;
    }
    QMenu menu(this);
    QAction *deleteAction = menu.addAction(tr("删除"));
    if (menu.exec(m_actionListView->mapToGlobal(listLocalPos)) == deleteAction) {
        for (IActionProvider *provider : m_actionProviders) {
            UserShortcutsProvider *shortcutsProvider = qobject_cast<UserShortcutsProvider *>(provider);
            if (shortcutsProvider) {
                shortcutsProvider->removeAction(actionId);
                break;
            }
        }
    }
}

void ToolbarPanel::handleProviderActionsChanged() {
    refreshActions();
}
