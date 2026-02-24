#include "FloatingAssistant.h"
#include <QMenu>
#include <QMouseEvent>
#include <QShowEvent>
#include <QPainter>
#include <QPainterPath>
#include <QFileDialog>
#include <QFileInfo>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QTimer>

FloatingAssistant::FloatingAssistant(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(kFloatingButtonSize, kFloatingButtonSize);

    m_userShortcutsProvider = new UserShortcutsProvider(this);
    m_toolbarPanel = new ToolbarPanel(nullptr);
    m_toolbarPanel->setAttribute(Qt::WA_DeleteOnClose, false);

    connect(m_toolbarPanel, &ToolbarPanel::addFolderRequested, this, &FloatingAssistant::handleAddFolder);
    connect(m_toolbarPanel, &ToolbarPanel::addExecutableRequested, this, &FloatingAssistant::handleAddExecutable);
    connect(m_toolbarPanel, &ToolbarPanel::addScriptRequested, this, &FloatingAssistant::handleAddScript);

    m_toolbarPanel->addProvider(m_userShortcutsProvider);

    m_contextMenu = new QMenu(this);
    m_contextMenu->addAction(tr("隐藏"), this, [this]() {
        m_wasHiddenByUser = true;
        hide();
    });
}

FloatingAssistant::~FloatingAssistant() {
    if (m_toolbarPanel) {
        m_toolbarPanel->close();
        m_toolbarPanel->deleteLater();
        m_toolbarPanel = nullptr;
    }
}

void FloatingAssistant::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    ensureStayOnTop();
    if (m_wasHiddenByUser) {
        m_wasHiddenByUser = false;
        QTimer::singleShot(50, this, [this]() { ensureStayOnTop(); });
    }
}

void FloatingAssistant::ensureStayOnTop() {
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    raise();
    activateWindow();
}

void FloatingAssistant::showToolbar() {
    updateToolbarPosition();
    m_toolbarPanel->refreshActions();
    m_toolbarPanel->show();
    m_toolbarPanel->raise();
    m_toolbarPanel->activateWindow();
}

void FloatingAssistant::hideToolbar() {
    m_toolbarPanel->hide();
}

void FloatingAssistant::toggleToolbar() {
    if (m_toolbarPanel->isVisible()) {
        hideToolbar();
    } else {
        showToolbar();
    }
}

void FloatingAssistant::updateToolbarPosition() {
    QPoint ballGlobalTopLeft = mapToGlobal(QPoint(0, 0));
    QRect ballRect(ballGlobalTopLeft, size());
    m_toolbarPanel->adjustSize();
    int toolbarWidth = m_toolbarPanel->width();
    int toolbarHeight = m_toolbarPanel->height();

    QScreen *screen = QGuiApplication::screenAt(ballRect.center());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    QRect availableGeometry = screen->availableGeometry();

    int spaceOnRight = availableGeometry.right() - ballRect.right();
    int spaceOnLeft = ballRect.left() - availableGeometry.left();
    bool showOnRight = spaceOnRight >= toolbarWidth || spaceOnRight >= spaceOnLeft;

    int toolbarX;
    if (showOnRight) {
        toolbarX = ballRect.right() + kToolbarGap;
    } else {
        toolbarX = ballRect.left() - toolbarWidth - kToolbarGap;
    }

    int toolbarY = ballRect.top();
    if (toolbarY + toolbarHeight > availableGeometry.bottom()) {
        toolbarY = availableGeometry.bottom() - toolbarHeight;
    }
    if (toolbarY < availableGeometry.top()) {
        toolbarY = availableGeometry.top();
    }

    m_toolbarPanel->move(toolbarX, toolbarY);
}

void FloatingAssistant::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragStartPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        m_isDragging = false;
    }
    QWidget::mousePressEvent(event);
}

void FloatingAssistant::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        if (!m_isDragging) {
            QPoint currentPos = event->globalPosition().toPoint();
            QPoint expectedPos = frameGeometry().topLeft() + m_dragStartPos;
            if (QPoint(currentPos - expectedPos).manhattanLength() > 4) {
                m_isDragging = true;
            }
        }
        if (m_isDragging) {
            move(event->globalPosition().toPoint() - m_dragStartPos);
            if (m_toolbarPanel->isVisible()) {
                updateToolbarPosition();
            }
        }
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void FloatingAssistant::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && !m_isDragging) {
        toggleToolbar();
    }
    QWidget::mouseReleaseEvent(event);
}

void FloatingAssistant::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(2, 2, width() - 4, height() - 4);
    painter.fillPath(path, QColor(60, 120, 200));
    painter.setPen(QColor(240, 248, 255));
    painter.setFont(QFont(QStringLiteral("Helvetica"), 12, QFont::Bold));
    painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("HT"));
}

void FloatingAssistant::contextMenuEvent(QContextMenuEvent *event) {
    m_contextMenu->popup(event->globalPos());
}

void FloatingAssistant::handleAddFolder() {
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("选择文件夹"));
    if (dirPath.isEmpty()) {
        return;
    }
    UserAction action;
    action.displayName = QFileInfo(dirPath).fileName();
    action.path = dirPath;
    action.type = UserActionType::Folder;
    m_userShortcutsProvider->addAction(action);
}

void FloatingAssistant::handleAddExecutable() {
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择可执行文件"), {},
#if defined(Q_OS_MAC)
        QStringLiteral("Applications (*.app);;All Files (*)")
#elif defined(Q_OS_WIN)
        QStringLiteral("Executables (*.exe);;All Files (*)")
#else
        QStringLiteral("All Files (*)")
#endif
    );
    if (filePath.isEmpty()) {
        return;
    }
    UserAction action;
    action.displayName = QFileInfo(filePath).fileName();
    action.path = filePath;
    action.type = UserActionType::Executable;
    m_userShortcutsProvider->addAction(action);
}

void FloatingAssistant::handleAddScript() {
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择脚本"), {},
        tr("Scripts (*.sh *.bash *.py *.pyw *.rb *.js);;All Files (*)"));
    if (filePath.isEmpty()) {
        return;
    }
    UserAction action;
    action.displayName = QFileInfo(filePath).fileName();
    action.path = filePath;
    action.type = UserActionType::Script;
    m_userShortcutsProvider->addAction(action);
}

void FloatingAssistant::addActionProvider(IActionProvider *provider) {
    if (provider) {
        m_toolbarPanel->addProvider(provider);
    }
}
