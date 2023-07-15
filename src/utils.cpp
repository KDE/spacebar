// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "utils.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QRegularExpression>

#include <KIO/ApplicationLauncherJob>
#include <KPeople/PersonData>
#include <KPeopleBackend/AbstractContact>
#include <KTextToHTML>

#include <ranges>

#include "global.h"

Utils *Utils::s_instance = nullptr;

namespace ranges = std::ranges;

template<typename T>
static QVariantList toVariantList(const QVector<T> &v)
{
    QVariantList l;
    l.reserve(v.size());
    std::transform(v.begin(), v.end(), std::back_inserter(l), [](const T &elem) {
        return QVariant::fromValue(elem);
    });
    return l;
}

static QSharedPointer<KPeople::PersonData> contactData(const QString &uri)
{
    return QSharedPointer<KPeople::PersonData>(new KPeople::PersonData(uri));
}

Utils::Utils(QQmlApplicationEngine *engine)
    : m_engine(engine)
{
    s_instance = this;
}

void Utils::showPassiveNotification(const QString &message, int timeout)
{
    m_window = qobject_cast<QQuickWindow *>(m_engine->rootObjects().at(0));

    if (m_window) {
        QMetaObject::invokeMethod(m_window,
                                  "showPassiveNotification",
                                  Q_ARG(QVariant, message),
                                  Q_ARG(QVariant, timeout),
                                  Q_ARG(QVariant, {}),
                                  Q_ARG(QVariant, {}));
    }
}

void Utils::showPassiveNotification(const QString &message, Utils::PassiveNotificationDuation timeout)
{
    m_window = qobject_cast<QQuickWindow *>(m_engine->rootObjects().at(0));

    QString timeoutStr;
    switch (timeout) {
    case ShortNotificationDuration:
        timeoutStr = SL("short");
        break;
    case LongNotificationDuration:
        timeoutStr = SL("long");
        break;
    }
    if (m_window) {
        QMetaObject::invokeMethod(m_window,
                                  "showPassiveNotification",
                                  Q_ARG(QVariant, message),
                                  Q_ARG(QVariant, timeoutStr),
                                  Q_ARG(QVariant, {}),
                                  Q_ARG(QVariant, {}));
    }
}

bool Utils::isPhoneNumber(const QString &text) const
{
    static const QRegularExpression phoneNumberRegex(SL(R"(^[+]*[(]{0,1}[0-9]{1,4}[)]{0,1}[-\s\./0-9]*$)"));

    return phoneNumberRegex.match(text).hasMatch();
}

