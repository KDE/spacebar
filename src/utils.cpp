#include "utils.h"

#include <QQmlApplicationEngine>
#include <QQuickWindow>


Utils *Utils::s_instance = nullptr;


Utils::Utils(QQmlApplicationEngine *engine)
    : m_engine(engine)
{
    s_instance = this;
}

void Utils::showPassiveNotification(const QString &message, int timeout)
{
    auto *window = qobject_cast<QQuickWindow *>(m_engine->rootObjects().first());
    if (window) {
        QMetaObject::invokeMethod(window, "showPassiveNotification", Q_ARG(QVariant, message), Q_ARG(QVariant, timeout), Q_ARG(QVariant, {}), Q_ARG(QVariant, {}));
    }
}

Utils *Utils::instance()
{
    return s_instance;
}
