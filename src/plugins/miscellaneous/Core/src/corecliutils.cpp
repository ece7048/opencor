/*******************************************************************************

Copyright (C) The University of Auckland

OpenCOR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenCOR is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************/

//==============================================================================
// CLI utilities
//==============================================================================

#include "corecliutils.h"
#include "filemanager.h"

//==============================================================================

#include <QtMath>

//==============================================================================

#include <QChar>
#include <QCryptographicHash>
#include <QDir>
#include <QIODevice>
#include <QLocale>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkInterface>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QResource>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTextStream>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QXmlStreamReader>

//==============================================================================

#if defined(Q_OS_WIN)
    #include <Windows.h>
#elif defined(Q_OS_LINUX)
    #include <unistd.h>
#elif defined(Q_OS_MAC)
    #include <mach/host_info.h>
    #include <mach/mach_host.h>
    #include <sys/sysctl.h>
#endif

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

#include "corecliutils.cpp.inl"

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================

QBoolList qVariantListToBoolList(const QVariantList &pVariantList)
{
    // Convert the given list of variants to a list of booleans

    QBoolList res = QBoolList();

    foreach (const QVariant &variant, pVariantList)
        res << variant.toBool();

    return res;
}

//==============================================================================

QVariantList qBoolListToVariantList(const QBoolList &pBoolList)
{
    // Convert the given list of booleans to a list of variants

    QVariantList res = QVariantList();

    foreach (int nb, pBoolList)
        res << nb;

    return res;
}

//==============================================================================

QIntList qVariantListToIntList(const QVariantList &pVariantList)
{
    // Convert the given list of variants to a list of integers

    QIntList res = QIntList();

    foreach (const QVariant &variant, pVariantList)
        res << variant.toInt();

    return res;
}

//==============================================================================

QVariantList qIntListToVariantList(const QIntList &pIntList)
{
    // Convert the given list of integers to a list of variants

    QVariantList res = QVariantList();

    foreach (int nb, pIntList)
        res << nb;

    return res;
}

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

void DummyMessageHandler::handleMessage(QtMsgType pType,
                                        const QString &pDescription,
                                        const QUrl &pIdentifier,
                                        const QSourceLocation &pSourceLocation)
{
    Q_UNUSED(pType);
    Q_UNUSED(pDescription);
    Q_UNUSED(pIdentifier);
    Q_UNUSED(pSourceLocation);

    // We ignore the message...
}

//==============================================================================

quint64 totalMemory()
{
    // Retrieve and return in bytes the total amount of physical memory

    quint64 res = 0;

#if defined(Q_OS_WIN)
    MEMORYSTATUSEX memoryStatus;

    memoryStatus.dwLength = sizeof(memoryStatus);

    GlobalMemoryStatusEx(&memoryStatus);

    res = quint64(memoryStatus.ullTotalPhys);
#elif defined(Q_OS_LINUX)
    res = quint64(sysconf(_SC_PHYS_PAGES))*quint64(sysconf(_SC_PAGESIZE));
#elif defined(Q_OS_MAC)
    int mib[2];

    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;

    size_t len = sizeof(res);

    sysctl(mib, 2, &res, &len, 0, 0);
#else
    #error Unsupported platform
#endif

    return res;
}

//==============================================================================

quint64 freeMemory()
{
    // Retrieve and return in bytes the available amount of physical memory

    quint64 res = 0;

#if defined(Q_OS_WIN)
    MEMORYSTATUSEX memoryStatus;

    memoryStatus.dwLength = sizeof(memoryStatus);

    GlobalMemoryStatusEx(&memoryStatus);

    res = quint64(memoryStatus.ullAvailPhys);
#elif defined(Q_OS_LINUX)
    res = quint64(sysconf(_SC_AVPHYS_PAGES))*quint64(sysconf(_SC_PAGESIZE));
#elif defined(Q_OS_MAC)
    vm_statistics_data_t vmStats;
    mach_msg_type_number_t infoCount = HOST_VM_INFO_COUNT;

    host_statistics(mach_host_self(), HOST_VM_INFO,
                    host_info_t(&vmStats), &infoCount);

    res = (quint64(vmStats.free_count)+quint64(vmStats.inactive_count))*quint64(vm_page_size);
#else
    #error Unsupported platform
#endif

    return res;
}

//==============================================================================

