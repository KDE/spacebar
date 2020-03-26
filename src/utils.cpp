#include "utils.h"

#include <QQmlApplicationEngine>
#include <QQuickWindow>

#include "global.h"

Utils *Utils::s_instance = nullptr;


Utils::Utils(QQmlApplicationEngine *engine)
    : m_engine(engine)

{
    s_instance = this;
}

void Utils::showPassiveNotification(const QString &message, int timeout)
{
    m_window = qobject_cast<QQuickWindow *>(m_engine->rootObjects().at(0));

    if (m_window) {
        QMetaObject::invokeMethod(m_window, "showPassiveNotification", Q_ARG(QVariant, message), Q_ARG(QVariant, timeout), Q_ARG(QVariant, {}), Q_ARG(QVariant, {}));
    }
}

void Utils::showPassiveNotification(const QString &message, Utils::PassiveNotificationDuation timeout)
{
    m_window = qobject_cast<QQuickWindow *>(m_engine->rootObjects().at(0));

    QString timeoutStr;
    switch(timeout) {
    case ShortNotificationDuration:
        timeoutStr = SL("short");
        break;
    case LongNotificationDuration:
        timeoutStr = SL("long");
        break;
    }
    if (m_window) {
        QMetaObject::invokeMethod(m_window, "showPassiveNotification", Q_ARG(QVariant, message), Q_ARG(QVariant, timeoutStr), Q_ARG(QVariant, {}), Q_ARG(QVariant, {}));
    }
}

Utils *Utils::instance()
{
    return s_instance;
}
