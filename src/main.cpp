#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QUrl>
#include <KLocalizedContext>

#include "Global.h"
#include "chatlistmodel.h"
#include "contactmapper.h"


Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(SL("KDE"));
    QCoreApplication::setOrganizationDomain(SL("kde.org"));
    QCoreApplication::setApplicationName(SL("spacebear"));

    QQmlApplicationEngine engine;

    //auto beforeTime = QTime::currentTime();
//    ContactMapper mapper;
//    qDebug() << mapper.uriForNumber(SL("+49 163 39366399"));
    //auto afterTime = QTime::currentTime();

    //qDebug() << "TimeBefore" << beforeTime.msecsSinceStartOfDay();
    //qDebug() << "AfterTime" << afterTime.msecsSinceStartOfDay();
    //qDebug() << "Time taken:" << afterTime.msecsSinceStartOfDay() - beforeTime.msecsSinceStartOfDay();

    qmlRegisterType<ChatListModel>("org.kde.spacebear", 1, 0, "ChatListModel");
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
