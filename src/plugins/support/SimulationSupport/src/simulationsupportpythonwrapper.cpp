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
// Python wrapper for SimulationSupport classes
//==============================================================================

#include "corecliutils.h"
#include "cellmlfileruntime.h"
#include "datastoreinterface.h"
#include "datastorepythonwrapper.h"
#include "pythonsupport.h"
#include "simulation.h"
#include "simulationsupportplugin.h"
#include "simulationsupportpythonwrapper.h"

//==============================================================================

#include <QMap>

//==============================================================================

namespace OpenCOR {
namespace SimulationSupport {

//==============================================================================

SimulationSupportPythonWrapper::SimulationSupportPythonWrapper(PyObject *pModule, QObject *pParent) : QObject(pParent)
{
    Q_UNUSED(pModule);

    PythonSupport::registerClass(&OpenCOR::SimulationSupport::Simulation::staticMetaObject);
    PythonSupport::registerClass(&OpenCOR::SimulationSupport::SimulationData::staticMetaObject);
    PythonSupport::registerClass(&OpenCOR::SimulationSupport::SimulationResults::staticMetaObject);
    PythonSupport::addInstanceDecorators(this);
}

//==============================================================================

bool SimulationSupportPythonWrapper::run(Simulation *pSimulation)
{
    // Check that we have enough memory to run our simulation

    bool runSimulation = true;

    double freeMemory = Core::freeMemory();
    double requiredMemory = pSimulation->requiredMemory();

    if (requiredMemory > freeMemory) {
        throw std::runtime_error(
            tr("The simulation requires %1 of memory and you have only %2 left.")
                .arg(Core::sizeAsString(requiredMemory), Core::sizeAsString(freeMemory)).toStdString());
    } else {
        // Theoretically speaking, we have enough memory to run the
        // simulation, so try to allocate all the memory we need for the
        // simulation by resetting its settings

        runSimulation = pSimulation->results()->reset();

        // Effectively run our simulation in case we were able to
        // allocate all the memory we need to run the simulation

        if (runSimulation) {
            return pSimulation->run();
        } else {
            throw std::runtime_error(
                tr("We could not allocate the %1 of memory required for the simulation.")
                    .arg(Core::sizeAsString(requiredMemory)).toStdString());
        }
    }
    return false;
}

//==============================================================================

PyObject * SimulationSupportPythonWrapper::algebraic(SimulationData *pSimulationData) const
{
    return DataStore::DataStorePythonWrapper::dataStoreValuesDict(pSimulationData->algebraicVariables());
}

//==============================================================================

PyObject * SimulationSupportPythonWrapper::constants(SimulationData *pSimulationData) const
{
    return DataStore::DataStorePythonWrapper::dataStoreValuesDict(pSimulationData->constantVariables());
}

//==============================================================================

PyObject * SimulationSupportPythonWrapper::rates(SimulationData *pSimulationData) const
{
    return DataStore::DataStorePythonWrapper::dataStoreValuesDict(pSimulationData->rateVariables());
}

//==============================================================================

PyObject * SimulationSupportPythonWrapper::states(SimulationData *pSimulationData) const
{
    return DataStore::DataStorePythonWrapper::dataStoreValuesDict(pSimulationData->stateVariables());
}

//==============================================================================

const OpenCOR::DataStore::DataStoreVariable * SimulationSupportPythonWrapper::points(SimulationResults *pSimulationResults) const
{
    return pSimulationResults->mPointVariable;
}

//==============================================================================

PyObject * SimulationSupportPythonWrapper::algebraic(SimulationResults *pSimulationResults) const
{
    return DataStore::DataStorePythonWrapper::dataStoreVariablesDict(pSimulationResults->mAlgebraicVariables);
}

//==============================================================================

PyObject * SimulationSupportPythonWrapper::constants(SimulationResults *pSimulationResults) const
{
    return DataStore::DataStorePythonWrapper::dataStoreVariablesDict(pSimulationResults->mConstantVariables);
}

//==============================================================================

PyObject * SimulationSupportPythonWrapper::rates(SimulationResults *pSimulationResults) const
{
    return DataStore::DataStorePythonWrapper::dataStoreVariablesDict(pSimulationResults->mRateVariables);
}

//==============================================================================

PyObject * SimulationSupportPythonWrapper::states(SimulationResults *pSimulationResults) const
{
    return DataStore::DataStorePythonWrapper::dataStoreVariablesDict(pSimulationResults->mStateVariables);
}

//==============================================================================

PyObject * SimulationSupportPythonWrapper::gradients(SimulationResults *pSimulationResults) const
{
    SimulationData *simulationData = pSimulationResults->mSimulation->data();

    const DataStore::DataStoreVariables constantVariables = simulationData->constantVariables();
    const DataStore::DataStoreVariables stateVariables = simulationData->stateVariables();
    const DataStore::DataStoreVariables gradientVariables = simulationData->gradientVariables();

    int *indices = simulationData->gradientsIndices();

    int gradientsCount = simulationData->gradientsCount();
    int statesCount = pSimulationResults->mSimulation->runtime()->statesCount();

    PyObject *gradientsDict = PyDict_New();
    QMap<QString, PyObject *> stateGradientsDictionaries;

    // We need to transpose gradients when building dictionary

    for (int i = 0; i < gradientsCount; ++i) {
        DataStore::DataStoreVariable *constant = constantVariables[indices[i]];

        for (int j = 0; j < statesCount; ++j) {
            DataStore::DataStoreVariable *state = stateVariables[j];
            DataStore::DataStoreVariable *gradient = gradientVariables[i*statesCount + j];

            // Each state variable has a dictionary containing gradients wrt each constant

            PyObject *stateGradientsDict = stateGradientsDictionaries[state->uri()];
            if (stateGradientsDict == 0) {
                stateGradientsDict = PyDict_New();
                PyDict_SetItemString(gradientsDict, state->uri().toLatin1().data(), stateGradientsDict);
                stateGradientsDictionaries[state->uri()] = stateGradientsDict;
            }

            PythonSupport::addObject(stateGradientsDict, constant->uri(), gradient);
        }
    }
    return gradientsDict;
}

//==============================================================================

}   // namespace SimulationSupport
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