QString digitGroupNumber(const QString &pNumber)
{
    // Digit group the given number (which we assume to be specified in the "C"
    // locale), if possible, and return the result of the digit grouping (which
    // is done keeping in mind the current locale)

    QString res = pNumber;
    bool validNumber;
    double number = res.toDouble(&validNumber);

    if (validNumber)
        res = QLocale().toString(number, 'g', 15);

    return res;
}

//==============================================================================

QString sizeAsString(double pSize, int pPrecision)
{
    // Note: pSize is a double rather than a quint64, in case we need to convert
    //       an insane size...

    QString units[9] = { QObject::tr("B"), QObject::tr("KB"), QObject::tr("MB"),
                         QObject::tr("GB"), QObject::tr("TB"), QObject::tr("PB"),
                         QObject::tr("EB"), QObject::tr("ZB"), QObject::tr("YB") };

    int i = qFloor(log(pSize)/log(1024.0));
    double size = pSize/qPow(1024.0, i);
    double scaling = qPow(10.0, pPrecision);

    size = qRound(scaling*size)/scaling;

    return QLocale().toString(size)+" "+units[i];
}

//==============================================================================

QString sha1(const QByteArray &pByteArray)
{
    // Return the SHA-1 value of the given byte array

    return QCryptographicHash::hash(pByteArray,
                                    QCryptographicHash::Sha1).toHex();
}

//==============================================================================

QString sha1(const QString &pString)
{
    // Return the SHA-1 value of the given string

    return sha1(pString.toUtf8());
}

//==============================================================================

void stringPositionAsLineColumn(const QString &pString, const QString &pEol,
                                int pPosition, int &pLine, int &pColumn)
{
    // Determine the line and column values of the given position within the
    // given string using the given end of line

    if ((pPosition < 0) || (pPosition >= pString.length())) {
        // Invalid position values

        pLine = -1;
        pColumn = -1;
    } else {
        pLine = pString.left(pPosition).count(pEol)+1;
        pColumn = pPosition-((pPosition >= pEol.length())?pString.lastIndexOf(pEol, pPosition-pEol.length()):-1);
    }
}

//==============================================================================

void stringLineColumnAsPosition(const QString &pString, const QString &pEol,
                                int pLine, int pColumn, int &pPosition)
{
    // Determine the position value of the given line and column within the
    // given string using the given end of line

    if ((pLine < 1) || (pColumn < 1)) {
        // Invalid line and/or colunn values

        pPosition = -1;
    } else {
        pPosition = 0;

        for (int i = 1; i < pLine; ++i) {
            int pos = pString.indexOf(pEol, pPosition);

            if (pos == -1) {
                // Invalid line value

                pPosition = -1;

                return;
            } else {
                pPosition = pos+pEol.length();
            }
        }

        pPosition += pColumn-1;

        // Make sure that the column value is actually valid by trying to
        // retrieve the line and column values from our computed position

        int testLine;
        int testColumn;

        stringPositionAsLineColumn(pString, pEol, pPosition, testLine, testColumn);

        if ((testLine != pLine) || (testColumn != pColumn))
            pPosition = -1;
    }
}

//==============================================================================

void * globalInstance(const QString &pObjectName, void *pDefaultGlobalInstance)
{
    // Retrieve and return the 'global' instance of an object
    // Note: initially, the plan was to have a static instance of an object and
    //       return its address. However, this approach doesn't work on Windows
    //       and Linux (but does on macOS). Indeed, say that the Core plugin is
    //       required by two other plugins, then these two plugins won't get the
    //       same 'copy' of the Core plugin. (It seems like) each 'copy' gets
    //       its own address space. (This is not the case on macOS, (most
    //       likely) because of the way applications are bundled on that
    //       platform.) So, to address this issue, we keep track of the address
    //       of a 'global' instance as a qApp property...

    QByteArray objectName = pObjectName.toUtf8().constData();
    QVariant res = qApp->property(objectName);

    if (!res.isValid()) {
        // There is no 'global' instance associated with the given object, so
        // use the object's default 'global' instance we were given, if any

        res = quint64(pDefaultGlobalInstance);

        qApp->setProperty(objectName, res);
    }

    return (void *) res.toULongLong();
}

//==============================================================================

static const auto SettingsActiveDirectory = QStringLiteral("ActiveDirectory");

//==============================================================================

