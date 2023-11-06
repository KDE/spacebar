// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "mms.h"
#include "settingsmanager.h"

#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardPaths>
#include <QUuid>

#include <random>

constexpr unsigned char MMS_CODE_END_STRING = 0x00;

constexpr unsigned char MMS_CODE_VERSION = 0x93; // v1.3

constexpr unsigned char MMS_CODE_NO = 0x80;
constexpr unsigned char MMS_CODE_YES = 0x81;

constexpr unsigned char MMS_CODE_MULTIPART_MIXED = 0xA3;
constexpr unsigned char MMS_CODE_MULTIPART_RELATED = 0xB3;

constexpr unsigned char MMS_CODE_ADDRESS_PRESENT_TOKEN = 0x80;
constexpr unsigned char MMS_CODE_INSERT_ADDRESS_TOKEN = 0x81;

constexpr unsigned char MMS_CODE_ABSOLUTE_TOKEN = 0x80;
constexpr unsigned char MMS_CODE_RELATIVE_TOKEN = 0x81;

constexpr unsigned char MMS_CODE_READ = 0x80;
constexpr unsigned char MMS_CODE_DELETE_WITHOUT_READ = 0x81;

constexpr unsigned char MMS_CODE_CANCEL_REQUEST_SUCCESSFULLY_RECEIVED = 0x80;

constexpr unsigned char MMS_CODE_PRIORITY_LOW = 0x80;
constexpr unsigned char MMS_CODE_PRIORITY_NORMAL = 0x81;
constexpr unsigned char MMS_CODE_PRIORITY_HIGH = 0x82;

constexpr unsigned char MMS_CODE_MESSAGE_CLASS_PERSONAL = 0x80;
constexpr unsigned char MMS_CODE_MESSAGE_CLASS_ADVERTISEMENT = 0x81;
constexpr unsigned char MMS_CODE_MESSAGE_CLASS_INFORMATIONAL = 0x82;
constexpr unsigned char MMS_CODE_MESSAGE_CLASS_AUTO = 0x83;

constexpr unsigned char MMS_HEADER_CANCEL_STATUS = 0xBF;
constexpr unsigned char MMS_HEADER_CONTENT_LOCATION = 0x83;
constexpr unsigned char MMS_HEADER_CONTENT_TYPE = 0x84;
constexpr unsigned char MMS_HEADER_DATE = 0x85;
constexpr unsigned char MMS_HEADER_DELIVERY_REPORT = 0x86;
constexpr unsigned char MMS_HEADER_DELIVERY_TIME = 0x87;
constexpr unsigned char MMS_HEADER_FROM = 0x89;
constexpr unsigned char MMS_HEADER_PRIORITY = 0x8F;
constexpr unsigned char MMS_HEADER_MESSAGE_CLASS = 0x8A;
constexpr unsigned char MMS_HEADER_MESSAGE_TYPE = 0x8C;
constexpr unsigned char MMS_HEADER_MESSAGE_ID = 0x8B;
constexpr unsigned char MMS_HEADER_READ_STATUS = 0x9B;
constexpr unsigned char MMS_HEADER_READ_REPORT = 0x90;
constexpr unsigned char MMS_HEADER_REPORT_ALLOWED = 0x91;
constexpr unsigned char MMS_HEADER_SENDER_VISIBILITY = 0x94;
constexpr unsigned char MMS_HEADER_STATUS = 0x95;
constexpr unsigned char MMS_HEADER_SUBJECT = 0x96;
constexpr unsigned char MMS_HEADER_TO = 0x97;
constexpr unsigned char MMS_HEADER_TRANSACTION_ID = 0x98;
constexpr unsigned char MMS_HEADER_VERSION = 0x8D;

constexpr unsigned char MMS_PARAM_NAME = 0x85;
constexpr unsigned char MMS_PARAM_START = 0x8A;
constexpr unsigned char MMS_PARAM_TYPE_SPEC = 0x89;

constexpr unsigned char MMS_PDU_TYPE_PUSH = 0x06;

Mms::Mms(QObject *parent)
    : QObject(parent)
{
}

