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
// Tab bar widget
//------------------------------------------------------------------------------
// Qt upgrade: make sure that our Tab bar widget still behaves as expected...
//==============================================================================

#include "tabbarwidget.h"

//==============================================================================

#include <QPainter>
#include <QStyleOption>
#include <QWheelEvent>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

void TabBarStyle::drawControl(ControlElement pElement,
                              const QStyleOption *pOption, QPainter *pPainter,
                              const QWidget *pWidget) const
{
    // Draw a control
    // Note: this is adapted from QCommonStyle::drawControl()...

    if (pElement == CE_TabBarTabLabel) {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(pOption)) {
            QRect tabRect = tab->rect;
            bool verticalTab =     (tab->shape == QTabBar::RoundedWest)
                                || (tab->shape == QTabBar::RoundedEast)
                                || (tab->shape == QTabBar::TriangularWest)
                                || (tab->shape == QTabBar::TriangularEast);
            int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;

            if (!styleHint(SH_UnderlineShortcut, pOption, pWidget))
                alignment |= Qt::TextHideMnemonic;

            if (verticalTab) {
                pPainter->save();

                int newX, newY, newRot;

                if (   (tab->shape == QTabBar::RoundedEast)
                    || (tab->shape == QTabBar::TriangularEast)) {
                    newX = tabRect.width()+tabRect.x();
                    newY = tabRect.y();

                    newRot = 90;
                } else {
                    newX = tabRect.x();
                    newY = tabRect.y()+tabRect.height();

                    newRot = -90;
                }

                QTransform transform = QTransform::fromTranslate(newX, newY);

                transform.rotate(newRot);

                pPainter->setTransform(transform, true);
            }

            QRect iconRect;

            tabLayout(tab, pWidget, &tabRect, &iconRect);

            if (!tab->icon.isNull()) {
                pPainter->drawPixmap(iconRect.x(), iconRect.y(),
                                     tab->icon.pixmap(pWidget?
                                                          pWidget->window()->windowHandle():
                                                          0,
                                                      tab->iconSize,
                                                      (tab->state & State_Enabled)?
                                                          QIcon::Normal:
                                                          QIcon::Disabled,
                                                      (tab->state & State_Selected)?
                                                          QIcon::On:
                                                          QIcon::Off));
            }

            drawItemText(pPainter, tabRect, alignment, tab->palette,
                         tab->state & State_Enabled, tab->text,
                         ((tab->state & State_Selected) && pWidget->isActiveWindow())?
                             QPalette::BrightText:
                             QPalette::WindowText);

            if (verticalTab)
                pPainter->restore();

            if (tab->state & State_HasFocus) {
                const int Offset = 1+pixelMetric(PM_DefaultFrameWidth);

                int x1 = tab->rect.left();
                int x2 = tab->rect.right()-1;

                QStyleOptionFocusRect option;

                option.QStyleOption::operator=(*tab);

                option.rect.setRect(x1+1+Offset, tab->rect.y()+Offset,
                                    x2-x1-2*Offset, tab->rect.height()-2*Offset);

                drawPrimitive(PE_FrameFocusRect, &option, pPainter, pWidget);
            }
        }
    } else {
        QProxyStyle::drawControl(pElement, pOption, pPainter, pWidget);
    }
}

//==============================================================================

QRect TabBarStyle::subElementRect(SubElement pElement,
                                  const QStyleOption *pOption,
                                  const QWidget *pWidget) const
{
    // Determine and return the size of a sub-element
    // Note: this is adapted from QCommonStyle::subElementRect()...

    if (pElement == SE_TabBarTabText) {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(pOption)) {
            QRect res;
            QRect dummy;

            tabLayout(tab, pWidget, &res, &dummy);

            return res;
        } else {
            return QRect();
        }
    } else {
        return QProxyStyle::subElementRect(pElement, pOption, pWidget);
    }
}

//==============================================================================

