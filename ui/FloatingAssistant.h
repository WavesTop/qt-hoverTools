#ifndef FLOATINGASSISTANT_H
#define FLOATINGASSISTANT_H

#include "ToolbarPanel.h"
#include "UserShortcutsProvider.h"
#include <QWidget>
#include <QPoint>

class QMenu;

class FloatingAssistant : public QWidget {
    Q_OBJECT
public:
    explicit FloatingAssistant(QWidget *parent = nullptr);
    ~FloatingAssistant() override;
    void showToolbar();
    void hideToolbar();
    void toggleToolbar();
    void addActionProvider(IActionProvider *provider);

protected:
    void showEvent(QShowEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void handleAddFolder();
    void handleAddExecutable();
    void handleAddScript();
    void updateToolbarPosition();
    void ensureStayOnTop();

    ToolbarPanel *m_toolbarPanel = nullptr;
    UserShortcutsProvider *m_userShortcutsProvider = nullptr;
    QMenu *m_contextMenu = nullptr;
    QPoint m_dragStartPos;
    bool m_isDragging = false;
    bool m_wasHiddenByUser = false;

    static const int kFloatingButtonSize = 44;
    static const int kToolbarGap = 2;
};

#endif // FLOATINGASSISTANT_H