QByteArray Mms::encodeNotifyResponse(const QString &transactionId, const QString &status)
{
    QByteArray data;
    data.append(encodeHeaderPrefix(SL("m-notifyresp-ind"), transactionId));

    data.append(MMS_HEADER_STATUS);
    data.append(encodeValueFromList(status, STATUS_VALUES));

    data.append(MMS_HEADER_REPORT_ALLOWED);
    data.append((SettingsManager::self()->shareDeliveryStatus() ? MMS_CODE_YES : MMS_CODE_NO));

    return data;
}

QByteArray Mms::encodeDeliveryAcknowledgement(const QString &transactionId)
{
    QByteArray data;
    data.append(encodeHeaderPrefix(SL("m-acknowledge-ind"), transactionId));

    data.append(MMS_HEADER_REPORT_ALLOWED);
    data.append((SettingsManager::self()->shareDeliveryStatus() ? MMS_CODE_YES : MMS_CODE_NO));

    return data;
}

// confirms the read status of the MM to the MMS Proxy-Relay - send when marking mms messages as read
QByteArray Mms::encodeReadReport(const QString &messageId)
{
    QByteArray data;
    data.append(encodeHeaderPrefix(SL("m-read-rec-ind"), messageId, true));

    data.append(MMS_HEADER_TO);
    // data.append();

    data.append(MMS_HEADER_FROM);
    // data.append();

    data.append(MMS_HEADER_READ_STATUS);
    data.append(MMS_CODE_READ); // MMS_CODE_DELETE_WITHOUT_READ

    // TODO: finish this
    return data;
}

// TODO add forwardMessage method - m-forward-req with handling of m-forward-conf - from, to, contentLocation, reportAllowed, deliveryReport, readReport
// TODO add deleteRequest method - m-delete-req with handling of m-delete-conf - contentLocation

QByteArray Mms::encodeCancelResponse(const QString &transactionId)
{
    QByteArray data;
    data.append(encodeHeaderPrefix(SL("m-cancel-conf"), transactionId));

    data.append(MMS_HEADER_CANCEL_STATUS);
    data.append(MMS_CODE_CANCEL_REQUEST_SUCCESSFULLY_RECEIVED);

    return data;
}

void Mms::decodeNotification(MmsMessage &message, const QByteArray &data)
{
    // Save copy for testing/debugging purposes. Delete later
    const QString folderTemp = saveLocation(SL("notifications"));
    const QString pathTemp = folderTemp + SL("/") + generateRandomId();
    saveData(data, pathTemp);

    int pos = 0;

    if (data.at(++pos) != MMS_PDU_TYPE_PUSH)
        return;

    int headerLen = unsignedInt(data, pos);

    if (data.at(pos) == 40) {
        // skip the open parenthesis
        pos += 2;
    }

    message.contentType = contentTypeValue(data, pos, message);
    if (message.contentType.isEmpty()) {
        qDebug() << "unknown content type";
        return;
    }

    pos += headerLen - pos;
    while (decodeHeader(message, data, pos))
        ;

    // use the message transaction id if url does not contain an id
    if (message.contentLocation.endsWith(SL("="))) {
        message.contentLocation.append(message.transactionId);
    }

    // remove if successfully decoded
    if (!message.transactionId.isEmpty()) {
        QFile file(pathTemp);
        file.remove();
    }
}

void Mms::decodeConfirmation(MmsMessage &message, const QByteArray &data)
{
    int pos = -1;
    while (decodeHeader(message, data, pos))
        ;

    // Save copy for testing/debugging purposes. Delete later
    if (!message.transactionId.isEmpty()) {
        const QString folder = saveLocation(SL("confirmations"));
        const QString path = folder + SL("/") + message.transactionId;
        saveData(data, path);
    }
}

