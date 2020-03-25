#pragma once

#include <QObject>
class QQmlApplicationEngine;

class Utils : public QObject
{
    Q_OBJECT

public:
    explicit Utils(QQmlApplicationEngine *engine);
    void showPassiveNotification(const QString &message, int timeout = 0);

    static Utils *instance();

    static Utils *s_instance;

private:
    QQmlApplicationEngine *m_engine;
};
