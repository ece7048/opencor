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
// Main window
//==============================================================================

#pragma once

//==============================================================================

#include "plugin.h"

//==============================================================================

#include <QMainWindow>
#include <QMap>
#include <QTranslator>
#include <QUrl>

//==============================================================================

namespace Ui {
    class MainWindow;
}   // namespace Ui

//==============================================================================

class QSettings;

//==============================================================================

namespace OpenCOR {

//==============================================================================

class CoreInterface;
class PluginManager;

//==============================================================================

enum {
    CleanRestart  =  843,
    NormalRestart = 1789
};

//==============================================================================

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &pApplicationDate);
    ~MainWindow() override;

    void handleArguments(const QStringList &pArguments);

    void showSelf();

protected:
    void changeEvent(QEvent *pEvent) override;
    void closeEvent(QCloseEvent *pEvent) override;

private:
    Ui::MainWindow *mGui;

    QString mApplicationDate;

    QSettings *mSettings;

    PluginManager *mPluginManager;

    Plugins mLoadedPluginPlugins;
    Plugins mLoadedI18nPlugins;
    Plugins mLoadedGuiPlugins;
    Plugins mLoadedPreferencesPlugins;
    Plugins mLoadedWindowPlugins;

    CoreInterface *mCoreInterface;

    QString mRawLocale;

    QTranslator mQtBaseTranslator;
    QTranslator mQtHelpTranslator;
    QTranslator mQtXmlPatternsTranslator;
    QTranslator mAppTranslator;

    QMap<QString, QMenu *> mMenus;

    QMenu *mFileNewMenu;

    QMenu *mViewWindowsMenu;
    QAction *mViewSeparator;

    Plugin *mViewPlugin;

    bool mDockedWindowsVisible;

    QByteArray mDockedWindowsState;

    void registerOpencorUrlScheme();

    void initializeGuiPlugin(Plugin *pPlugin);

    void loadSettings();
    void saveSettings() const;

    void setLocale(const QString &pRawLocale = QString(),
                   bool pForceSetting = false);

    void reorderViewWindowsMenu();

    void updateViewWindowsMenu(QAction *pAction);

    void restart(bool pSaveSettings) const;

    void showEnableActions(const QList<QAction *> &pActions);

    void showPreferencesDialog(const QString &pPluginName = QString());

    void showDockedWindows(bool pShow, bool pInitialisation);

private slots:
    void openFileOrHandleUrl(const QString &pFileNameOrOpencorUrl);
    void handleMessage(const QString &pMessage);

    void actionFullScreenTriggered();
    void actionSystemTriggered();
    void actionEnglishTriggered();
    void actionFrenchTriggered();
    void actionPluginsTriggered();
    void actionPreferencesTriggered();
    void actionHomePageTriggered();
    void actionCheckForUpdatesTriggered();
    void actionAboutTriggered();

    void updateGui(OpenCOR::Plugin *pViewPlugin, const QString &pFileName);

    void showDockedWindows(bool pShow);
    void toggleDockedWindows();

    void updateDockWidgetsVisibility();

    void resetAll();

    void handleUrl(const QUrl &pUrl);

    void exitFullScreenMode();
};

//==============================================================================

}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
