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
// Central widget
//==============================================================================

#pragma once

//==============================================================================

#include "file.h"
#include "guiinterface.h"
#include "widget.h"
#include "viewinterface.h"

//==============================================================================

#include <QDir>
#include <QMap>
#include <QTabBar>

//==============================================================================

class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QStackedWidget;

//==============================================================================

namespace OpenCOR {

//==============================================================================

class Plugin;

//==============================================================================

namespace Core {

//==============================================================================

class CentralWidget;
class Dialog;
class UserMessageWidget;
class TabBarWidget;

//==============================================================================

class CentralWidgetMode
{
public:
    explicit CentralWidgetMode(CentralWidget *pOwner);
    ~CentralWidgetMode();

    bool isEnabled() const;
    void setEnabled(bool pEnabled);

    TabBarWidget * viewTabs() const;

    Plugins viewPlugins() const;
    void addViewPlugin(Plugin *pViewPlugin);

private:
    bool mEnabled;

    TabBarWidget *mViewTabs;

    Plugins mViewPlugins;
};

//==============================================================================

class CentralWidget : public Widget
{
    Q_OBJECT

public:
    explicit CentralWidget(QWidget *pParent);
    ~CentralWidget() override;

    void loadSettings(QSettings *pSettings) override;
    void saveSettings(QSettings *pSettings) const override;

    void settingsLoaded(const Plugins &pLoadedPlugins);

    void retranslateUi() override;

    void addView(Plugin *pPlugin);

    TabBarWidget * newTabBarWidget(const QTabBar::Shape &pShape,
                                   bool pFileTabs = false);

    QString currentFileName() const;

    void openFile(const QString &pFileName,
                  const File::Type &pType = File::Local,
                  const QString &pUrl = QString());
    void openFiles(const QStringList &pFileNames);

    void openRemoteFile(const QString &pUrl, bool pShowWarning = true);

    bool canClose();

    bool selectMode(const QString &pModeName);
    bool selectView(const QString &pViewName);

protected:
    void dragEnterEvent(QDragEnterEvent *pEvent) override;
    void dragMoveEvent(QDragMoveEvent *pEvent) override;
    void dropEvent(QDropEvent *pEvent) override;

private:
    enum State {
        Starting,
        Idling,
        UpdatingGui,
        Stopping
    };

    State mState;

    QSettings *mSettings;

    Plugins mLoadedFileHandlingPlugins;
    Plugins mLoadedFileTypePlugins;
    Plugins mLoadedGuiPlugins;
    Plugins mLoadedViewPlugins;

    TabBarWidget *mModeTabs;
    TabBarWidget *mFileTabs;

    QMap<int, ViewInterface::Mode> mModeTabIndexModes;
    QMap<ViewInterface::Mode, int> mModeModeTabIndexes;

    QMap<QString, int> mFileModeTabIndexes;
    QMap<QString, QMap<int, int>> mFileModeViewTabIndexes;

    QStringList mFileNames;

    QStackedWidget *mContents;

    QWidget *mLogoView;

    UserMessageWidget *mNoViewMsg;

    QMap<ViewInterface::Mode, CentralWidgetMode *> mModes;

    Dialog *mRemoteFileDialog;
    QLabel *mRemoteFileDialogUrlLabel;
    QLineEdit *mRemoteFileDialogUrlValue;
    QDialogButtonBox *mRemoteFileDialogButtonBox;

    QMap<QString, QString> mRemoteLocalFileNames;

    QMap<QString, QWidget *> mViews;

    Plugin * viewPlugin(int pIndex) const;
    Plugin * viewPlugin(const QString &pFileName) const;

    void updateNoViewMsg();

    bool saveFile(int pIndex, bool pNeedNewFileName = false);

    bool canCloseFile(int pIndex);

    void updateFileTab(int pIndex, bool pIconOnly = false);

    void updateStatusBarWidgets(QList<QWidget *> pWidgets);

    QString viewKey(int pMode, int pView, const QString &pFileName);

    void fileReloadedOrSaved(const QString &pFileName, bool pFileReloaded);

    void setTabBarCurrentIndex(TabBarWidget *pTabBar, int pIndex);

    void reloadFile(int pIndex, bool pForce);

    bool closeFile(int pIndex, bool pForceClosing);
    void closeAllFiles(bool pForceClosing);

signals:
    void guiUpdated(OpenCOR::Plugin *pViewPlugin, const QString &pFileName);

    void atLeastOneView(bool pAtLeastOneView);

    void canSave(bool pEnabled);
    void canSaveAs(bool pEnabled);
    void canSaveAll(bool pEnabled);

    void atLeastOneFile(bool pAtLeastOneFile);
    void atLeastTwoFiles(bool pAtLeastTwoFiles);

public slots:
    void openFile();

    void openRemoteFile();

    void reloadFile();

    void duplicateFile();

    void toggleLockedFile();

    void saveFile();
    void saveFileAs();
    void saveAllFiles();

    void previousFile();
    void nextFile();

    bool closeFile(int pIndex);
    bool closeFile();
    void closeAllFiles();

private slots:
    void updateGui();

    void openRemoteFileChanged();
    void doOpenRemoteFile();
    void cancelOpenRemoteFile();

    void fileChanged(const QString &pFileName, bool pFileChanged,
                     bool pDependenciesChanged);
    void fileDeleted(const QString &pFileName);

    void updateModifiedSettings();

    void filePermissionsChanged(const QString &pFileName);
    void fileModified(const QString &pFileName);

    void fileReloaded(const QString &pFileName);

    void fileCreated(const QString &pFileName, const QString &pUrl);
    void fileDuplicated(const QString &pFileName);
    void fileRenamed(const QString &pOldFileName, const QString &pNewFileName);

    void fileSaved(const QString &pFileName);

    void moveFile(int pFromIndex, int pToIndex);

    void updateFileTabIcon(const QString &pViewName, const QString &pFileName,
                           const QIcon &pIcon);
    void updateFileTabIcons();
};

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
