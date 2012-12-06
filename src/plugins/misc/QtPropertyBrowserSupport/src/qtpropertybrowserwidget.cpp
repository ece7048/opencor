//==============================================================================
// QtPropertyBrowserWidget class
//==============================================================================

#include "qtpropertybrowserwidget.h"

//==============================================================================

#include <QHeaderView>
#include <QScrollBar>
#include <QSettings>
#include <QTreeWidget>

//==============================================================================

#include <QtVariantPropertyManager>

//==============================================================================

namespace OpenCOR {
namespace QtPropertyBrowserSupport {

//==============================================================================

int QtPropertyBrowserTreeWidget::rowHeight(const QModelIndex &pIndex) const
{
    // Return the row's height (using the protected version of rowHeight()
    // defined in QTreeWidget)

    return QTreeWidget::rowHeight(pIndex);
}

//==============================================================================

void DoublePropertyManager::setUnit(QtProperty *pProperty, const QString &pUnit)
{
    // Make sure that the property exists

    if (!mData.contains(pProperty))
        return;

    // Set the unit for the given property

    Data data = mData[pProperty];

    if (pUnit.compare(data.unit)) {
        data.unit = pUnit;

        mData[pProperty] = data;
    }
}

//==============================================================================

QString DoublePropertyManager::valueText(const QtProperty *pProperty) const
{
    // Make sure that the property exists

    if (!mData.contains(pProperty))
        return QString();

    // Return our property as a string

    return QString::number(mData[pProperty].value);
}

//==============================================================================

void DoublePropertyManager::initializeProperty(QtProperty *pProperty)
{
    // Initialise the property

    mData[pProperty] = Data();
}

//==============================================================================

void DoublePropertyManager::uninitializeProperty(QtProperty *pProperty)
{
    // Uninitialise the property

    mData.remove(pProperty);
}

//==============================================================================

void QtPropertyBrowserWidget::constructor(const bool &pAutoResizeHeight)
{
    // Customise ourselves

    mAutoResizeHeight = pAutoResizeHeight;

    mPropertyManager = new QtVariantPropertyManager(this);
    QtVariantEditorFactory *editorFactory = new QtVariantEditorFactory(this);

    setAlternatingRowColors(false);
    setFactoryForManager(mPropertyManager, editorFactory);
    setPropertiesWithoutValueMarked(true);
    setResizeMode(QtTreePropertyBrowser::Interactive);
    setRootIsDecorated(false);

    QTreeWidget *propertyWidget = treeWidget();

#ifdef Q_WS_MAC
    propertyWidget->setAttribute(Qt::WA_MacShowFocusRect, 0);
    // Note: the above removes the focus border since it messes up the look of
    //       our property editor...
#endif
    propertyWidget->setFrameShape(QFrame::NoFrame);

    if (pAutoResizeHeight)
        propertyWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Resize our height in case one of our header's sections gets resized

    connect(propertyWidget->header(), SIGNAL(sectionResized(int, int, int)),
            this, SLOT(resizeHeight()));

    // Make our tree widget our focus proxy

    setFocusProxy(propertyWidget);
}

//==============================================================================

QtPropertyBrowserWidget::QtPropertyBrowserWidget(const bool &pAutoResizeHeight,
                                                 QWidget *pParent) :
    QtTreePropertyBrowser(pParent)
{
    // Construct our object

    constructor(pAutoResizeHeight);
}

//==============================================================================

QtPropertyBrowserWidget::QtPropertyBrowserWidget(QWidget *pParent) :
    QtTreePropertyBrowser(pParent)
{
    // Construct our object

    constructor();
}

//==============================================================================

void QtPropertyBrowserWidget::retranslateUi()
{
    // Retranslate our header labels

    treeWidget()->setHeaderLabels(QStringList() << tr("Property")
                                                << tr("Value"));
}

//==============================================================================

static const QString SettingsColumnWidth = "ColumnWidth%1";

//==============================================================================

void QtPropertyBrowserWidget::loadSettings(QSettings *pSettings)
{
    // Retrieve the width of each column
    // Note: to use splitterPosition()/setSplitterPosition() is not sufficient
    //       since we also need to retrieve the width of the value column
    //       otherwise we may get a horizontal scroll bar when we wouldn't, had
    //       we been able to retrieve the width of the value column...

    QTreeWidget *propertyWidget = treeWidget();

    for (int i = 0, iMax = propertyWidget->header()->count(); i < iMax; ++i)
        propertyWidget->setColumnWidth(i, pSettings->value(SettingsColumnWidth.arg(i),
                                                           propertyWidget->columnWidth(i)).toInt());
}

//==============================================================================

void QtPropertyBrowserWidget::saveSettings(QSettings *pSettings) const
{
    // Keep track of the width of each column
    // Note: see note above in loadSettings()...

    QTreeWidget *propertyWidget = treeWidget();

    for (int i = 0, iMax = propertyWidget->header()->count(); i < iMax; ++i)
        pSettings->setValue(SettingsColumnWidth.arg(i),
                            propertyWidget->columnWidth(i));
}

//==============================================================================

QSize QtPropertyBrowserWidget::sizeHint() const
{
    // Return either our default/ideal size, depending on the case

    if (mAutoResizeHeight) {
        // We automatically resize our height, so determine our ideal size which
        // is based on the width of our different columns, and the height of our
        // header and our different rows

        QtPropertyBrowserTreeWidget *propertyWidget = static_cast<QtPropertyBrowserTreeWidget *>(treeWidget());
        int hintWidth  = 0;
        int hintHeight = propertyWidget->header()->height();

        for (int i = 0, iMax = propertyWidget->header()->count(); i < iMax; ++i)
            hintWidth += propertyWidget->columnWidth(i);

        for (int i = 0, iMax = properties().count(); i < iMax; ++i)
            hintHeight += propertyWidget->rowHeight(propertyWidget->model()->index(i, 0));

        return QSize(hintWidth, hintHeight);
    } else {
        // We don't automatically resize our height, so our ideal size is our
        // maximum size...

        return maximumSize();
    }
}

//==============================================================================

QtVariantProperty * QtPropertyBrowserWidget::addProperty(const int pType,
                                                         const QString &pName)
{
    // Create a property

    QtVariantProperty *res = mPropertyManager->addProperty(pType, pName);

    // Add it to our GUI

    QtAbstractPropertyBrowser::addProperty(res);

    // Return it

    return res;
}

//==============================================================================

void QtPropertyBrowserWidget::selectFirstProperty()
{
    // Select the first property, if any

    setCurrentItem(items(properties().first()).first());
}

//==============================================================================

void QtPropertyBrowserWidget::resizeEvent(QResizeEvent *pEvent)
{
    // Default handling of the event

    QtTreePropertyBrowser::resizeEvent(pEvent);

    // Resize our height

    resizeHeight();
}

//==============================================================================

void QtPropertyBrowserWidget::resizeHeight()
{
    // Resize our height, so that we don't have any blank space at the bottom,
    // but only if required

    if (mAutoResizeHeight) {
        QSize idealSize = sizeHint();
        QTreeWidget *propertyWidget = treeWidget();

        propertyWidget->setFixedHeight( idealSize.height()
                                       +((width() < idealSize.width())?
                                             propertyWidget->horizontalScrollBar()->height():
                                             0));
        // Note: the new height consists of our ideal height to which we add the
        //       height of our horizontal scroll bar, should it be shown (i.e.
        //       if our width is smaller than that of our ideal size)...
    }
}

//==============================================================================

}   // namespace QScintillaSupport
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
