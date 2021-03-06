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
// Simulation
//==============================================================================

#pragma once

//==============================================================================

#include "datastoreinterface.h"
#include "simulationsupportglobal.h"
#include "solverinterface.h"

//==============================================================================

class QEventLoop;

//==============================================================================

namespace OpenCOR {

//==============================================================================

namespace CellMLSupport {
    class CellmlFile;
    class CellmlFileRuntime;
}   // namespace CellMLSupport

//==============================================================================

namespace COMBINESupport {
    class CombineArchive;
}   // namespace COMBINESupport

//==============================================================================

namespace SEDMLSupport {
    class SedmlFile;
}   // namespace SEDMLSupport

//==============================================================================

namespace SimulationSupport {

//==============================================================================

class Simulation;
class SimulationWorker;

//==============================================================================

class SIMULATIONSUPPORT_EXPORT SimulationData : public QObject
{
    Q_OBJECT

public:
    explicit SimulationData(Simulation *pSimulation);
    ~SimulationData() override;

    void reload();

    Simulation * simulation() const;

    double * constants() const;
    double * rates() const;
    double * states() const;
    double * algebraic() const;

    int delay() const;
    void setDelay(int pDelay);

    double startingPoint() const;
    void setStartingPoint(double pStartingPoint, bool pRecompute = true);

    double endingPoint() const;
    void setEndingPoint(double pEndingPoint);

    double pointInterval() const;
    void setPointInterval(double pPointInterval);

    SolverInterface * odeSolverInterface() const;

    QString odeSolverName() const;
    void setOdeSolverName(const QString &pOdeSolverName);

    Solver::Solver::Properties odeSolverProperties() const;
    void addOdeSolverProperty(const QString &pName, const QVariant &pValue);

    SolverInterface * nlaSolverInterface() const;

    QString nlaSolverName() const;
    void setNlaSolverName(const QString &pNlaSolverName, bool pReset = true);

    Solver::Solver::Properties nlaSolverProperties() const;
    void addNlaSolverProperty(const QString &pName, const QVariant &pValue,
                              bool pReset = true);

    void reset(bool pInitialize = true);

    void recomputeComputedConstantsAndVariables(double pCurrentPoint,
                                                bool pInitialize);
    void recomputeVariables(double pCurrentPoint);

    bool isModified() const;
    void checkForModifications();

private:
    Simulation *mSimulation;

    int mDelay;

    double mStartingPoint;
    double mEndingPoint;
    double mPointInterval;

    QString mOdeSolverName;
    Solver::Solver::Properties mOdeSolverProperties;

    QString mDaeSolverName;
    Solver::Solver::Properties mDaeSolverProperties;

    QString mNlaSolverName;
    Solver::Solver::Properties mNlaSolverProperties;

    double *mConstants;
    double *mRates;
    double *mStates;
    double *mDummyStates;
    double *mAlgebraic;

    double *mInitialConstants;
    double *mInitialStates;

    void createArrays();
    void deleteArrays();

    SolverInterface * solverInterface(const QString &pSolverName) const;

signals:
    void updated(double pCurrentPoint);
    void modified(bool pIsModified);

    void error(const QString &pMessage);
};

//==============================================================================

class SIMULATIONSUPPORT_EXPORT SimulationResults : public QObject
{
    Q_OBJECT

public:
    explicit SimulationResults(Simulation *pSimulation);
    ~SimulationResults() override;

    void reload();

    void reset();

    int runsCount() const;

    bool addRun();

    void addPoint(double pPoint);

    quint64 size(int pRun = -1) const;

    DataStore::DataStore * dataStore() const;

    double * points(int pRun = -1) const;

    double * constants(int pIndex, int pRun = -1) const;
    double * rates(int pIndex, int pRun = -1) const;
    double * states(int pIndex, int pRun = -1) const;
    double * algebraic(int pIndex, int pRun = -1) const;

private:
    Simulation *mSimulation;

    DataStore::DataStore *mDataStore;

    DataStore::DataStoreVariable *mPoints;

    DataStore::DataStoreVariables mConstants;
    DataStore::DataStoreVariables mRates;
    DataStore::DataStoreVariables mStates;
    DataStore::DataStoreVariables mAlgebraic;

    void createDataStore();
    void deleteDataStore();

    QString uri(const QStringList &pComponentHierarchy, const QString &pName);
};

//==============================================================================

class SIMULATIONSUPPORT_EXPORT Simulation : public QObject
{
    Q_OBJECT

public:
    enum FileType {
        CellmlFile,
        SedmlFile,
        CombineArchive
    };

    explicit Simulation(const QString &pFileName);
    ~Simulation() override;

    QString fileName() const;

    void save();
    void reload();
    void rename(const QString &pFileName);

    CellMLSupport::CellmlFileRuntime * runtime() const;

    SimulationWorker * worker() const;

    Simulation::FileType fileType() const;

    CellMLSupport::CellmlFile * cellmlFile() const;
    SEDMLSupport::SedmlFile * sedmlFile() const;
    COMBINESupport::CombineArchive * combineArchive() const;

    SimulationData * data() const;
    SimulationResults * results() const;

    int runsCount() const;

    bool addRun();

    bool isRunning() const;
    bool isPaused() const;

    double currentPoint() const;

    int delay() const;
    void setDelay(int pDelay);

    double size();

    bool run();
    bool pause();
    bool resume();
    bool stop();

    bool reset();

private:
    QString mFileName;

    FileType mFileType;

    CellMLSupport::CellmlFile *mCellmlFile;
    SEDMLSupport::SedmlFile *mSedmlFile;
    COMBINESupport::CombineArchive *mCombineArchive;

    CellMLSupport::CellmlFileRuntime *mRuntime;

    SimulationWorker *mWorker;

    SimulationData *mData;
    SimulationResults *mResults;

    QEventLoop *mWorkerFinishedEventLoop;

    void retrieveFileDetails(bool pRecreateRuntime = true);

    bool simulationSettingsOk(bool pEmitSignal = true);

signals:
    void running(bool pIsResuming);
    void paused();
    void stopped(qint64 pElapsedTime);

    void error(const QString &pMessage);
};

//==============================================================================

}   // namespace SimulationSupport
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
