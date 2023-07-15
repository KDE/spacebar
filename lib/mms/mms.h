// SPDX-FileCopyrightText: 2021 Michael Lang <criticaltemp@protonmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QDateTime>
#include <QObject>

#include <span>

#include <global.h>
#include <phonenumberlist.h>

/*
 * Spec documentation:
 * http://www.openmobilealliance.org/release/MMS/V1_3-20110913-A/OMA-TS-MMS_ENC-V1_3-20110913-A.pdf
 *
 * Content types:
 * https://technical.openmobilealliance.org/OMNA/wsp/wsp_content_type_codes.html
 */

struct FieldMap {
    unsigned char id;
    QStringView name;
    QStringView type;
};

constexpr std::array<QStringView, 24> MESSAGE_TYPES = {
    u"m-send-req",         u"m-send-conf",     u"m-notification-ind", u"m-notifyresp-ind",  u"m-retrieve-conf",    u"m-acknowledge-ind",
    u"m-delivery-ind",     u"m-read-rec-ind",  u"m-read-orig-ind",    u"m-forward-req",     u"m-forward-conf",     u"m-mbox-store-req",
    u"m-mbox-store-conf",  u"m-mbox-view-req", u"m-mbox-view-conf",   u"m-mbox-upload-req", u"m-mbox-upload-conf", u"m-mbox-delete-req",
    u"m-mbox-delete-conf", u"m-mbox-descr",    u"m-delete-req",       u"m-delete-conf",     u"m-cancel-req",       u"m-cancel-conf"};

constexpr std::array<QStringView, 81> CONTENT_TYPES = {u"*/*",
                                                       u"text/*",
                                                       u"text/html",
                                                       u"text/plain",
                                                       u"text/x-hdml",
                                                       u"text/x-ttml",
                                                       u"text/x-vCalendar",
                                                       u"text/x-vCard",
                                                       u"text/vnd.wap.wml",
                                                       u"text/vnd.wap.wmlscript",
                                                       u"text/vnd.wap.wta-event",
                                                       u"multipart/*",
                                                       u"multipart/mixed",
                                                       u"multipart/form-data",
                                                       u"multipart/byteranges",
                                                       u"multipart/alternative",
                                                       u"application/*",
                                                       u"application/java-vm",
                                                       u"application/x-www-form-urlencoded",
                                                       u"application/x-hdmlc",
                                                       u"application/vnd.wap.wmlc",
                                                       u"application/vnd.wap.wmlscriptc",
                                                       u"application/vnd.wap.wta-eventc",
                                                       u"application/vnd.wap.uaprof",
                                                       u"application/vnd.wap.wtls-ca-certificate",
                                                       u"application/vnd.wap.wtls-user-certificate",
                                                       u"application/x-x509-ca-cert",
                                                       u"application/x-x509-user-cert",
                                                       u"image/*",
                                                       u"image/gif",
                                                       u"image/jpeg",
                                                       u"image/tiff",
                                                       u"image/png",
                                                       u"image/vnd.wap.wbmp",
                                                       u"application/vnd.wap.multipart.*",
                                                       u"application/vnd.wap.multipart.mixed",
                                                       u"application/vnd.wap.multipart.form-data",
                                                       u"application/vnd.wap.multipart.byteranges",
                                                       u"application/vnd.wap.multipart.alternative",
                                                       u"application/xml",
                                                       u"text/xml",
                                                       u"application/vnd.wap.wbxml",
                                                       u"application/x-x968-cross-cert",
                                                       u"application/x-x968-ca-cert",
                                                       u"application/x-x968-user-cert",
                                                       u"text/vnd.wap.si",
                                                       u"application/vnd.wap.sic",
                                                       u"text/vnd.wap.sl",
                                                       u"application/vnd.wap.slc",
                                                       u"text/vnd.wap.co",
                                                       u"application/vnd.wap.coc",
                                                       u"application/vnd.wap.multipart.related",
                                                       u"application/vnd.wap.sia",
                                                       u"text/vnd.wap.connectivity-xml",
                                                       u"application/vnd.wap.connectivity-wbxml",
                                                       u"application/pkcs7-mime",
                                                       u"application/vnd.wap.hashed-certificate",
                                                       u"application/vnd.wap.signed-certificate",
                                                       u"application/vnd.wap.cert-response",
                                                       u"application/xhtml+xml",
                                                       u"application/wml+xml",
                                                       u"text/css",
                                                       u"application/vnd.wap.mms-message",
                                                       u"application/vnd.wap.rollover-certificate",
                                                       u"application/vnd.wap.locc+wbxml",
                                                       u"application/vnd.wap.loc+xml",
                                                       u"application/vnd.syncml.dm+wbxml",
                                                       u"application/vnd.syncml.dm+xml",
                                                       u"application/vnd.syncml.notification",
                                                       u"application/vnd.wap.xhtml+xml",
                                                       u"application/vnd.wv.csp.cir",
                                                       u"application/vnd.oma.dd+xml",
                                                       u"application/vnd.oma.drm.message",
                                                       u"application/vnd.oma.drm.content",
                                                       u"application/vnd.oma.drm.rights+xml",
                                                       u"application/vnd.oma.drm.rights+wbxml",
                                                       u"application/vnd.wv.csp+xml ",
                                                       u"application/vnd.wv.csp+wbxml",
                                                       u"application/vnd.syncml.ds.notification",
                                                       u"audio/*",
                                                       u"video/* "};