QString activeDirectory()
{
    // Retrieve and return the active directory

    return QSettings().value(SettingsActiveDirectory, QDir::homePath()).toString();
}

//==============================================================================

void setActiveDirectory(const QString &pDirName)
{
    // Keep track of the active directory

    QSettings().setValue(SettingsActiveDirectory, pDirName);
}

//==============================================================================

bool isDirectory(const QString &pDirName)
{
    // Return whether the given directory exists

    return !pDirName.isEmpty() && QDir(pDirName).exists();
}

//==============================================================================

bool isEmptyDirectory(const QString &pDirName)
{
    // Return whether the given directory exists and is empty

    if (pDirName.isEmpty()) {
        return false;
    } else {
        QDir dir(pDirName);

        return     dir.exists()
               && !dir.entryInfoList(QDir::AllEntries|QDir::System|QDir::Hidden|QDir::NoDotAndDotDot).count();
    }
}

//==============================================================================

#ifdef Q_OS_WIN
    #pragma optimize("", off)
#endif

void doNothing(int pMax)
{
    // A silly function, which aim is simply to do nothing
    // Note: this function came about because there is no way, on Windows, to
    //       pause a thread for less than a millisecond (and this is in the best
    //       of cases)...

    for (int i = 0; i < pMax; ++i)
#ifdef Q_OS_WIN
        ;
#else
        asm("nop");
#endif
}

#ifdef Q_OS_WIN
    #pragma optimize("", on)
#endif

//==============================================================================

void checkFileNameOrUrl(const QString &pInFileNameOrUrl, bool &pOutIsLocalFile,
                        QString &pOutFileNameOrUrl)
{
    // Determine whether pInFileNameOrUrl refers to a local or a remote file,
    // and set pOutIsLocalFile and pOutFileNameOrUrl accordingly
    // Note #1: to use QUrl::isLocalFile() is not enough. Indeed, say that
    //          pInFileNameOrUrl is equal to
    //              /home/me/mymodel.cellml
    //          then QUrl(pInFileNameOrUrl).isLocalFile() will be false. For it
    //          to be true, we would have to initialise the QUrl object using
    //          QUrl::fromLocalFile(), but we can't do that since we don't know
    //          whether pInFileNameOrUrl refers to a local file or not. So,
    //          instead we test for the scheme and host of the QUrl object...
    // Note #2: a local file can be passed as a URL. For example,
    //              file:///home/me/mymodel.cellml
    //          is a URL, but effectively a local file, hence pOutIsLocalFile is
    //          to be true and pOutFileNameOrUrl is to be set to
    //              /home/me/mymodel.cellml
    //          However, to use fileNameOrUrl.toLocalFile() to retrieve that
    //          file won't work with a path that contains spaces, hence we
    //          return pInFileNameOrUrl after having removed "file:///" or
    //          "file://" from it on Windows and Linux/macOS, respectively...

    QUrl fileNameOrUrl = pInFileNameOrUrl;

    pOutIsLocalFile =    !fileNameOrUrl.scheme().compare("file")
                      ||  fileNameOrUrl.host().isEmpty();
    pOutFileNameOrUrl = pOutIsLocalFile?
#ifdef Q_OS_WIN
                            canonicalFileName(QString(pInFileNameOrUrl).remove("file:///")):
#else
                            canonicalFileName(QString(pInFileNameOrUrl).remove("file://")):
#endif
                            fileNameOrUrl.url();
}

//==============================================================================

QString formatXml(const QString &pXml)
{
    // Format the given XML

    QDomDocument domDocument;

    if (domDocument.setContent(pXml))
        return serialiseDomDocument(domDocument);
    else
        return QString();
}

//==============================================================================

void cleanContentMathml(QDomElement pDomElement)
{
    // Clean up the current element
    // Note: the idea is to remove all the attributes that are not in the
    //       MathML namespace. Indeed, if we were to leave them in then the XSL
    //       transformation would either do nothing or, worst, crash OpenCOR...

    QDomNamedNodeMap attributes = pDomElement.attributes();
    QList<QDomNode> nonMathmlAttributes = QList<QDomNode>();

    for (int i = 0, iMax = attributes.count(); i < iMax; ++i) {
        QDomNode attribute = attributes.item(i);

        if (attribute.localName().compare(attribute.nodeName()))
            nonMathmlAttributes << attribute;
    }

    foreach (QDomNode nonMathmlAttribute, nonMathmlAttributes)
        pDomElement.removeAttributeNode(nonMathmlAttribute.toAttr());

    // Go through the element's child elements, if any, and clean them up

    for (QDomElement childElement = pDomElement.firstChildElement();
         !childElement.isNull(); childElement = childElement.nextSiblingElement()) {
        cleanContentMathml(childElement);
    }
}