void TabBarStyle::tabLayout(const QStyleOptionTab *pOption,
                            const QWidget *pWidget, QRect *pTextRect,
                            QRect *pIconRect) const
{
    // Compute the tab layout
    // Note: this is adapted from QCommonStylePrivate::tabLayout()...

    Q_ASSERT(pTextRect);
    Q_ASSERT(pIconRect);

    QRect textRect = pOption->rect;
    bool verticalTab =    (pOption->shape == QTabBar::RoundedWest)
                       || (pOption->shape == QTabBar::RoundedEast)
                       || (pOption->shape == QTabBar::TriangularWest)
                       || (pOption->shape == QTabBar::TriangularEast);

    if (verticalTab)
        textRect.setRect(0, 0, textRect.height(), textRect.width());

    if (!pOption->leftButtonSize.isEmpty())
        textRect.adjust(-4, 0, 0, 0);

    int horizontalShift = pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, pOption, pWidget);
    int verticalShift = pixelMetric(QStyle::PM_TabBarTabShiftVertical, pOption, pWidget);
    int horizontalPadding = 0.5*pixelMetric(QStyle::PM_TabBarTabHSpace, pOption, pWidget);
    int verticalPadding = 0.5*pixelMetric(QStyle::PM_TabBarTabVSpace, pOption, pWidget);

    if (   (pOption->shape == QTabBar::RoundedSouth)
        || (pOption->shape == QTabBar::TriangularSouth)) {
        verticalShift = -verticalShift;
    }

    textRect.adjust(horizontalPadding, verticalShift-verticalPadding,
                    horizontalShift-horizontalPadding, verticalPadding);

    if (pOption->state & QStyle::State_Selected) {
        textRect.setTop(textRect.top()-verticalShift);
        textRect.setRight(textRect.right()-horizontalShift);
    }

    if (!pOption->leftButtonSize.isEmpty()) {
        textRect.setLeft( textRect.left()
                         +(verticalTab?
                               pOption->leftButtonSize.height():
                               pOption->leftButtonSize.width()));
    }

    if (!pOption->rightButtonSize.isEmpty()) {
        textRect.setRight( textRect.right()
                          -(verticalTab?
                                pOption->rightButtonSize.height():
                                pOption->rightButtonSize.width()));
    }

    if (!pOption->icon.isNull()) {
        QSize iconSize = pOption->iconSize;

        if (!iconSize.isValid()) {
            int iconExtent = pixelMetric(QStyle::PM_SmallIconSize);

            iconSize = QSize(iconExtent, iconExtent);
        }

        QSize tabIconSize = pOption->icon.actualSize(iconSize,
                                                     (pOption->state & QStyle::State_Enabled)?
                                                         QIcon::Normal:
                                                         QIcon::Disabled,
                                                     (pOption->state & QStyle::State_Selected)?
                                                         QIcon::On:
                                                         QIcon::Off);

        tabIconSize = QSize(qMin(tabIconSize.width(), iconSize.width()),
                            qMin(tabIconSize.height(), iconSize.height()));

        *pIconRect = QRect(textRect.left(), textRect.center().y()-0.5*tabIconSize.height(),
                           tabIconSize.width(), tabIconSize.height());

        if (!verticalTab)
            *pIconRect = visualRect(pOption->direction, pOption->rect, *pIconRect);

        textRect.setLeft(textRect.left()+tabIconSize.width()+4);
    }

    if (!verticalTab)
        textRect = visualRect(pOption->direction, pOption->rect, textRect);

    *pTextRect = textRect;
}

//==============================================================================

TabBarWidget::TabBarWidget(QWidget *pParent) :
    QTabBar(pParent),
    mOldIndex(-1)
{
    // Customise our style, but only if we are on macOS
    // Note: indeed, between Qt 5.6.x LTS and Qt 5.9.x LTS, the styling of a
    //       QTabBar has changed quite a bit and is now not working properly,
    //       hence we fix things by having 'our' own style (see
    //       https://bugreports.qt.io/browse/QTBUG-61742)...

#ifdef Q_OS_MAC
    setStyle(new TabBarStyle());
#endif
}

//==============================================================================

int TabBarWidget::oldIndex() const
{
    // Return our old index

    return mOldIndex;
}

//==============================================================================

void TabBarWidget::setOldIndex(int pOldIndex)
{
    // Set our old index

    mOldIndex = pOldIndex;
}

//==============================================================================

void TabBarWidget::wheelEvent(QWheelEvent *pEvent)
{
    // Switch to the next/previous tab, if possible

    int offset = (pEvent->delta() < 0)?1:-1;

    for (int i = currentIndex()+offset; (i >= 0) && (i < count()); i += offset) {
        if (isTabEnabled(i)) {
            setCurrentIndex(i);

            break;
        }
    }

    QWidget::wheelEvent(pEvent);
}

//==============================================================================

QSize TabBarWidget::tabSizeHint(int pIndex) const
{
    // Determine and return our tab size hint, keeping in mind our style when on
    // macOS

#ifdef Q_OS_MAC
    int shift = tabsClosable()?12:0;
    QIcon icon = tabIcon(pIndex);

    if (!icon.isNull()) {
        int iconExtent = style()->pixelMetric(QStyle::PM_SmallIconSize);

        shift += icon.actualSize(QSize(iconExtent, iconExtent)).width()+4;
    }

    if (   (shape() == RoundedNorth) || (shape() == RoundedSouth)
        || (shape() == TriangularNorth) || (shape() == TriangularSouth)) {
        return QTabBar::tabSizeHint(pIndex)-QSize(shift, 0);
    } else {
        return QTabBar::tabSizeHint(pIndex)-QSize(0, shift);
    }
#else
    return QTabBar::tabSizeHint(pIndex);
#endif
}

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