void Mms::decodeMessage(MmsMessage &message, const QByteArray &data)
{
    // Save copy for testing/debugging purposes. Delete later
    const QString folderTemp = saveLocation(SL("downloads"));
    const QString pathTemp = folderTemp + SL("/") + generateRandomId();
    saveData(data, pathTemp);

    int pos = -1;
    while (decodeHeader(message, data, pos))
        ;

    // add from number and normalize numbers
    QStringList numbers = message.to;
    if (PhoneNumber(message.from).isValid()) {
        numbers.append(message.from);
    }
    numbers.sort();
    message.phoneNumberList = PhoneNumberList(numbers.join(u'~'));

    // remove own number if present
    const int index = message.phoneNumberList.indexOf(message.ownNumber);
    if (index >= 0) {
        message.phoneNumberList.removeAt(index);
    }

    if (message.contentType == SL("application/vnd.wap.multipart.*") || message.contentType == SL("application/vnd.wap.multipart.mixed")
        || message.contentType == SL("application/vnd.wap.multipart.form-data") || message.contentType == SL("application/vnd.wap.multipart.byteranges")
        || message.contentType == SL("application/vnd.wap.multipart.alternative") || message.contentType == SL("application/vnd.wap.multipart.related")) {
        decodeMessageBody(message, data, pos);
    }

    // remove if successfully decoded or if is an outgoing message
    if (!message.messageId.isEmpty() || !message.ownNumber.isValid() || PhoneNumber(message.from) == message.ownNumber) {
        QFile file(pathTemp);
        file.remove();
    }
}