//==============================================================================

QString cleanContentMathml(const QString &pContentMathml)
{
    // Clean up and return the given MathML string

    QDomDocument domDocument;

    if (domDocument.setContent(pContentMathml, true)) {
        cleanContentMathml(domDocument.documentElement());

        return domDocument.toString(-1);
    } else {
        return QString();
    }
}

//==============================================================================

void cleanPresentationMathml(QDomElement pDomElement)
{
    // Merge successive child mrow elements, as long as their parent is not an
    // element that requires a specific number of arguments (which could become
    // wrong if we were to merge two successive mrow elements)
    // Note: see http://www.w3.org/TR/MathML2/chapter3.html#id.3.1.3.2 for the
    //       list of the elements to check...

    if (   pDomElement.nodeName().compare("mfrac")
        && pDomElement.nodeName().compare("mroot")
        && pDomElement.nodeName().compare("msub")
        && pDomElement.nodeName().compare("msup")
        && pDomElement.nodeName().compare("msubsup")
        && pDomElement.nodeName().compare("munder")
        && pDomElement.nodeName().compare("mover")
        && pDomElement.nodeName().compare("munderover")
        && pDomElement.nodeName().compare("munderover")
        && pDomElement.nodeName().compare("munderover")
        && pDomElement.nodeName().compare("munderover")
        && pDomElement.nodeName().compare("munderover")
        && pDomElement.nodeName().compare("munderover")) {
        for (QDomElement childElement = pDomElement.firstChildElement();
             !childElement.isNull(); childElement = childElement.nextSiblingElement()) {
            QDomElement nextChildElement = childElement.nextSiblingElement();

            if (   !nextChildElement.isNull()
                && !childElement.nodeName().compare("mrow")
                && !childElement.nodeName().compare(nextChildElement.nodeName())) {
                // The current and next child elements are both mrow's, so merge
                // them together

                for (QDomElement nextChildChildElement = nextChildElement.firstChildElement();
                     !nextChildChildElement.isNull(); nextChildChildElement = nextChildElement.firstChildElement()) {
                    childElement.appendChild(nextChildChildElement);
                }

                pDomElement.removeChild(nextChildElement);
            }
        }
    }

    // Recursively clean ourselves

    for (QDomElement childElement = pDomElement.firstChildElement();
         !childElement.isNull(); childElement = childElement.nextSiblingElement()) {
        cleanPresentationMathml(childElement);
    }

    // Move the contents of child mrow elements to their parent, should it also
    // be an mrow element
    // Note: we do this after having recursively cleaned ourselves to make sure
    //       that we also take into account the root element, in case it's an
    //       mrow element and the contents of its mrow child elements have been
    //       moved to it...

    if (!pDomElement.nodeName().compare("mrow")) {
        for (QDomElement childElement = pDomElement.firstChildElement();
             !childElement.isNull(); ) {
            QDomElement nextChildElement = childElement.nextSiblingElement();

            if (!childElement.nodeName().compare("mrow")) {
                // The current child element is an mrow, so move its contents to
                // its parent

                for (QDomElement childChildElement = childElement.firstChildElement();
                     !childChildElement.isNull(); childChildElement = childElement.firstChildElement()) {
                    pDomElement.insertBefore(childChildElement, childElement);
                }

                pDomElement.removeChild(childElement);
            }

            childElement = nextChildElement;
        }
    }
}

//==============================================================================

QString cleanPresentationMathml(const QString &pPresentationMathml)
{
    // Clean the given Presentation MathML

    QDomDocument domDocument;

    if (domDocument.setContent(pPresentationMathml)) {
        cleanPresentationMathml(domDocument.documentElement());

        return domDocument.toString(-1);
    } else {
        return QString();
    }
}

//==============================================================================

