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
// Editor find/replace widget
//==============================================================================

#pragma once

//==============================================================================

#include "widget.h"

//==============================================================================

#include <QColor>
#include <QString>

//==============================================================================

class QLabel;
class QTextEdit;

//==============================================================================

namespace Ui {
    class EditorWidgetFindReplaceWidget;
}   // namespace Ui

//==============================================================================

namespace OpenCOR {
namespace EditorWidget {

//==============================================================================

class EditorWidgetFindReplaceWidget : public Core::Widget
{
    Q_OBJECT

public:
    explicit EditorWidgetFindReplaceWidget(QWidget *pParent);
    ~EditorWidgetFindReplaceWidget() override;

    void retranslateUi() override;

    bool isCaseSensitive() const;
    bool searchWholeWordsOnly() const;
    bool useRegularExpression() const;

    void setReadOnly(bool pReadOnly);

    void updateFrom(EditorWidgetFindReplaceWidget *pFindReplace);

    bool isFindPreviousNextAvailable() const;

    void selectFindText() const;

    QString findText() const;
    void setFindText(const QString &pFindText);

    QString replaceText() const;

    bool findEditHasFocus() const;
    bool replaceEditHasFocus() const;

    bool isActive() const;
    void setActive(bool pActive);

protected:
    void changeEvent(QEvent *pEvent) override;
    void keyPressEvent(QKeyEvent *pEvent) override;
    void resizeEvent(QResizeEvent *pEvent) override;

private:
    Ui::EditorWidgetFindReplaceWidget *mGui;

    QAction *mDropDownAction;

    QAction *mCaseSensitiveAction;
    QAction *mWholeWordsOnlyAction;
    QAction *mRegularExpressionAction;

    QAction *mClearFindTextAction;
    QAction *mClearReplaceTextAction;

    bool mActive;

    void updateHeight();
    void updateStyleSheet();

signals:
    void keyPressed(QKeyEvent *pEvent, bool &pHandled);

    void findTextChanged(const QString &pText);

    void canFindReplace(bool pCanFindReplace);

    void findPreviousRequested();
    void findNextRequested();

    void replaceRequested();
    void replaceAndFindRequested();
    void replaceAllRequested();

private slots:
    void findPreviousButtonClicked();
    void findNextButtonClicked();

    void replaceButtonClicked();
    void replaceAndFindButtonClicked();
    void replaceAllButtonClicked();

    void searchOptionChanged();

    void updateClearFindTextAction(const QString &pText);
    void updateClearReplaceTextAction(const QString &pText);
};

//==============================================================================

}   // namespace EditorWidget
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
