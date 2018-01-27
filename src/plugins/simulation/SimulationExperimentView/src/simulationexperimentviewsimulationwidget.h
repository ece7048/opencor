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
// Simulation Experiment view simulation widget
//==============================================================================

#pragma once

//==============================================================================

#include "corecliutils.h"
#include "graphpanelplotwidget.h"
#include "widget.h"

//==============================================================================

class QFrame;
class QLabel;
class QMenu;
class QTextEdit;

//==============================================================================

class QwtWheel;

//==============================================================================

namespace libsedml {
    class SedDocument;
    class SedModel;
    class SedRepeatedTask;
    class SedSimulation;
    class SedVariable;
}   // namespace libsedml

//==============================================================================

namespace OpenCOR {

//==============================================================================

class DataStoreInterface;

//==============================================================================

namespace CellMLSupport {
    class CellmlFileRuntimeParameter;
}   // namespace CellMLSupport

//==============================================================================

namespace Core {
    class ProgressBarWidget;
    class Property;
    class PropertyEditorWidget;
    class SplitterWidget;
    class ToolBarWidget;
    class UserMessageWidget;
}   // namespace Core

//==============================================================================

namespace GraphPanelWidget {
    class GraphPanelWidget;
}   // namespace GraphPanelWidget

//==============================================================================

namespace SEDMLSupport {
    class SedmlFile;
}   // namespace SEDMLSupport

//==============================================================================

namespace SimulationSupport {
    class Simulation;
}   // namespace SimulationSupport

//==============================================================================

namespace SimulationExperimentView {

//==============================================================================

class SimulationExperimentViewContentsWidget;
class SimulationExperimentViewPlugin;
class SimulationExperimentViewWidget;

//==============================================================================

class SimulationExperimentViewSimulationWidget : public Core::Widget
{
    Q_OBJECT

public:
    explicit SimulationExperimentViewSimulationWidget(SimulationExperimentViewPlugin *pPlugin,
                                                      SimulationExperimentViewWidget *pViewWidget,
                                                      const QString &pFileName,
                                                      QWidget *pParent);
    ~SimulationExperimentViewSimulationWidget();

    virtual void retranslateUi();

    void initialize(const bool &pReloadingView = false);
    void finalize();

    void setSizes(const QIntList &pSizes);

    SimulationExperimentViewContentsWidget * contentsWidget() const;

    QIcon fileTabIcon() const;

    bool save(const QString &pFileName);

    void filePermissionsChanged();
    void fileModified();
    void fileReloaded();

    void fileRenamed(const QString &pOldFileName, const QString &pNewFileName);

    SimulationSupport::Simulation * simulation() const;

    void updateGui(const bool &pCheckVisibility = false);
    void updateSimulationResults(SimulationExperimentViewSimulationWidget *pSimulationWidget,
                                 const quint64 &pSimulationResultsSize,
                                 const bool &pClearGraphs);

    void resetSimulationProgress();

protected:
    virtual void paintEvent(QPaintEvent *pEvent);

private:
    enum ErrorType {
        General,
        InvalidCellmlFile,
        InvalidSimulationEnvironment
    };

    SimulationExperimentViewPlugin *mPlugin;

    SimulationExperimentViewWidget *mViewWidget;

    QMap<QAction *, DataStoreInterface *> mDataStoreInterfaces;

    QMap<QAction *, Plugin *> mCellmlBasedViewPlugins;

    SimulationSupport::Simulation *mSimulation;

    Core::ProgressBarWidget *mProgressBarWidget;

    int mProgress;
    bool mLockedDevelopmentMode;

    Core::ToolBarWidget *mToolBarWidget;

    QMenu *mSimulationDataExportDropDownMenu;

    QFrame *mTopSeparator;
    QFrame *mBottomSeparator;

    QAction *mRunPauseResumeSimulationAction;
    QAction *mStopSimulationAction;
    QAction *mResetModelParametersAction;
    QAction *mClearSimulationDataAction;
    QAction *mDevelopmentModeAction;
    QAction *mAddGraphPanelAction;
    QAction *mRemoveGraphPanelAction;
    QAction *mRemoveCurrentGraphPanelAction;
    QAction *mRemoveAllGraphPanelsAction;
    QAction *mCellmlOpenAction;
    QAction *mSedmlExportAction;
    QAction *mSedmlExportSedmlFileAction;
    QAction *mSedmlExportCombineArchiveAction;
    QAction *mSimulationDataExportAction;

    QwtWheel *mDelayWidget;
    QLabel *mDelayValueWidget;

    Core::SplitterWidget *mSplitterWidget;

    SimulationExperimentViewContentsWidget *mContentsWidget;

    bool mRunActionEnabled;

    Core::UserMessageWidget *mInvalidModelMessageWidget;

    QTextEdit *mOutputWidget;

    ErrorType mErrorType;

    bool mValidSimulationEnvironment;

    GraphPanelWidget::GraphPanelPlotWidgets mPlots;
    QMap<GraphPanelWidget::GraphPanelPlotWidget *, bool> mUpdatablePlotViewports;

    QStringList mSimulationProperties;
    QStringList mSolversProperties;
    QMap<Core::PropertyEditorWidget *, QStringList> mGraphPanelProperties;
    QMap<Core::PropertyEditorWidget *, QStringList> mGraphsProperties;

    bool mSimulationPropertiesModified;
    bool mSolversPropertiesModified;
    QMap<Core::PropertyEditorWidget *, bool>  mGraphPanelPropertiesModified;
    QMap<Core::PropertyEditorWidget *, bool>  mGraphsPropertiesModified;