void Mms::encodeMessage(MmsMessage &message, QByteArray &data, const QStringList &files, qint64 totalSize)
{
    auto settings = SettingsManager::self();
    int sizeLimit = settings->totalMaxAttachmentSize() * 1024;
    int parts = files.count() + (message.text.isEmpty() ? 0 : 1);

    data.append(encodeHeaderPrefix(SL("m-send-req"), generateTransactionId()));

    data.append(MMS_HEADER_DATE);
    data.append(encodeLongInteger(QDateTime::currentSecsSinceEpoch())); // optional

    data.append(MMS_HEADER_FROM);
    data.append(encodeFromValue(message.from.remove(SL("-")).remove(SL(" ")) + SL("/TYPE=PLMN")));
    message.from.clear();

    for (auto &number : message.to) {
        data.append(MMS_HEADER_TO);
        data.append(encodeEncodedStringValue(number.remove(SL("-")).remove(SL(" ")) + SL("/TYPE=PLMN")));
    }
    message.to.clear();

    // TODO add message setting for this
    // data.append(MMS_HEADER_SENDER_VISIBILITY);
    // data.append(MMS_CODE_YES);

    // TODO add message setting for this
    // data.append(MMS_HEADER_SUBJECT);
    // data.append(encodeEncodedStringValue(SL("NoSubject")));

    // TODO add message setting for this
    data.append(MMS_HEADER_MESSAGE_CLASS);
    data.append(MMS_CODE_MESSAGE_CLASS_PERSONAL);

    // TODO add message setting for this
    // data.append(MMS_HEADER_PRIORITY);
    // data.append(MMS_CODE_PRIORITY_NORMAL);

    // TODO add message setting for this
    // data.append(MMS_HEADER_DELIVERY_TIME);
    // data.append(encodeLongInteger(QDateTime::currentSecsSinceEpoch() + delaySeconds));

    data.append(MMS_HEADER_READ_REPORT);
    data.append((settings->requestReadReports() ? MMS_CODE_YES : MMS_CODE_NO));

    data.append(MMS_HEADER_DELIVERY_REPORT);
    data.append((settings->requestDeliveryReports() ? MMS_CODE_YES : MMS_CODE_NO));

    data.append(MMS_HEADER_CONTENT_TYPE);
    if (parts > 1 && settings->autoCreateSmil()) {
        data.append(0x1B);
        data.append(MMS_CODE_MULTIPART_RELATED);

        // add param fields
        data.append(MMS_PARAM_START);
        data.append(encodeTextValue(SL("<smil>"))); // start
        data.append(MMS_PARAM_TYPE_SPEC);
        data.append(encodeTextValue(SL("application/smil")));

        parts++; // for the added smil part
    } else {
        data.append(MMS_CODE_MULTIPART_MIXED);
    }

    // message body parts
    data.append(encodeUnsignedInt(parts));

    int smilInsertPos = data.length();
    QString smil;

    for (const auto &path : files) {
        QFile file(path.mid(6, path.length() - 6)); // remove "file://" prefix
        file.open(QIODevice::ReadOnly);
        QByteArray fileData = file.readAll();
        QString name = path.mid(path.lastIndexOf(SL("/")) + 1);
        QMimeDatabase db;
        QString mimeType = db.mimeTypeForData(fileData).name();

        /*
         * WSP Content Type codes do not have numeric identifiers for the
         * following media types. Use alternate supported media types instead.
         */
        if (mimeType.toLower() == SL("text/vcard")) {
            mimeType = SL("text/x-vCard");
        } else if (mimeType.toLower() == SL("text/calendar")) {
            mimeType = SL("text/x-vCalendar");
        }

        // resize/transcode if total size exceeds max size
        if (totalSize > sizeLimit) {
            if (mimeType.contains(SL("image")) && mimeType != SL("image/gif")) {
                name = SL("Resized_") + name;
                QImage image, imageModified;

                // conversion to jpg should reduce the size enough in most cases
                if (mimeType != SL("image/jpeg")) {
                    totalSize -= fileData.length();
                    imageModified.loadFromData(fileData);
                    fileData.clear();
                    QBuffer buffer(&fileData);
                    buffer.open(QIODevice::WriteOnly);
                    imageModified.save(&buffer, "JPG");
                    totalSize += fileData.length();

                    // update extension and mimeType
                    int extPos = name.lastIndexOf(SL("."));
                    name = name.remove(extPos, name.length() - extPos) + SL(".jpg");
                    mimeType = SL("image/jpeg");
                }

                const float ratio = (float)fileData.length() / totalSize;
                while (fileData.length() * ratio > sizeLimit * ratio / files.count()) {
                    image.loadFromData(fileData);
                    float factor = (float)fileData.length() / sizeLimit > 8 ? 2 : 1.1;
                    imageModified = image.scaled(image.width() / factor, image.height() / factor, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                    fileData.clear();
                    QBuffer buffer(&fileData);
                    buffer.open(QIODevice::WriteOnly);
                    imageModified.save(&buffer, "JPG");
                }
            }
            // TODO investigate if feasible to convert/transcode audio?
        }

        // shorten name and preserve extension
        if (name.length() > 27) {
            const int extPos = name.lastIndexOf(SL("."));
            const int extLen = name.length() - extPos;
            if (extLen < 5) {
                name = name.left(27 - extLen - 1) + name.right(extLen);
            } else {
                name = name.left(27);
            }
        }

        data.append(encodePart(encodeValueFromList(mimeType, CONTENT_TYPES), name, fileData));

        if (parts > 1 && settings->autoCreateSmil()) {
            smil.append(SL(R"(<par dur="5000ms">)"));

            if (mimeType.contains(SL("image"))) {
                smil.append(SL(R"(<img src=")"));
                smil.append(name);
                smil.append(SL(R"(" region="Image" />)"));
            }

            smil.append(SL(R"(</par>)"));
        }
    }

    // add text as one of the parts
    if (!message.text.isEmpty()) {
        const QString name = SL("text01.txt");
        data.append(encodePart(encodeValueFromList(SL("text/plain"), CONTENT_TYPES), name, message.text.toUtf8()));
        message.text.clear();

        if (parts > 1 && settings->autoCreateSmil()) {
            smil.remove(smil.length() - 6, 6);
            smil.append(SL(R"(<text src=")"));
            smil.append(name);
            smil.append(SL(R"(" region="Text" />)"));
            smil.append(SL(R"(</par>)"));
        }
    }

    if (parts > 1 && settings->autoCreateSmil()) {
        // standard layout - image and text, other files currently not included/supported
        smil.prepend(SL(
            R"(<smil><head><layout><root-layout/><region id="Image" fit="meet" top="0" left="0" height="80%" width="100%"/><region id="Text" top="80%" left="0" height="20%" width="100%"/></layout></head><body>)"));

        smil.append(SL(R"(</body></smil>)"));

        const QString name = SL("smil.xml");
        QByteArray params;
        params.append((unsigned char)0xC0);
        params.append(R"(")");
        params.append(encodeTextValue(SL("<smil>")));
        params.append((unsigned char)0x8E);
        params.append(encodeTextValue(SL("smil.xml")));
        QByteArray smilData = encodePart(encodeTextValue(SL("application/smil")), name, smil.toUtf8(), params);
        data.insert(smilInsertPos, smilData);
    }

    // decode message here so attachments get formatted the same as a message being sent
    decodeMessage(message, data);
}

bool Mms::decodeHeader(MmsMessage &message, const QByteArray &data, int &pos)
{
    if ((pos + 1) >= data.length()) {
        return false;
    }

    unsigned char field = data.at(++pos);
    for (const auto &headerField : HEADER_FIELDS) {
        if (headerField.id == field) {
            QVariant val;
            QStringView type = headerField.type;
            if (type == u"")
                val = data.at(++pos);
            else if (type == u"unsignedInt")
                val = unsignedInt(data, pos);
            else if (type == u"longInteger")
                val = longInteger(data, pos);
            else if (type == u"shortInteger")
                val = shortInteger(data, pos);
            else if (type == u"integerValue")
                val = integerValue(data, pos);
            else if (type == u"textString")
                val = textString(data, pos);
            else if (type == u"encodedStringValue")
                val = encodedStringValue(data, pos);
            else if (type == u"fromValue")
                val = fromValue(data, pos);
            else if (type == u"contentTypeValue")
                val = contentTypeValue(data, pos, message);
            else if (type == u"messageTypeValue")
                val = messageTypeValue(data, pos);
            else if (type == u"dateValue")
                val = dateValue(data, pos);
            else if (type == u"messageClassValue")
                val = messageClassValue(data, pos);
            else if (type == u"mmsVersion")
                val = mmsVersion(data, pos);

            // capture relevant values
            QStringView name = headerField.name;
            if (name == u"messageType")
                message.messageType = val.toString();
            else if (name == u"transactionId")
                message.transactionId = val.toString();
            else if (name == u"messageId")
                message.messageId = val.toString();
            else if (name == u"messageSize")
                message.messageSize = val.toInt();
            else if (name == u"expiry")
                message.expiry = val.toDateTime();
            else if (name == u"date")
                message.date = val.toDateTime();
            else if (name == u"to")
                message.to.append(PhoneNumber(val.toString().remove(QLatin1String("/TYPE=PLMN"))).toInternational());
            else if (name == u"from")
                message.from = PhoneNumber(val.toString().remove(QLatin1String("/TYPE=PLMN"))).toInternational();
            else if (name == u"subject")
                message.subject = val.toString();
            else if (name == u"contentType")
                message.contentType = val.toString();
            else if (name == u"contentLocation")
                message.contentLocation = val.toString();
            else if (name == u"responseStatus")
                message.responseStatus = val.toInt();
            else if (name == u"responseText")
                message.responseText = val.toString();
            // else qDebug() << headerField.name << ":" << val.toString();

            return true;
        }
    }

    pos--;
    return false;
}

QByteArray Mms::encodeHeaderPrefix(const QString &type, const QString &id, bool msgId)
{
    QByteArray data;
    data.append(MMS_HEADER_MESSAGE_TYPE);
    data.append(encodeValueFromList(type, MESSAGE_TYPES));

    data.append(msgId ? MMS_HEADER_MESSAGE_ID : MMS_HEADER_TRANSACTION_ID);
    data.append(encodeTextValue(id));

    data.append(MMS_HEADER_VERSION);
    data.append(MMS_CODE_VERSION);

    return data;
}

bool Mms::decodeMessageBody(MmsMessage &message, const QByteArray &data, int &pos)
{
    if ((pos + 1) >= data.length()) {
        return false;
    }

    int parts = unsignedInt(data, pos);

    // allow extra 2 parts for text and smil
    if (parts > SettingsManager::self()->maxAttachments() + 2) {
        qDebug() << "Max attachments exceeded! Parts:" << parts;
        return false;
    }

    const QString folder = QString::number(hash(message.phoneNumberList.toString()));
    const QString attachmentsFolder = saveLocation(SL("attachments"), folder);

    QJsonArray attachments;

    // the body header should not be confused with the actual mms header
    for (int i = 0; i < parts; i++) {
        int headerLen = unsignedInt(data, pos);
        int dataLen = unsignedInt(data, pos);

        int temp = pos;
        QString contentType = contentTypeValue(data, pos, message);
        pos = temp + 1;

        QByteArray fileData = data.mid(pos + headerLen, dataLen);
        pos += headerLen + dataLen - 1;

        if (contentType == SL("application/smil")) {
            message.smil += QString::fromUtf8(fileData);
            continue;
        } else {
            QMimeDatabase db;
            QMimeType mime = db.mimeTypeForData(fileData);
            QString fileName;
            QString path;
            QString text;
            QString name;

            if (contentType == SL("text/plain")) {
                if (parts == 1 || (parts == 2 && !message.smil.isEmpty())) {
                    // no other content so just treat like regular text
                    message.text.append(QString::fromUtf8(fileData));
                    if (parts == 2) {
                        message.smil.clear();
                    }
                    continue;
                } else {
                    text = QString::fromUtf8(fileData);
                }
            } else {
                // save file
                fileName = generateRandomId() + SL(".") + mime.preferredSuffix();
                path = attachmentsFolder + SL("/") + fileName;
                saveData(fileData, path);
            }

            name = message.partNames.length() > i ? message.partNames[i] : fileName;

            // json data for use in message page
            QJsonObject object{
                {SL("name"), name},
                {SL("fileName"), fileName},
                {SL("size"), dataLen},
                {SL("mimeType"), mime.name()},
                {SL("iconName"), mime.iconName()},
                {SL("text"), text},
            };

            attachments.append(object);
        }
    }

    if (attachments.size() > 0) {
        QJsonDocument jsonDoc;
        jsonDoc.setArray(attachments);
        QByteArray fileText = jsonDoc.toJson(QJsonDocument::Compact);
        message.attachments = QString::fromUtf8(fileText);
    }

    return false;
}

QString Mms::generateRandomId()
{
    QString intermediateId = SL("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
    std::shuffle(intermediateId.begin(), intermediateId.end(), std::mt19937(std::random_device()()));
    intermediateId.truncate(15);

    return intermediateId;
}

QString Mms::generateTransactionId()
{
    return SL("m-") + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString Mms::saveLocation(const QString &folderName, const QString &subFolderName)
{
    QString fileLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    fileLocation.append(SL("/spacebar/") + folderName);
    if (!subFolderName.isEmpty()) {
        fileLocation.append(SL("/") + subFolderName);
    }

    if (!QDir().mkpath(fileLocation)) {
        qDebug() << "Could not create the storage directory at" << fileLocation;
    }

    return fileLocation;
}

bool Mms::saveData(const QByteArray &data, const QString &path)
{
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();

    return true;
}

QString Mms::contentTypeValue(const QByteArray &data, int &pos, MmsMessage &message)
{
    QString type;
    if (data.at(pos + 1) & 0x80) {
        type = lookupValString(data, pos, CONTENT_TYPES);
    } else if (data.at(pos + 1) == 0 || data.at(pos + 1) >= 0x20) {
        type = textString(data, pos);
    } else {
        int len = valueLength(data, pos);
        int end = pos + len;

        // check for extension-media, next byte is between 32-127
        if (data.at(pos + 1) == 0 || (data.at(pos + 1) >= 32 && (unsigned char)data.at(pos + 1) <= 127)) {
            type = encodedStringValue(data, pos);
        } else if ((data.at(pos + 1) & 0x80) || data.at(pos + 1) <= 30) {
            type = lookupValString(data, pos, CONTENT_TYPES);
        }

        while (pos < end) {
            unsigned char field = data.at(++pos) & 0x7F;
            int len = PARAM_FIELDS.size();
            for (int i = 0; i < len; i++) {
                if (PARAM_FIELDS[i].id == field) {
                    QVariant val;
                    QStringView type = PARAM_FIELDS[i].type;
                    if (type == u"unsignedInt")
                        val = unsignedInt(data, pos);
                    else if (type == u"longInteger")
                        val = longInteger(data, pos);
                    else if (type == u"shortInteger")
                        val = shortInteger(data, pos);
                    else if (type == u"integerValue")
                        val = integerValue(data, pos);
                    else if (type == u"textString")
                        val = textString(data, pos);
                    else if (type == u"encodedStringValue")
                        val = encodedStringValue(data, pos);
                    else if (type == u"fromValue")
                        val = fromValue(data, pos);
                    else
                        val = data.at(++pos);

                    // store relevant values
                    QStringView name = PARAM_FIELDS[i].name;
                    if (name == u"name")
                        message.partNames.append(val.toString());
                    // else qDebug() << param_fields[i].name << ":" << val.toString();

                    i = len;
                }
            }
        }
    }

    return type;
}

QString Mms::messageTypeValue(const QByteArray &data, int &pos)
{
    return lookupValString(data, pos, MESSAGE_TYPES);
}

unsigned int Mms::unsignedInt(const QByteArray &data, int &pos)
{
    unsigned int unint = 0;
    unsigned int octet;
    char cont = 1;

    while (cont != 0) {
        unint = unint << 7;
        octet = data.at(++pos);
        unint += (octet & 0x7F);
        cont = (octet & 0x80);
    }

    return unint;
}

quint64 Mms::longInteger(const QByteArray &data, int &pos)
{
    int octetcount = data.at(++pos);

    if (octetcount > 30) {
        return 0;
    }

    quint64 value = 0;
    for (int i = 0; i < octetcount; i++) {
        value = value << 8;
        value += (unsigned char)data.at(++pos);
    }

    return value;
}

int Mms::shortInteger(const QByteArray &data, int &pos)
{
    return data.at(++pos) & 0x7F;
}

int Mms::integerValue(const QByteArray &data, int &pos)
{
    if (data.at(pos + 1) < 31) {
        return longInteger(data, pos);
    } else if ((unsigned char)data.at(pos + 1) > 127) {
        return shortInteger(data, pos);
    } else {
        pos++;
        qDebug() << "parse int error";
        return 0;
    }
}

int Mms::valueLength(const QByteArray &data, int &pos)
{
    if (data.at(pos + 1) < 31) {
        return data.at(++pos);
    } else if (data.at(pos + 1) == 31) {
        // length is an Uint
        pos++;
        return unsignedInt(data, pos);
    } else {
        qDebug() << "parse error";
        return 0;
    }
}

QString Mms::textString(const QByteArray &data, int &pos, int len)
{
    int length = 0, start = pos + 1;
    int size = len >= 0 ? len + pos + 0 : data.length();
    while ((pos + 1) < size && data.at(++pos))
        ++length;

    QByteArray text = data.mid(start, length);
    return QString::fromUtf8(text);
}

QString Mms::encodedStringValue(const QByteArray &data, int &pos)
{
    if (data.at(pos + 1) <= 31) {
        int len = valueLength(data, pos);
        pos++;
        return textString(data, pos, len);
    } else {
        return textString(data, pos);
    }
}

QString Mms::fromValue(const QByteArray &data, int &pos)
{
    int len = valueLength(data, pos);

    if ((unsigned char)data.at(pos + 1) == MMS_CODE_ADDRESS_PRESENT_TOKEN) {
        pos++;
        return encodedStringValue(data, pos);
    } else if ((unsigned char)data.at(pos + 1) == MMS_CODE_INSERT_ADDRESS_TOKEN) {
        pos++;
        return QString();
    } else {
        qDebug() << "No from token found";
        pos += (len == 0 ? 1 : (len & 0x7F));
        return QString();
    }
}

QDateTime Mms::dateValue(const QByteArray &data, int &pos)
{
    int len = data.at(++pos);
    quint64 value = 0;

    if ((unsigned char)data.at(pos + 1) == MMS_CODE_RELATIVE_TOKEN) {
        // relative token
        pos++;
        value = integerValue(data, pos);
        value = QDateTime::currentSecsSinceEpoch() + value;
    } else {
        int end = pos + len;
        // absolute token
        if ((unsigned char)data.at(pos + 1) == MMS_CODE_ABSOLUTE_TOKEN) {
            pos++;
        }
        while (pos < end) {
            value = value << 8;
            value += data.at(++pos);
        }
    }

    return QDateTime::fromSecsSinceEpoch(value);
}

QString Mms::messageClassValue(const QByteArray &data, int &pos)
{
    unsigned char value = data.at(++pos);
    if (value > 127 && value < 132) {
        // 128=personal (default), 129=advertisement, 130=informational, 131=auto
        return QString::number(value);
    } else {
        qDebug() << "Token-text";
        QVector<unsigned char> separators = {11, 32, 40, 41, 44, 47, 58, 59, 60, 61, 62, 63, 64, 91, 92, 93, 123, 125};

        QByteArray token;
        while (value > 31 && !separators.contains(value)) {
            token.append(value);
            value = data.at(++pos);
        }

        return QString::fromUtf8(token);
    }
}

QString Mms::mmsVersion(const QByteArray &data, int &pos)
{
    QString major = QString::number((data.at(pos + 1) & 0x70) >> 4);
    QString minor = QString::number(data.at(++pos) & 0x0F);
    return major + SL(".") + minor;
}

QString Mms::lookupValString(const QByteArray &data, int &pos, std::span<const QStringView> list)
{
    return list[shortInteger(data, pos)].toString();
}

QByteArray Mms::encodeUnsignedInt(unsigned int value)
{
    QByteArray bytes;

    bytes.append(value & 0x7F);
    value = value >> 7;
    while (value > 0) {
        bytes.prepend(0x80 | (value & 0x7F));
        value = value >> 7;
    }

    return bytes;
}

QByteArray Mms::encodeLongInteger(quint64 value)
{
    QByteArray bytes;
    while (value > 0) {
        bytes.prepend(value);
        value = value >> 8;
    }
    bytes.prepend(bytes.length());

    return bytes;
}

QByteArray Mms::encodeTextValue(const QString &value)
{
    QByteArray bytes;
    bytes.append(value.toUtf8());
    bytes.append(MMS_CODE_END_STRING);

    return bytes;
}

QByteArray Mms::encodeEncodedStringValue(const QString &value)
{
    QByteArray bytes;
    bytes.append(value.length() + 2);
    bytes.append(MMS_HEADER_CONTENT_LOCATION);
    bytes.append(value.toUtf8());
    bytes.append(MMS_CODE_END_STRING);

    return bytes;
}

QByteArray Mms::encodeFromValue(const QString &value)
{
    QByteArray bytes;
    if (value.length() == 0) {
        bytes.append(1);
        bytes.append(MMS_CODE_INSERT_ADDRESS_TOKEN);
    } else {
        QByteArray encodedAddress = encodeEncodedStringValue(value);
        bytes.append(encodedAddress.length() + 1);
        bytes.append(MMS_CODE_ADDRESS_PRESENT_TOKEN);
        bytes.append(encodedAddress);
    }

    return bytes;
}

QByteArray Mms::encodeValueFromList(const QString &value, std::span<const QStringView> list)
{
    QByteArray type;
    int index = list.size();
    while (index > 0) {
        index--;
        if (list[index] == value || index == 0) {
            type.append(index + 128);
            break;
        }
    }

    return type;
}

QByteArray Mms::encodePart(const QByteArray &type, const QString &name, const QByteArray &body, const QByteArray &params)
{
    QByteArray header;
    header.append(type);
    header.append(MMS_PARAM_NAME);
    header.append(encodeTextValue(name));
    header.prepend(encodeUnsignedInt(header.length()));
    header.append(params);

    QByteArray data;
    data.append(encodeUnsignedInt(header.length()));
    data.append(encodeUnsignedInt(body.length()));
    data.append(header);
    data.append(body);

    return data;
}
