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
// Simulation worker
//==============================================================================

#pragma once

//==============================================================================

#include <QObject>
#include <QWaitCondition>

//==============================================================================

namespace OpenCOR {

//==============================================================================

namespace CellMLSupport {
    class CellmlFileRuntime;
}   // namespace CellMLSupport

//==============================================================================

namespace SimulationSupport {

//==============================================================================

class Simulation;

//==============================================================================

class SimulationWorker : public QObject
{
    Q_OBJECT

public:
    explicit SimulationWorker(Simulation *pSimulation,
                              SimulationWorker *&pSelf);

    bool isRunning() const;
    bool isPaused() const;

    double currentPoint() const;

    bool run();
    bool pause();
    bool resume();
    bool stop();

    bool reset();

private:
    QThread *mThread;

    Simulation *mSimulation;

    CellMLSupport::CellmlFileRuntime *mRuntime;

    double mCurrentPoint;

    bool mPaused;
    bool mStopped;

    bool mReset;

    QWaitCondition mPausedCondition;

    bool mError;

    SimulationWorker *&mSelf;

signals:
    void running(bool pIsResuming);
    void paused();

    void finished(qint64 pElapsedTime);

    void error(const QString &pMessage);

private slots:
    void started();

    void emitError(const QString &pMessage);
};

//==============================================================================

}   // namespace SimulationSupport
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
