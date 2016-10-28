/*******************************************************************************

Copyright The University of Auckland

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*******************************************************************************/

//==============================================================================
// PMR Workspaces window
//==============================================================================

#include "borderedwidget.h"
#include "corecliutils.h"
#include "coreguiutils.h"
#include "filemanager.h"
#include "toolbarwidget.h"
#include "pmrwebservice.h"
#include "pmrworkspaceswindownewworkspace.h"
#include "pmrworkspaceswindowwidget.h"
#include "pmrworkspaceswindowwindow.h"

//==============================================================================

#include "ui_pmrworkspaceswindowwindow.h"

//==============================================================================

#include <Qt>

//==============================================================================

#include <QDir>
#include <QMainWindow>
#include <QPoint>
#include <QSettings>
#include <QTimer>

//==============================================================================

namespace OpenCOR {
namespace PMRWorkspacesWindow {

//==============================================================================

PmrWorkspacesWindowWindow::PmrWorkspacesWindowWindow(QWidget *pParent) :
    Core::OrganisationWidget(pParent),
    mGui(new Ui::PmrWorkspacesWindowWindow)
{
    // Set up the GUI

    mGui->setupUi(this);

    // Create an instance of PMR

    mPmrWebService = new PMRSupport::PmrWebService(this);

    // Create a tool bar widget with different buttons

    Core::ToolBarWidget *toolBarWidget = new Core::ToolBarWidget(this);

    toolBarWidget->addAction(mGui->actionNew);
    toolBarWidget->addAction(mGui->actionRefresh);

    // Right align logon/off

    QWidget *spacer1 = new QWidget();
    spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolBarWidget->addWidget(spacer1);

    toolBarWidget->addAction(mGui->actionAuthenticate);
    toolBarWidget->addAction(mGui->actionUnauthenticate);

    QWidget *spacer2 = new QWidget();
    spacer2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    spacer2->setMinimumSize(4, 0);
    toolBarWidget->addWidget(spacer2);

    mGui->layout->addWidget(toolBarWidget);

    // Create and add the workspaces widget

    mWorkspacesWindowWidget = new PmrWorkspacesWindowWidget(mPmrWebService, this);

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    mGui->dockWidgetContents->layout()->addWidget(new Core::BorderedWidget(mWorkspacesWindowWidget,
                                                                           true, true, true, true));
#elif defined(Q_OS_MAC)
    mGui->dockWidgetContents->layout()->addWidget(new Core::BorderedWidget(mWorkspacesWindowWidget,
                                                                           true, false, false, false));
#else
    #error Unsupported platform
#endif

    // Keep track of the window's visibility, so that we can request the list of
    // exposures, if necessary

    connect(this, SIGNAL(visibilityChanged(bool)),
            this, SLOT(retrieveWorkspaces(const bool &)));

    // Watch for changes to managed files

    Core::FileManager * fileManagerInstance = Core::FileManager::instance();

    connect(fileManagerInstance, SIGNAL(fileCreated(const QString &, const QString &)),
            this, SLOT(fileCreated(const QString &, const QString &)));
    connect(fileManagerInstance, SIGNAL(fileDuplicated(const QString &)),
            this, SLOT(fileDuplicated(const QString &)));
    connect(fileManagerInstance, SIGNAL(fileDeleted(const QString &)),
            this, SLOT(fileDeleted(const QString &)));

    // Some connections to process responses from the PMR web service

    connect(mPmrWebService, SIGNAL(busy(const bool &)),
            this, SLOT(busy(const bool &)));
    connect(mPmrWebService, SIGNAL(progress(const double &)),
            this, SLOT(showProgress(const double &)));

    connect(mPmrWebService, SIGNAL(error(const QString &, const bool &)),
            this, SLOT(showError(const QString &)));
    connect(mPmrWebService, SIGNAL(information(const QString &)),
            this, SLOT(showInformation(const QString &)));
    connect(mPmrWebService, SIGNAL(warning(const QString &)),
            this, SLOT(showWarning(const QString &)));

    connect(mPmrWebService, SIGNAL(authenticated(const bool &)),
            this, SLOT(updateAuthenticationStatus(const bool &)));
    connect(mPmrWebService, SIGNAL(workspaces(const PMRSupport::PmrWorkspaces &)),
            mWorkspacesWindowWidget, SLOT(initialiseWorkspaceWidget(const PMRSupport::PmrWorkspaces &)));

    // Connections to process requests from our widget

    connect(mWorkspacesWindowWidget, SIGNAL(information(const QString &)),
            this, SLOT(showInformation(const QString &)));
    connect(mWorkspacesWindowWidget, SIGNAL(openFileRequested(const QString &)),
            this, SLOT(openFile(const QString &)));
    connect(mWorkspacesWindowWidget, SIGNAL(warning(const QString &)),
            this, SLOT(showWarning(const QString &)));

    // Retranslate the GUI

    retranslateUi();
}

//==============================================================================

PmrWorkspacesWindowWindow::~PmrWorkspacesWindowWindow()
{
    // Delete the GUI

    delete mGui;
}

//==============================================================================

void PmrWorkspacesWindowWindow::retranslateUi()
{
    // Retranslate the whole window

    mGui->retranslateUi(this);

    // Retranslate the workspaces widget

    mWorkspacesWindowWidget->retranslateUi();
}

//==============================================================================

void PmrWorkspacesWindowWindow::resizeEvent(QResizeEvent *pEvent)
{
    // Default handling of the event

    Core::OrganisationWidget::resizeEvent(pEvent);

    // Resize our busy widget

    mWorkspacesWindowWidget->resizeBusyWidget();
}

//==============================================================================

void PmrWorkspacesWindowWindow::loadSettings(QSettings *pSettings)
{
    // Retrieve the settings of the workspaces widget

    pSettings->beginGroup(mWorkspacesWindowWidget->objectName());
        mWorkspacesWindowWidget->loadSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

void PmrWorkspacesWindowWindow::saveSettings(QSettings *pSettings) const
{
    // Keep track of the settings of the workspaces widget

    pSettings->beginGroup(mWorkspacesWindowWidget->objectName());
        mWorkspacesWindowWidget->saveSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

void PmrWorkspacesWindowWindow::busy(const bool &pBusy)
{
    // Show ourselves as busy or not busy anymore

    static int counter = 0;

    counter += pBusy?1:-1;

    if (pBusy && (counter == 1)) {
        mWorkspacesWindowWidget->showProgressBusyWidget();

        mGui->dockWidgetContents->setEnabled(false);
    } else if (!pBusy && !counter) {
        // Re-enable the GUI side

        mWorkspacesWindowWidget->hideBusyWidget();

        mGui->dockWidgetContents->setEnabled(true);

        // And give focus to??
    }
}

//==============================================================================

void PmrWorkspacesWindowWindow::showProgress(const double &pProgress)
{
    mWorkspacesWindowWidget->setBusyWidgetProgress(pProgress);
}

//==============================================================================

void PmrWorkspacesWindowWindow::showError(const QString &pMessage)
{
    // Show the given message as an error

    Core::criticalMessageBox(Core::mainWindow(), windowTitle(), pMessage);
}

//==============================================================================

void PmrWorkspacesWindowWindow::showInformation(const QString &pMessage)
{
    // Show the given message as informative text

    Core::informationMessageBox(Core::mainWindow(), windowTitle(), pMessage);
}

//==============================================================================

void PmrWorkspacesWindowWindow::showWarning(const QString &pMessage)
{
    // Show the given message as a warning

    Core::warningMessageBox(Core::mainWindow(), windowTitle(), pMessage);
}

//==============================================================================

void PmrWorkspacesWindowWindow::retrieveWorkspaces(const bool &pVisible)
{
    // Request authentication status, if we are becoming visible and the list
    // of workspaces has never been requested before (through a single shot, this
    // to allow other events, such as the one asking OpenCOR's main window to
    // resize itself, to be handled properly). This will result in the workspace
    // list being populated if we are in fact authenticated with PMR.

    static bool firstTime = true;

    if (pVisible && firstTime) {
        firstTime = false;

        QTimer::singleShot(0, this, SLOT(getAuthenticationStatus()));
    }
}

//==============================================================================

void PmrWorkspacesWindowWindow::getAuthenticationStatus()
{
    // Results in us being sent an authenticated() signal

    mPmrWebService->getAuthenticationStatus();
}

//==============================================================================

void PmrWorkspacesWindowWindow::updateAuthenticationStatus(const bool &pAuthenticated)
{
    // Show authentication state and allow workspace creation if authenticated

    Core::showEnableAction(mGui->actionAuthenticate, pAuthenticated);
    Core::showEnableAction(mGui->actionNew, true, pAuthenticated);
    Core::showEnableAction(mGui->actionRefresh, true, pAuthenticated);
    Core::showEnableAction(mGui->actionUnauthenticate, !pAuthenticated);

    if (pAuthenticated)
        mWorkspacesWindowWidget->refreshWorkspaces();
    else
        mWorkspacesWindowWidget->clearWorkspaces();
}

//==============================================================================

void PmrWorkspacesWindowWindow::on_actionAuthenticate_triggered()
{
    // Log on to PMR

    mPmrWebService->authenticate();
}

//==============================================================================

void PmrWorkspacesWindowWindow::on_actionNew_triggered()
{
    // Create a new workspace

    PmrWorkspacesWindowNewWorkspace *newWorkspaceDialog = new PmrWorkspacesWindowNewWorkspace(Core::mainWindow());

    if (newWorkspaceDialog->exec() == QDialog::Accepted) {
        // Create the folder for the new workspace

        QDir workspaceFolder(newWorkspaceDialog->path());

        if (!workspaceFolder.exists()) {
            workspaceFolder.mkpath(".");
        } else {
            // TODO Need to check the folder is empty and if not, give
            // an error and return.
            //
            // However folder selection in the new workspace dialog
            // should ensure we have an empty folder.
            //
            // What error message do we see from git if the folder
            // isn't empty??
        }

        // Ask the PMR web service to create a new workspace, resulting in the
        // (empty) workspace being cloned into its folder

        mPmrWebService->requestNewWorkspace(newWorkspaceDialog->title(),
                                            newWorkspaceDialog->description(),
                                            newWorkspaceDialog->path());
    }
}

//==============================================================================

void PmrWorkspacesWindowWindow::on_actionRefresh_triggered()
{
    // Ask the workspaces widget to refresh itself

    mWorkspacesWindowWidget->refreshWorkspaces();
}

//==============================================================================

void PmrWorkspacesWindowWindow::on_actionUnauthenticate_triggered()
{
    // Log off PMR

    if (QMessageBox::question(this, tr("OpenCOR"),
                              tr("Log off PMR?")) == QMessageBox::Yes) {
        mPmrWebService->authenticate(false);
    }
}

//==============================================================================

void PmrWorkspacesWindowWindow::fileCreated(const QString &pFileName,
                                            const QString &pUrl)
{
    Q_UNUSED(pFileName);
    Q_UNUSED(pUrl);
}

//==============================================================================

void PmrWorkspacesWindowWindow::fileDeleted(const QString &pFileName)
{
    Q_UNUSED(pFileName);
}

//==============================================================================

void PmrWorkspacesWindowWindow::fileDuplicated(const QString &pFileName)
{
    Q_UNUSED(pFileName);
}

//==============================================================================

void PmrWorkspacesWindowWindow::fileReloaded(const QString &pFileName)
{
    mWorkspacesWindowWidget->refreshWorkspaceFile(pFileName);
}

//==============================================================================

void PmrWorkspacesWindowWindow::fileRenamed(const QString &pOldFileName, const QString &pNewFileName)
{
    mWorkspacesWindowWidget->refreshWorkspaceFile(pOldFileName);
    mWorkspacesWindowWidget->refreshWorkspaceFile(pNewFileName);
}

//==============================================================================

}   // namespace PMRWorkspacesWindow
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