QString newFileName(const QString &pFileName, const QString &pExtra,
                    bool pBefore, const QString &pFileExtension)
{
    // Return the name of a 'new' file
    // Note: see Tests::newFileNameTests() for what we want to be able to get...

    FileManager *fileManagerInstance = FileManager::instance();
    QString fileName = fileManagerInstance->isRemote(pFileName)?
                           fileManagerInstance->url(pFileName):
                           pFileName;
    QFileInfo fileInfo = fileName;
    QString fileCanonicalPath = fileInfo.canonicalPath();
    QString fileBaseName = fileInfo.baseName();
    QString fileCompleteSuffix = pFileExtension.isEmpty()?
                                     fileInfo.completeSuffix():
                                     pFileExtension;

    if (!fileCanonicalPath.compare("."))
        fileCanonicalPath = QString();
    else
        fileCanonicalPath += "/";

    if (!fileCompleteSuffix.isEmpty())
        fileCompleteSuffix.prepend('.');

    static const QString Space = " ";
    static const QString Hyphen = "-";
    static const QString Underscore = "_";

    int nbOfSpaces = fileBaseName.count(Space);
    int nbOfHyphens = fileBaseName.count(Hyphen);
    int nbOfUnderscores = fileBaseName.count(Underscore);

    if (pExtra.isEmpty()) {
        return fileCanonicalPath+fileBaseName+fileCompleteSuffix;
    } else {
        static const QRegularExpression InitialCapitalLetterRegEx = QRegularExpression("^\\p{Lu}");

        QString separator = ((nbOfSpaces >= nbOfHyphens) && (nbOfSpaces >= nbOfUnderscores))?
                                Space+Hyphen+Space:
                                ((nbOfUnderscores >= nbOfSpaces) && (nbOfUnderscores >= nbOfHyphens))?
                                    Underscore:
                                    Hyphen;
        QString extra = pExtra;

        if (!InitialCapitalLetterRegEx.match(fileBaseName).hasMatch())
            extra[0] = extra[0].toLower();

        if (pBefore)
            return fileCanonicalPath+extra+separator+fileBaseName+fileCompleteSuffix;
        else
            return fileCanonicalPath+fileBaseName+separator+extra+fileCompleteSuffix;
    }

    return fileName;
}

//==============================================================================

QString newFileName(const QString &pFileName, const QString &pExtra,
                    bool pBefore)
{
    // Return the name of a 'new' file

    return newFileName(pFileName, pExtra, pBefore, QString());
}

//==============================================================================

QString newFileName(const QString &pFileName, const QString &pFileExtension)
{
    // Return the name of a 'new' file

    return newFileName(pFileName, QString(), true, pFileExtension);
}

//==============================================================================

bool validXml(const QByteArray &pXml, const QByteArray &pSchema)
{
    // Validate the given XML against the given schema

    QXmlSchema schema;
    DummyMessageHandler dummyMessageHandler;

    schema.setMessageHandler(&dummyMessageHandler);

    schema.load(pSchema);

    QXmlSchemaValidator validator(schema);

    return validator.validate(pXml);
}

//==============================================================================

bool validXmlFile(const QString &pXmlFileName, const QString &pSchemaFileName)
{
    // Validate the given XML file against the given schema file

    QByteArray xmlContents;
    QByteArray schemaContents;

    readFileContentsFromFile(pXmlFileName, xmlContents);
    readFileContentsFromFile(pSchemaFileName, schemaContents);

    return validXml(xmlContents, schemaContents);
}

//==============================================================================

bool sortSerialisedAttributes(const QString &pSerialisedAttribute1,
                              const QString &pSerialisedAttribute2)
{
    // Determine which of the two serialised attributes should be first based on
    // the attribute name, i.e. ignoring the "=<AttributeValue>" bit

    return pSerialisedAttribute1.left(pSerialisedAttribute1.indexOf('=')).compare(pSerialisedAttribute2.left(pSerialisedAttribute2.indexOf('=')), Qt::CaseInsensitive) < 0;
}

//==============================================================================