constexpr std::array<QStringView, 8> STATUS_VALUES =
    {u"expired", u"retrieved", u"rejected", u"deferred", u"unrecognized", u"indeterminate", u"forwarded", u"unreachable"};

constexpr std::array<FieldMap, 63> HEADER_FIELDS = {FieldMap{0x81, u"bcc", u"encodedStringValue"},
                                                    {0x82, u"cc", u"encodedStringValue"},
                                                    {0x83, u"contentLocation", u"textString"},
                                                    {0x84, u"contentType", u"contentTypeValue"},
                                                    {0x85, u"date", u"dateValue"},
                                                    {0x86, u"deliveryReport", u"shortInteger"},
                                                    {0x87, u"deliveryTime", u"dateValue"},
                                                    {0x88, u"expiry", u"dateValue"},
                                                    {0x89, u"from", u"fromValue"},
                                                    {0x8A, u"messageClass", u"messageClassValue"},
                                                    {0x8B, u"messageId", u"textString"},
                                                    {0x8C, u"messageType", u"messageTypeValue"},
                                                    {0x8D, u"mmsVersion", u"mmsVersion"},
                                                    {0x8E, u"messageSize", u"longInteger"},
                                                    {0x8F, u"priority", u"shortInteger"},
                                                    {0x90, u"readReport", u"shortInteger"},
                                                    {0x91, u"reportAllowed", u"shortInteger"},
                                                    {0x92, u"responseStatus", u"shortInteger"},
                                                    {0x93, u"responseText", u"encodedStringValue"},
                                                    {0x94, u"senderVisibility", u"shortInteger"},
                                                    {0x95, u"status", u"shortInteger"},
                                                    {0x96, u"subject", u"encodedStringValue"},
                                                    {0x97, u"to", u"encodedStringValue"},
                                                    {0x98, u"transactionId", u"textString"},
                                                    {0x99, u"retrieveStatus", u"shortInteger"},
                                                    {0x9A, u"retrieveText", u"encodedStringValue"},
                                                    {0x9B, u"readStatus", u"shortInteger"},
                                                    {0x9C, u"replyCharging", u"shortInteger"},
                                                    {0x9D, u"replyChargingDeadline", u"fromValue"},
                                                    {0x9E, u"replyChargingId", u"textString"},
                                                    {0x9F, u"replyChargingSize", u"longInteger"},
                                                    {0xA0, u"previoulySentBy", u"encodedStringValue"},
                                                    {0xA1, u"previoulySentDate", u"longInteger"},
                                                    {0xA2, u"store", u"shortInteger"},
                                                    {0xA3, u"mmState", u"shortInteger"},
                                                    {0xA4, u"mmFlags", u"encodedStringValue"},
                                                    {0xA5, u"storeStatus", u"shortInteger"},
                                                    {0xA6, u"storeStatusText", u"encodedStringValue"},
                                                    {0xA7, u"stored", u"shortInteger"},
                                                    {0xA8, u"attributes", u"shortInteger"},
                                                    {0xA9, u"totals", u"shortInteger"},
                                                    {0xAA, u"mboxTotals", u"shortInteger"},
                                                    {0xAB, u"quotas", u"shortInteger"},
                                                    {0xAC, u"mboxQuotas", u"shortInteger"},
                                                    {0xAD, u"messageCount", u"shortInteger"},
                                                    {0xAE, u"content", u"encodedStringValue"},
                                                    {0xAF, u"start", u"shortInteger"},
                                                    {0xB0, u"additionalHeaders", u"shortInteger"},
                                                    {0xB1, u"distributionIndicator", u"shortInteger"},
                                                    {0xB2, u"elementDescription", u"textString"},
                                                    {0xB3, u"limit", u"shortInteger"},
                                                    {0xB4, u"recRetrievalMode", u"shortInteger"},
                                                    {0xB5, u"recdRetrievalModeText", u"encodedStringValue"},
                                                    {0xB6, u"statusText", u"encodedStringValue"},
                                                    {0xB7, u"applicId", u"textString"},
                                                    {0xB8, u"replyApplicId", u"textString"},
                                                    {0xB9, u"auxAppliInfo", u"textString"},
                                                    {0xBA, u"contentClass", u"shortInteger"},
                                                    {0xBB, u"drmContent", u"shortInteger"},
                                                    {0xBC, u"adaptationAllowed", u"shortInteger"},
                                                    {0xBD, u"replaceId", u"textString"},
                                                    {0xBE, u"cancelId", u"textString"},
                                                    {0xBF, u"cancelStatus", u"shortInteger"}};

