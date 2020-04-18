#pragma once

#include <QObject>
#include <QRegularExpression>
class QQmlApplicationEngine;
class QQuickWindow;

class Utils : public QObject
{
    Q_OBJECT

public:
    enum PassiveNotificationDuation {
        ShortNotificationDuration,
        LongNotificationDuration
    };

    explicit Utils(QQmlApplicationEngine *engine);
    void showPassiveNotification(const QString &message, int timeout = 0);
    void showPassiveNotification(const QString &message, PassiveNotificationDuation timeout);

    Q_INVOKABLE bool isPhoneNumber(const QString text);

    static Utils *instance();

    static Utils *s_instance;

private:
    QQmlApplicationEngine *m_engine;
    QQuickWindow *m_window = nullptr;
    QRegularExpression m_phoneNumberRegex;
};