void cleanDomElement(QDomElement &pDomElement,
                     QMap<QString, QByteArray> &pElementsAttributes)
{
    // Serialise all the element's attributes and sort their serialised version
    // before removing them from the element and adding a new attribute that
    // will later on be used for string replacement

    static quint64 attributeNumber = 0;
    static const int ULLONG_WIDTH = ceil(log(ULLONG_MAX));

    if (pDomElement.hasAttributes()) {
        QStringList serialisedAttributes = QStringList();
        QDomNamedNodeMap domElementAttributes = pDomElement.attributes();

        while (domElementAttributes.count()) {
            // Serialise (ourselves) the element's attribute
            // Note: to rely on QDomNode::save() to do the serialisation isn't
            //       good enough. Indeed, if it is going to be fine for an
            //       attribute that doesn't have a prefix, e.g.
            //           name="my_name"
            //       it may not be fine for an attribute with a prefix, e.g.
            //           cmeta:id="my_cmeta_id"
            //       since depending on how that attribute has been created
            //       (i.e. using QDomDocument::createAttribute() or
            //       QDomDocument::createAttributeNS()), then it may or not have
            //       a namespace associated with it. If it does, then its
            //       serialisation will look something like
            //           cmeta:id="my_cmeta_id" xmlns:cmeta="http://www.cellml.org/metadata/1.0#"
            //       which is clearly not what we want since that's effectively
            //       two attributes in one. So, we need to separate them, which
            //       is what we do here, after making sure that the namespace
            //       for the attribute is not already defined for the given DOM
            //       element...

            QDomAttr attributeNode = domElementAttributes.item(0).toAttr();

            if (attributeNode.namespaceURI().isEmpty()) {
                serialisedAttributes << attributeNode.name()+"=\""+attributeNode.value().toHtmlEscaped()+"\"";
            } else {
                serialisedAttributes << attributeNode.prefix()+":"+attributeNode.name()+"=\""+attributeNode.value().toHtmlEscaped()+"\"";

                if (   attributeNode.prefix().compare(pDomElement.prefix())
                    && attributeNode.namespaceURI().compare(pDomElement.namespaceURI())) {
                    serialisedAttributes << "xmlns:"+attributeNode.prefix()+"=\""+attributeNode.namespaceURI()+"\"";
                }
            }

            // Remove the attribute node from the element

            pDomElement.removeAttributeNode(attributeNode);
        }

        // Sort the serialised attributes, using the attributes' name, and
        // remove duplicates, if any

        std::sort(serialisedAttributes.begin(), serialisedAttributes.end(),
                  sortSerialisedAttributes);

        serialisedAttributes.removeDuplicates();

        // Keep track of the serialisation of the element's attribute

        QString elementAttributes = QString("Element%1Attributes").arg(++attributeNumber, ULLONG_WIDTH, 10, QChar('0'));

        pElementsAttributes.insert(elementAttributes, serialisedAttributes.join(' ').toUtf8());

        // Add a new attribute to the element
        // Note: this attribute, once serialised by QDomDocument::save(), will
        //       be used to do a string replacement (see
        //       serialiseDomDocument())...

        domElementAttributes.setNamedItem(pDomElement.ownerDocument().createAttribute(elementAttributes));
    }

    // Recursively clean ourselves

    for (QDomElement childElement = pDomElement.firstChildElement();
         !childElement.isNull(); childElement = childElement.nextSiblingElement()) {
        cleanDomElement(childElement, pElementsAttributes);
    }
}

//==============================================================================

QByteArray serialiseDomDocument(const QDomDocument &pDomDocument)
{
    // Serialise the given DOM document
    // Note: normally, we would simply be using QDomDocument::save(), but we
    //       want elements' attributes to be sorted when serialised (so that it
    //       is easier to compare two different XML documents). Unfortunately,
    //       QDomDocument::save() doesn't provide such a functionality (since
    //       the order of attributes doesn't matter in XML). So, we make a call
    //       to QDomDocument::save(), but only after having removed all the
    //       elements' attributes, which we serialise manually afterwards...

    QByteArray res = QByteArray();

    // Make a deep copy of the given DOM document and remove all the elements'
    // attributes (but keep track of them, so that we can later on serialise
    // them manually)

    QDomDocument domDocument = pDomDocument.cloneNode().toDocument();
    QMap<QString, QByteArray> elementsAttributes = QMap<QString, QByteArray>();

    for (QDomElement childElement = domDocument.firstChildElement();
         !childElement.isNull(); childElement = childElement.nextSiblingElement()) {
        cleanDomElement(childElement, elementsAttributes);
    }

    // Serialise our 'reduced' DOM document

    QTextStream textStream(&res, QIODevice::WriteOnly);

    domDocument.save(textStream, 4);

    // Manually serialise the elements' attributes

    foreach (const QString &elementAttribute, elementsAttributes.keys())
        res.replace(elementAttribute+"=\"\"", elementsAttributes.value(elementAttribute));

    return res;
}

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
