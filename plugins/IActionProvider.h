#ifndef IACTIONPROVIDER_H
#define IACTIONPROVIDER_H

#include <QString>
#include <QIcon>
#include <QObject>
#include <QVariantMap>

struct ActionItem {
    QString id;
    QString displayName;
    QIcon icon;
    QString toolTip;
    QVariantMap userData;
};

class IActionProvider : public QObject {
    Q_OBJECT
public:
    explicit IActionProvider(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IActionProvider() = default;

    virtual QString providerId() const = 0;
    virtual QString displayName() const = 0;
    virtual QVector<ActionItem> actions() const = 0;
    virtual bool run(const QString &actionId) = 0;

signals:
    void actionsChanged();
};

#endif // IACTIONPROVIDER_H