    QIntList mGraphPanelsWidgetSizes;
    bool mGraphPanelsWidgetSizesModified;

    bool mCanUpdatePlotsForUpdatedGraphs;

    bool mNeedReloadView;

    bool mNeedUpdatePlots;

    QMap<GraphPanelWidget::GraphPanelPlotGraph *, quint64> mOldDataSizes;

    void reloadView();

    void output(const QString &pMessage);

    void updateSimulationMode();

    int tabBarPixmapSize() const;

    void updateRunPauseAction(const bool &pRunActionEnabled);

    void updateDataStoreActions();

    void updateInvalidModelMessageWidget();

    bool updatePlot(GraphPanelWidget::GraphPanelPlotWidget *pPlot,
                    const bool &pCanSetAxes = true,
                    const bool &pForceReplot = false);

    double * data(SimulationSupport::Simulation *pSimulation,
                  CellMLSupport::CellmlFileRuntimeParameter *pParameter) const;

    void updateGraphData(GraphPanelWidget::GraphPanelPlotGraph *pGraph,
                         const quint64 &pSize);

    void updateSimulationProperties(Core::Property *pProperty = 0);
    void updateSolversProperties(Core::Property *pProperty,
                                 const bool &pResetNlaSolver);
    void updateSolversProperties(Core::Property *pProperty);
    void updateSolversProperties(const bool &pResetNlaSolver);
    void updateSolversProperties();

    CellMLSupport::CellmlFileRuntimeParameter * runtimeParameter(libsedml::SedVariable *pSedmlVariable);

    bool furtherInitialize();
    void initializeGui(const bool &pValidSimulationEnvironment);
    void initializeSimulation();

    void initialiseTrackers();

    QString fileName(const QString &pFileName, const QString &pBaseFileName,
                     const QString &pFileExtension, const QString &pCaption,
                     const QStringList &pFileFilters);

    void addSedmlSimulation(libsedml::SedDocument *pSedmlDocument,
                            libsedml::SedModel *pSedmlModel,
                            libsedml::SedRepeatedTask *pSedmlRepeatedTask,
                            libsedml::SedSimulation *pSedmlSimulation,
                            const int &pOrder);
    void addSedmlVariableTarget(libsedml::SedVariable *pSedmlVariable,
                                const QString &pComponent,
                                const QString &pVariable);
    bool createSedmlFile(SEDMLSupport::SedmlFile *pSedmlFile,
                         const QString &pFileName, const QString &pModelSource);

    QStringList allPropertyValues(Core::PropertyEditorWidget *pPropertyEditor) const;

    void updateFileModifiedStatus();

signals:
    void splitterMoved(const QIntList &pSizes);

    void graphPanelSettingsRequested();
    void graphsSettingsRequested();

    void graphToggled(OpenCOR::GraphPanelWidget::GraphPanelPlotGraph *pGraph);

    void legendToggled();

    void logarithmicXAxisToggled();
    void logarithmicYAxisToggled();

private slots:
    void runPauseResumeSimulation();
    void stopSimulation();
    void developmentMode();
    void addGraphPanel();
    void removeGraphPanel();
    void removeCurrentGraphPanel();
    void removeAllGraphPanels();
    void resetModelParameters();
    void clearSimulationData();
    void sedmlExportSedmlFile(const QString &pFileName = QString());
    void sedmlExportCombineArchive(const QString &pFileName = QString());

    void emitSplitterMoved();

    void simulationDataExport();

    void updateDelayValue(const double &pDelayValue);

    void simulationRunning(const bool &pIsResuming);
    void simulationPaused();
    void simulationStopped(const qint64 &pElapsedTime);

    void resetProgressBar();
    void resetFileTabIcon();

    void simulationError(const QString &pMessage,
                         const ErrorType &pErrorType = General);

    void simulationDataModified(const bool &pIsModified);

    void simulationPropertyChanged(OpenCOR::Core::Property *pProperty);
    void solversPropertyChanged(OpenCOR::Core::Property *pProperty);

    void graphPanelAdded(OpenCOR::GraphPanelWidget::GraphPanelWidget *pGraphPanel,
                         const bool &pActive);
    void graphPanelRemoved(OpenCOR::GraphPanelWidget::GraphPanelWidget *pGraphPanel);

    void addGraph(OpenCOR::CellMLSupport::CellmlFileRuntimeParameter *pParameterX,
                  OpenCOR::CellMLSupport::CellmlFileRuntimeParameter *pParameterY);

    void graphAdded(OpenCOR::GraphPanelWidget::GraphPanelWidget *pGraphPanel,
                    OpenCOR::GraphPanelWidget::GraphPanelPlotGraph *pGraph,
                    const OpenCOR::GraphPanelWidget::GraphPanelPlotGraphProperties &pGraphProperties);
    void graphsRemoved(OpenCOR::GraphPanelWidget::GraphPanelWidget *pGraphPanel,
                       const OpenCOR::GraphPanelWidget::GraphPanelPlotGraphs &pGraphs);

    void graphsUpdated(const OpenCOR::GraphPanelWidget::GraphPanelPlotGraphs &pGraphs);
    void graphUpdated(OpenCOR::GraphPanelWidget::GraphPanelPlotGraph *pGraph);

    void openCellmlFile();

    void plotAxesChanged();

    void dataStoreExportDone(const QString &pErrorMessage);
    void dataStoreExportProgress(const double &pProgress);

    void checkSimulationProperties();
    void checkSolversProperties();
    void checkGraphPanelsAndGraphs();
};

//==============================================================================

}   // namespace SimulationExperimentView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