bool Utils::isPremiumNumber(const PhoneNumberList &phoneNumberList) const
{
    // premium SMS shortcodes across all countries with overly broad regexes removed
    // all of AOSP regexes combined: https://cs.android.com/android/platform/superproject/+/master:frameworks/base/core/res/res/xml/sms_short_codes.xml
    static const QRegularExpression premiumRegex(SL(
        R"##(^(15191|55[56]00|11[2456]1|3024|19998882|19944444|330[12]|87744|901[234]|93(?:94|101)|9426|9525|18(?:16|423)|19(?:1[56]|35)|3336|4161|444[4689]|501[34]|7781|60999|88188|43030|543|83111|30118|7510|9\d{6,7}|11(?:111|833)|1232(?:013|021|060|075|286|358)|118(?:44|80|86)|20[25]00|220(?:21|22|88|99)|221(?:14|21)|223(?:44|53|77)|224[13]0|225(?:20|59|90)|226(?:06|10|20|26|30|40|56|70)|227(?:07|33|39|66|76|78|79|88|99)|228(?:08|11|66|77)|23300|30030|3[12347]000|330(?:33|55|66)|33(?:233|331|366|533)|34(?:34|567)|37000|40(?:040|123|444|[3568]00)|41(?:010|414)|44(?:000|044|344|44[24]|544)|50005|50100|50123|50555|51000|52(?:255|783)|54(?:100|2542)|55(?:077|[24]00|222|333|55|[12369]55)|56(?:789|886)|60800|6[13]000|66(?:[12348]66|566|766|777|88|999)|68888|70(?:07|123|777)|76766|77(?:007|070|222|444|[567]77)|80(?:008|123|888)|82(?:002|[378]00|323|444|472|474|488|727)|83(?:005|[169]00|333|830)|84(?:141|300|32[34]|343|488|499|777|888)|85888|86(?:188|566|640|644|650|677|868|888)|870[24]9|871(?:23|[49]9)|872(?:1[0-8]|49|99)|87499|875(?:49|55|99)|876(?:0[1367]|1[1245678]|54|99)|877(?:00|99)|878(?:15|25|3[567]|8[12])|87999|880(?:08|44|55|77|99)|88688|888(?:03|10|8|89)|8899|90(?:009|999)|99999|1\d{3}|90\d{5}|15330|1701[0-3]|[23][57]\d{3}|280\d{2}|[79]9[57]\d{3}|0600.*|0700.*|171(?:59|63)|[4-8]\d{4}|[5-8]\d{4}|801[234]|888[239]|54\d{3}|19[0-5]\d{2}|0691227910|1784|5[3-9]\d{3}|4422|4545|4\d{4}|415[2367]|444[69]|335[02]|4161|444[469]|77[2359]0|8444|919[3-5]|968[2-5]|13[89]1|1394|16[34]5|6\d{4}|18(?:19|63|7[1-4])|53035|7766|32298|33776|4466|5040|2201|222[67]|3903|8995|4679|74240|79(?:10|866)|92525|6[1289]\d{3}|12(?:63|66|88)|13(?:14|80)|1(?:1[56]1|899)|2(?:09[57]|322|47[46]|880|990)|3[589]33|4161|44(?:4[3-9]|81)|77(?:33|81)|8424|72\d{3}|73800|[368]\d{3}|\d{4}|4\d{6}|11[3-7]1|4161|4333|444[689]|444[3-9]|70[579]4|7540|20433|21(?:344|472)|22715|23(?:333|847)|24(?:15|28)0|25209|27(?:449|606|663)|28498|305(?:00|83)|32(?:340|941)|33(?:166|786|849)|34746|35(?:182|564)|37975|38(?:135|146|254)|41(?:366|463)|42335|43(?:355|500)|44(?:578|711|811)|45814|46(?:157|173|327)|46666|47553|48(?:221|277|669)|50(?:844|920)|51(?:062|368)|52944|54(?:723|892)|55928|56483|57370|59(?:182|187|252|342)|60339|61(?:266|982)|62478|64(?:219|898)|65(?:108|500)|69(?:208|388)|70877|71851|72(?:078|087|465)|73(?:288|588|882|909|997)|74(?:034|332|815)|76426|79213|81946|83177|84(?:103|685)|85797|86(?:234|236|666)|89616|90(?:715|842|938)|91(?:362|958)|94719|95297|96(?:040|666|835|969)|97(?:142|294|688)|99(?:689|796|807))$)##"));

    bool isMatched = false;
    int i = phoneNumberList.size();
    while (!isMatched && i > 0) {
        i--;

        // remove country code, hyphens, and spaces
        auto numberString = phoneNumberList.at(i).toNational();
        numberString.replace(SL("-"), QString()).replace(SL(" "), QString());

        isMatched = premiumRegex.match(numberString).hasMatch();
    }

    return isMatched;
}

void Utils::launchPhonebook()
{
    auto phonebook = KService::serviceByDesktopName(QStringLiteral("org.kde.phonebook"));

    if (!phonebook) {
        qWarning() << "Could not find plasma-phonebook";
        return;
    }

    auto *job = new KIO::ApplicationLauncherJob(phonebook);
    job->start();
}

void Utils::copyTextToClipboard(const QString &text) const
{
    qGuiApp->clipboard()->setText(text);
}

QString Utils::textToHtml(const QString &text)
{
    return KTextToHTML::convertToHtml(text, KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::ConvertPhoneNumbers));
}

Utils *Utils::instance()
{
    return s_instance;
}

QQmlApplicationEngine *Utils::qmlEngine() const
{
    return m_engine;
}

PhoneNumber Utils::phoneNumber(const QString &number) const
{
    return PhoneNumber(number);
}

QString Utils::phoneNumberToInternationalString(const PhoneNumber &phoneNumber) const
{
    return phoneNumber.toInternational();
}

PhoneNumberList Utils::phoneNumberList(const QStringList &phoneNumbers) const
{
    auto p = phoneNumbers;
    std::sort(p.begin(), p.end());

    PhoneNumberList list;
    ranges::transform(p, std::back_inserter(list), [](const auto &phoneNumber) {
        return PhoneNumber(phoneNumber);
    });

    return list;
}

QString Utils::phoneNumberListToString(const PhoneNumberList &phoneNumberList) const
{
    return phoneNumberList.toString();
}

bool Utils::isLocale24HourTime()
{
    return QLocale::system().timeFormat(QLocale::ShortFormat).toLower().indexOf(SL("ap")) == -1;
}

QVariantList Utils::phoneNumbers(const QString &kPeopleUri)
{
    auto person = contactData(kPeopleUri);
    auto vcard = person->contactCustomProperty(KPeople::AbstractContact::VCardProperty).toByteArray();
    auto addressee = converter.parseVCard(vcard);

    return toVariantList(addressee.phoneNumbers());
}
