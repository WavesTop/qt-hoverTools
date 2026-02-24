#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QScreen>
#include <QStyle>
#include <QGuiApplication>
#include "FloatingAssistant.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("HoverTools"));
    app.setApplicationDisplayName(QStringLiteral("桌面悬浮助手"));

    FloatingAssistant assistant;
    QRect screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();
    assistant.move(screenGeometry.right() - 60, screenGeometry.bottom() - 60);
    assistant.show();
    assistant.raise();
    assistant.activateWindow();

    QObject::connect(&app, &QApplication::applicationStateChanged, [&assistant](Qt::ApplicationState state) {
        if (state == Qt::ApplicationActive) {
            assistant.show();
            assistant.raise();
            assistant.activateWindow();
        }
    });

    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        QSystemTrayIcon trayIcon(&app);
        trayIcon.setToolTip(QStringLiteral("桌面悬浮助手"));
        trayIcon.setIcon(app.style()->standardIcon(QStyle::SP_ComputerIcon));
        QMenu *trayContextMenu = new QMenu();
        trayContextMenu->addAction(QStringLiteral("显示"), [&assistant]() {
            assistant.show();
            assistant.raise();
            assistant.activateWindow();
        });
        trayContextMenu->addAction(QStringLiteral("退出"), &app, &QApplication::quit);
        trayIcon.setContextMenu(trayContextMenu);
        trayIcon.show();
        QObject::connect(&trayIcon, &QSystemTrayIcon::activated, [&assistant](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger) {
                assistant.show();
                assistant.raise();
                assistant.activateWindow();
            }
        });
    }

    return app.exec();
}