constexpr std::array<FieldMap, 29> PARAM_FIELDS = {FieldMap{0x01, u"q", u"qValue"},         {0x02, u"charset", u"textString"},
                                                   {0x03, u"level", u"versionValue"},       {0x04, u"type", u"integerValue"},
                                                   {0x05, u"name", u"textString"},          {0x06, u"filename", u"encodedStringValue"},
                                                   {0x07, u"differences", u"fieldName"},    {0x08, u"padding", u"shortInteger"},
                                                   {0x09, u"typeSpec", u"textString"},      {0x0A, u"start", u"textString"},
                                                   {0x0B, u"startInfo", u"textString"},     {0x0C, u"comment", u"textString"},
                                                   {0x0D, u"domain", u"textString"},        {0x0E, u"maxAge", u"longInteger"},
                                                   {0x0F, u"path", u"textString"},          {0x10, u"secure", u""},
                                                   {0x11, u"sec", u"shortInteger"},         {0x12, u"mac", u"textValue"},
                                                   {0x13, u"creationDate", u"longInteger"}, {0x14, u"modificationDate", u"longInteger"},
                                                   {0x15, u"readDate", u"longInteger"},     {0x16, u"size", u"integerValue"},
                                                   {0x17, u"nameVal", u"textValue"},        {0x18, u"filenameVal", u"textValue"},
                                                   {0x19, u"startMulti", u"textValue"},     {0x1A, u"startInfoMulti", u"textValue"},
                                                   {0x1B, u"commentVal", u"textValue"},     {0x1C, u"domainVal", u"textValue"},
                                                   {0x1D, u"pathVal", u"textValue"}};

struct MmsMessage {
    QString messageType;
    QString transactionId;
    QString messageId;
    qint64 messageSize = 0;
    QDateTime expiry;
    QDateTime date = QDateTime::currentDateTime();
    QStringList to;
    QString from;
    QString subject;
    QString contentType;
    QString contentLocation;
    QString text;
    QString attachments;
    QString smil;
    QStringList partNames;
    int responseStatus = 0;
    QString responseText;

    // The following are not part of a mms. Stored here rather than passing extra params
    QString databaseId;
    PhoneNumberList phoneNumberList;
    PhoneNumber ownNumber;
};
Q_DECLARE_METATYPE(MmsMessage)

class Mms : public QObject
{
    Q_OBJECT

public:
    explicit Mms(QObject *parent = nullptr);

    QByteArray encodeNotifyResponse(const QString &transactionId, const QString &status);
    QByteArray encodeDeliveryAcknowledgement(const QString &transactionId);
    QByteArray encodeReadReport(const QString &messageId);
    QByteArray encodeCancelResponse(const QString &transactionId);
    void decodeNotification(MmsMessage &message, const QByteArray &data);
    void decodeConfirmation(MmsMessage &message, const QByteArray &data);
    void decodeMessage(MmsMessage &message, const QByteArray &data);
    void encodeMessage(MmsMessage &message, QByteArray &data, const QStringList &files, qint64 totalSize);
    static QString generateTransactionId();

private:
    bool decodeHeader(MmsMessage &message, const QByteArray &data, int &pos);
    QByteArray encodeHeaderPrefix(const QString &type, const QString &id, bool msgId = false);
    bool decodeMessageBody(MmsMessage &message, const QByteArray &data, int &pos);
    QString saveLocation(const QString &folderName, const QString &subFolderName = QString());
    bool saveData(const QByteArray &data, const QString &path);
    static QString generateRandomId();

    QString contentTypeValue(const QByteArray &data, int &pos, MmsMessage &message);
    QString messageTypeValue(const QByteArray &data, int &pos);
    unsigned int unsignedInt(const QByteArray &data, int &pos);
    quint64 longInteger(const QByteArray &data, int &pos);
    int shortInteger(const QByteArray &data, int &pos);
    int integerValue(const QByteArray &data, int &pos);
    int valueLength(const QByteArray &data, int &pos);
    QString textString(const QByteArray &data, int &pos, int len = -1);
    QString encodedStringValue(const QByteArray &data, int &pos);
    QString fromValue(const QByteArray &data, int &pos);
    QDateTime dateValue(const QByteArray &data, int &pos);
    QString messageClassValue(const QByteArray &data, int &pos);
    QString mmsVersion(const QByteArray &data, int &pos);
    QString lookupValString(const QByteArray &data, int &pos, std::span<const QStringView> list);

    QByteArray encodeUnsignedInt(unsigned int value);
    QByteArray encodeLongInteger(quint64 value);
    QByteArray encodeTextValue(const QString &value);
    QByteArray encodeEncodedStringValue(const QString &value);
    QByteArray encodeFromValue(const QString &value);
    QByteArray encodeValueFromList(const QString &value, std::span<const QStringView> list);
    QByteArray encodePart(const QByteArray &type, const QString &name, const QByteArray &body, const QByteArray &params = QByteArrayLiteral(""));
};
