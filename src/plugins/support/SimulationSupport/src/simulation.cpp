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

#include "cellmlfilemanager.h"
#include "combinefilemanager.h"
#include "interfaces.h"
#include "sedmlfilemanager.h"
#include "simulation.h"
#include "simulationworker.h"

//==============================================================================

#include <QSet>
#include <QVector>
#include <QtMath>

//==============================================================================

namespace OpenCOR {
namespace SimulationSupport {

//==============================================================================

SimulationData::SimulationData(Simulation *pSimulation) :
    mSimulation(pSimulation),
    mDelay(0),
    mStartingPoint(0.0),
    mEndingPoint(1000.0),
    mPointInterval(1.0),
    mOdeSolverName(QString()),
    mOdeSolverProperties(Solver::Solver::Properties()),
    mDaeSolverName(QString()),
    mDaeSolverProperties(Solver::Solver::Properties()),
    mNlaSolverName(QString()),
    mNlaSolverProperties(Solver::Solver::Properties()),
    mResultsDataStore(0),
    mGradientsDataStore(0),
    mGradientIndices(QVector<int>()),
    mGradientsArray(0),
    mSimulationDataUpdatedFunction(std::bind(&SimulationData::updateParameters, this))
{
    // Create our various arrays

    createArrays();

    createResultsDataStore();
}

//==============================================================================

SimulationData::~SimulationData()
{
    // Delete some internal objects

    deleteArrays();

    delete[] mGradientsArray;
}

//==============================================================================

void SimulationData::reload()
{
    // Reload ourselves by deleting and recreating our arrays

    deleteArrays();
    createArrays();
}

//==============================================================================

Simulation * SimulationData::simulation() const
{
    // Return our simulation

    return mSimulation;
}

//==============================================================================

double * SimulationData::constants() const
{
    // Return our constants array

    return mConstantsArray;
}

//==============================================================================

double * SimulationData::rates() const
{
    // Return our rates array

    return mRatesArray;
}

//==============================================================================

double * SimulationData::states() const
{
    // Return our states array

    return mStatesArray;
}

//==============================================================================

double * SimulationData::algebraic() const
{
    // Return our algebraic array

    return mAlgebraicArray;
}

//==============================================================================

double * SimulationData::condVar() const
{
    // Return our condVar array

    return mCondVarArray;
}

//==============================================================================

int SimulationData::delay() const
{
    // Return our delay

    return mDelay;
}

//==============================================================================

void SimulationData::setDelay(const int &pDelay)
{
    // Set our delay

    mDelay = pDelay;
}

//==============================================================================

double SimulationData::startingPoint() const
{
    // Return our starting point

    return mStartingPoint;
}

//==============================================================================

void SimulationData::setStartingPoint(const double &pStartingPoint,
                                      const bool &pRecompute)
{
    // Set our starting point

    mStartingPoint = pStartingPoint;

    // Recompute our 'computed constants' and 'variables', i.e. reset ourselves
    // witout initialisation (hence passing false to reset())

    if (pRecompute)
        reset(false);
}

//==============================================================================

double SimulationData::endingPoint() const
{
    // Return our ending point

    return mEndingPoint;
}

//==============================================================================

void SimulationData::setEndingPoint(const double &pEndingPoint)
{
    // Set our ending point

    mEndingPoint = pEndingPoint;
}

//==============================================================================

double SimulationData::pointInterval() const
{
    // Return our point interval

    return mPointInterval;
}

//==============================================================================

void SimulationData::setPointInterval(const double &pPointInterval)
{
    // Set our point interval

    mPointInterval = pPointInterval;
}

//==============================================================================

SolverInterface * SimulationData::solverInterface(const QString &pSolverName) const
{
    // Return the named solver interface, if any

    foreach (SolverInterface *solverInterface, Core::solverInterfaces()) {
        if (!solverInterface->solverName().compare(pSolverName))
            return solverInterface;
    }

    return 0;
}

//==============================================================================

SolverInterface * SimulationData::odeSolverInterface() const
{
    // Return our ODE solver interface, if any

    return solverInterface(odeSolverName());
}

//==============================================================================

QString SimulationData::odeSolverName() const
{
    // Return our ODE solver name

    return (   mSimulation->runtime()
            && mSimulation->runtime()->needOdeSolver())?mOdeSolverName:QString();
}

//==============================================================================

void SimulationData::setOdeSolverName(const QString &pOdeSolverName)
{
    // Set our ODE solver name and reset its properties

    if (mSimulation->runtime() && mSimulation->runtime()->needOdeSolver()) {
        mOdeSolverName = pOdeSolverName;

        mOdeSolverProperties.clear();
    }
}

//==============================================================================

Solver::Solver::Properties SimulationData::odeSolverProperties() const
{
    // Return our ODE solver's properties

    return (   mSimulation->runtime()
            && mSimulation->runtime()->needOdeSolver())?mOdeSolverProperties:Solver::Solver::Properties();
}

//==============================================================================

void SimulationData::addOdeSolverProperty(const QString &pName,
                                          const QVariant &pValue)
{
    // Add an ODE solver property

    if (mSimulation->runtime() && mSimulation->runtime()->needOdeSolver())
        mOdeSolverProperties.insert(pName, pValue);
}

//==============================================================================

SolverInterface * SimulationData::daeSolverInterface() const
{
    // Return our DAE solver interface, if any

    return solverInterface(daeSolverName());
}

//==============================================================================

QString SimulationData::daeSolverName() const
{
    // Return our DAE solver name

    return (   mSimulation->runtime()
            && mSimulation->runtime()->needDaeSolver())?mDaeSolverName:QString();
}

//==============================================================================

void SimulationData::setDaeSolverName(const QString &pDaeSolverName)
{
    // Set our DAE solver name and reset its properties

    if (mSimulation->runtime() && mSimulation->runtime()->needDaeSolver()) {
        mDaeSolverName = pDaeSolverName;

        mDaeSolverProperties.clear();
    }
}

//==============================================================================

Solver::Solver::Properties SimulationData::daeSolverProperties() const
{
    // Return our DAE solver's properties

    return (   mSimulation->runtime()
            && mSimulation->runtime()->needDaeSolver())?mDaeSolverProperties:Solver::Solver::Properties();
}

//==============================================================================

void SimulationData::addDaeSolverProperty(const QString &pName,
                                          const QVariant &pValue)
{
    // Add an DAE solver property

    if (mSimulation->runtime() && mSimulation->runtime()->needDaeSolver())
        mDaeSolverProperties.insert(pName, pValue);
}

//==============================================================================

SolverInterface * SimulationData::nlaSolverInterface() const
{
    // Return our NLA solver interface, if any

    return solverInterface(nlaSolverName());
}

//==============================================================================

QString SimulationData::nlaSolverName() const
{
    // Return our NLA solver name

    return (   mSimulation->runtime()
            && mSimulation->runtime()->needNlaSolver())?mNlaSolverName:QString();
}

//==============================================================================

void SimulationData::setNlaSolverName(const QString &pNlaSolverName,
                                      const bool &pReset)
{
    // Set our NLA solver name and reset its properties

    if (mSimulation->runtime() && mSimulation->runtime()->needNlaSolver()) {
        mNlaSolverName = pNlaSolverName;

        mNlaSolverProperties.clear();

        // Reset our parameter values, if required
        // Note: to only recompute our 'computed constants' and 'variables' is
        //       not sufficient since some constants may also need to be
        //       reinitialised...

        if (pReset)
            reset();
    }
}

//==============================================================================

Solver::Solver::Properties SimulationData::nlaSolverProperties() const
{
    // Return our NLA solver's properties

    return (   mSimulation->runtime()
            && mSimulation->runtime()->needNlaSolver())?mNlaSolverProperties:Solver::Solver::Properties();
}

//==============================================================================

void SimulationData::addNlaSolverProperty(const QString &pName,
                                          const QVariant &pValue,
                                          const bool &pReset)
{
    // Add an NLA solver property

    if (mSimulation->runtime() && mSimulation->runtime()->needNlaSolver()) {
        mNlaSolverProperties.insert(pName, pValue);

        // Reset our parameter values, if required
        // Note: to only recompute our 'computed constants' and 'variables' is
        //       not sufficient since some constants may also need to be
        //       reinitialised...

        if (pReset)
            reset();
    }
}

//==============================================================================

void SimulationData::reset(const bool &pInitialize)
{
    // Reset our parameter values which means both initialising our 'constants'
    // and computing our 'computed constants' and 'variables'
    // Note #1: we must check whether our runtime needs NLA solver and, if so,
    //          then retrieve an instance of our NLA solver since some of the
    //          resetting may require solving one or several NLA systems...
    // Note #2: recomputeComputedConstantsAndVariables() will let people know
    //          that our data has changed...

    CellMLSupport::CellmlFileRuntime *runtime = mSimulation->runtime();

    if (!runtime)
        return;

    Solver::NlaSolver *nlaSolver = 0;

    if (runtime->needNlaSolver()) {
        // Set our NLA solver
        // Note: we unset it at the end of this method...

        nlaSolver = static_cast<Solver::NlaSolver *>(nlaSolverInterface()->solverInstance());

        Solver::setNlaSolver(runtime->address(), nlaSolver);

        // Keep track of any error that might be reported by our NLA solver

        connect(nlaSolver, SIGNAL(error(const QString &)),
                this, SIGNAL(error(const QString &)));

        // Initialise our NLA solver

        nlaSolver->setProperties(mNlaSolverProperties);
    }

    // Reset our parameter values

    if (pInitialize) {
        memset(mConstantsArray, 0, runtime->constantsCount()*Solver::SizeOfDouble);
        memset(mRatesArray, 0, runtime->ratesCount()*Solver::SizeOfDouble);
        memset(mStatesArray, 0, runtime->statesCount()*Solver::SizeOfDouble);
        memset(mAlgebraicArray, 0, runtime->algebraicCount()*Solver::SizeOfDouble);
        memset(mCondVarArray, 0, runtime->condVarCount()*Solver::SizeOfDouble);

        runtime->initializeConstants()(mConstantsArray, mRatesArray, mStatesArray);
    }

    recomputeComputedConstantsAndVariables(mStartingPoint, pInitialize);

    // Delete our NLA solver, if any

    if (nlaSolver) {
        delete nlaSolver;

        Solver::unsetNlaSolver(runtime->address());
    }

    // Keep track of our various initial values

    if (pInitialize) {
        memcpy(mInitialConstantsArray, mConstantsArray, runtime->constantsCount()*Solver::SizeOfDouble);
        memcpy(mInitialStatesArray, mStatesArray, runtime->statesCount()*Solver::SizeOfDouble);
    }

    // Let people know whether our data is 'cleaned', i.e. not modified
    // Note: no point in checking if we are initialising...

    if (!pInitialize)
        emit modified(isModified());
}

//==============================================================================

void SimulationData::recomputeComputedConstantsAndVariables(const double &pCurrentPoint,
                                                            const bool &pInitialize)
{
    // Make sure that our runtime is valid

    CellMLSupport::CellmlFileRuntime *runtime = mSimulation->runtime();

    if (runtime && runtime->isValid()) {
        // Recompute our 'computed constants'

        runtime->computeComputedConstants()(mConstantsArray, mRatesArray,
                                            pInitialize?mStatesArray:mDummyStatesArray);

        // Recompute some 'constant' algebraic variables

        if (Runtime->modelType() == CellMLSupport::CellmlFileRuntime::Ode) {
            Runtime->computeOdeRates()(pCurrentPoint, mConstantsArray, mRatesArray,
                                       mStatesArray, mAlgebraicArray);
        }

        // Recompute our 'variables'

        recomputeVariables(pCurrentPoint);

        // Let people know that our data has been updated

        emit updatedParameters(pCurrentPoint);
    }
}

//==============================================================================

void SimulationData::recomputeVariables(const double &pCurrentPoint)
{
    // Recompute our 'variables'

    CellMLSupport::CellmlFileRuntime *runtime = mSimulation->runtime();

    if (!runtime)
        return;

    if (runtime->modelType() == CellMLSupport::CellmlFileRuntime::Ode)
        runtime->computeOdeVariables()(pCurrentPoint, mConstantsArray, mRatesArray,
                                       mStatesArray, mAlgebraicArray);
    else
        runtime->computeDaeVariables()(pCurrentPoint, mConstantsArray, mRatesArray,
                                       mStatesArray, mAlgebraicArray, mCondVarArray);
}

//==============================================================================

bool SimulationData::isModified() const
{
    CellMLSupport::CellmlFileRuntime *runtime = mSimulation->runtime();

    if (!runtime)
        return false;

    // Check whether any of our constants or states has been modified
    // Note: we start with our states since they are more likely to be modified
    //       than our constants...

    for (int i = 0, iMax = runtime->statesCount(); i < iMax; ++i) {
        if (!qIsFinite(mStatesArray[i]) || (mStatesArray[i] != mInitialStatesArray[i]))
            return true;
    }

    for (int i = 0, iMax = runtime->constantsCount(); i < iMax; ++i) {
        if (!qIsFinite(mConstantsArray[i]) || (mConstantsArray[i] != mInitialConstantsArray[i]))
            return true;
    }

    return false;
}

//==============================================================================

void SimulationData::checkForModifications()
{
    // Let people know whether any of our constants or states has been modified

    emit modified(isModified());
}

//==============================================================================

void SimulationData::updateParameters(SimulationData *pSimulationData)
{
    emit pSimulationData->updatedParameters(pSimulationData->mStartingPoint);
}

//==============================================================================

void SimulationData::createArrays()
{
    // Create our various arrays, if possible

    CellMLSupport::CellmlFileRuntime *runtime = mSimulation->runtime();

    if (runtime) {
        // Create our various arrays to compute our model

        mConstantsArray = new double[runtime->constantsCount()];
        mRatesArray = new double[runtime->ratesCount()];
        mStatesArray = new double[runtime->statesCount()];
        mDummyStatesArray = new double[runtime->statesCount()];
        mAlgebraicArray = new double[runtime->algebraicCount()];
        mCondVarArray = new double[runtime->condVarCount()];

        // Create our various arrays to keep track of our various initial values

        mInitialConstantsArray = new double[runtime->constantsCount()];
        mInitialStatesArray = new double[runtime->statesCount()];

        // Initialise our data store variables with their data arrays

        if (mResultsDataStore) {
            mConstantVariables.setNextValuePtrs(mConstantsArray);
            mRateVariables.setNextValuePtrs(mRatesArray);
            mStateVariables.setNextValuePtrs(mStatesArray);
            mAlgebraicVariables.setNextValuePtrs(mAlgebraicArray);
        }
    } else {
        mConstantsArray = mRatesArray = mStatesArray = mDummyStatesArray = mAlgebraicArray = mCondVarArray = 0;
        mInitialConstantsArray = mInitialStatesArray = 0;
    }
}

//==============================================================================

void SimulationData::deleteArrays()
{
    // Delete our various arrays

    delete[] mConstantsArray;
    delete[] mRatesArray;
    delete[] mStatesArray;
    delete[] mDummyStatesArray;
    delete[] mAlgebraicArray;
    delete[] mCondVarArray;

    delete[] mInitialConstantsArray;
    delete[] mInitialStatesArray;

    // Clear our data store variables

    if (mResultsDataStore) {
        mConstantVariables.clearNextValuePtrs();
        mRateVariables.clearNextValuePtrs();
        mStateVariables.clearNextValuePtrs();
        mAlgebraicVariables.clearNextValuePtrs();
    }
}

//==============================================================================

QString SimulationData::uri(const QStringList &pComponentHierarchy,
                            const QString &pName)
{
    // Generate an URI using the given component hierarchy and name

    QString res = pComponentHierarchy.join('/')+"/"+pName;

    return res.replace('\'', "/prime");
}

//==============================================================================

void SimulationData::createResultsDataStore()
{
    // Make sure that we have a runtime

    CellMLSupport::CellmlFileRuntime *runtime = mSimulation->runtime();

    if (!runtime)
        return true;

    // Create our data store and populate it with a variable of integration, as
    // well as with constant, rate, state and algebraic variables

    mResultsDataStore = new DataStore::DataStore(mRuntime->cellmlFile()->xmlBase());

    mPointVariable = mResultsDataStore->addVoi();
    mConstantVariables = mResultsDataStore->addVariables(runtime->constantsCount(), mConstantsArray);
    mRateVariables = mResultsDataStore->addVariables(runtime->ratesCount(), mRatesArray);
    mStateVariables = mResultsDataStore->addVariables(runtime->statesCount(), mStatesArray);
    mAlgebraicVariables = mResultsDataStore->addVariables(runtime->algebraicCount(), mAlgebraicArray);

    // Customise our variable of integration, as well as our constant, rate,
    // state and algebraic variables

    for (int i = 0, iMax = runtime->parameters().count(); i < iMax; ++i) {
        CellMLSupport::CellmlFileRuntimeParameter *parameter = runtime->parameters()[i];
        DataStore::DataStoreVariable *variable = 0;

        switch (parameter->type()) {
        case CellMLSupport::CellmlFileRuntimeParameter::Voi:
            mPointVariable->setIcon(parameter->icon());
            mPointVariable->setUri(uri(runtime->variableOfIntegration()->componentHierarchy(),
                                   runtime->variableOfIntegration()->name()));
            mPointVariable->setLabel(runtime->variableOfIntegration()->name());
            mPointVariable->setUnit(runtime->variableOfIntegration()->unit());

            break;
        case CellMLSupport::CellmlFileRuntimeParameter::Constant:
        case CellMLSupport::CellmlFileRuntimeParameter::ComputedConstant:
            variable = mConstantVariables[parameter->index()];

            break;
        case CellMLSupport::CellmlFileRuntimeParameter::Rate:
            variable = mRateVariables[parameter->index()];

            break;
        case CellMLSupport::CellmlFileRuntimeParameter::State:
            variable = mStateVariables[parameter->index()];

            break;
        case CellMLSupport::CellmlFileRuntimeParameter::Algebraic:
            variable = mAlgebraicVariables[parameter->index()];

            break;
        default:
            // Not a relevant type, so do nothing

            ;
        }

        if (variable) {
            variable->setIcon(parameter->icon());
            variable->setUri(uri(parameter->componentHierarchy(), parameter->formattedName()));
            variable->setLabel(parameter->formattedName());
            variable->setUnit(parameter->formattedUnit(runtime->variableOfIntegration()->unit()));
        }
    }
}

//==============================================================================

DataStore::DataStore * SimulationData::resultsDataStore() const
{
    return mResultsDataStore;
}

//==============================================================================

DataStore::DataStoreVariable * SimulationData::pointVariable() const
{
    return mPointVariable;
}

//==============================================================================

DataStore::DataStoreVariables SimulationData::constantVariables() const
{
    return mConstantVariables;
}

//==============================================================================

DataStore::DataStoreVariables SimulationData::rateVariables() const
{
    return mRateVariables;
}

//==============================================================================

DataStore::DataStoreVariables SimulationData::stateVariables() const
{
    return mStateVariables;
}

//==============================================================================

DataStore::DataStoreVariables SimulationData::algebraicVariables() const
{
    return mAlgebraicVariables;
}

//==============================================================================

void SimulationData::calculateGradients(const int &pIndex, bool pCalculate)
{
    int pos = mGradientIndices.indexOf(pIndex);

    if (pCalculate) {
        if (pos == -1) mGradientIndices.append(pIndex);
    } else if (pos > -1) {
        mGradientIndices.remove(pos);
    }
}

//==============================================================================

void SimulationData::calculateGradients(const QString &pConstantUri, bool pCalculate)
{
    // Make sure that we have a runtime

    CellMLSupport::CellmlFileRuntime *runtime = mSimulation->runtime();

    if (!runtime)
        return true;

    for (int i = 0, iMax = runtime->parameters().count(); i < iMax; ++i) {
        CellMLSupport::CellmlFileRuntimeParameter *parameter = runtime->parameters()[i];

        if (parameter->type() == CellMLSupport::CellmlFileRuntimeParameter::Constant
                && pConstantUri == mConstantVariables[parameter->index()]->uri()) {
            calculateGradients(parameter->index(), pCalculate);

            // Update icon in parameter information widget

            emit gradientCalculation(parameter, pCalculate);
        }
    }
}

//==============================================================================

bool SimulationData::createGradientsDataStore()
{
    // Delete the previous gradients array and data store

    delete[] mGradientsArray;
    delete mGradientsDataStore;

    // Indices to all constants having gradients calculated

    QSet<int> gradientIndices = QSet<int>::fromList(mGradientIndices.toList());

    // Reset variables in case of early return

    mGradientsArray = 0;
    mGradientsDataStore = 0;

    // Make sure that we have a runtime

    CellMLSupport::CellmlFileRuntime *runtime = mSimulation->runtime();

    if (!runtime)
        return true;

    // Retrieve the size of our data and make sure that it is valid

    qulonglong simulationSize = qulonglong(mSimulation->size());

    if (!simulationSize)
        return true;

    // Gradients are calculated for each state variable wrt each constant

    int gradientsCount = gradientIndices.size();

    if (gradientsCount) {
        try {
            int statesCount = runtime->statesCount();

            int gradientsSize = gradientsCount*statesCount;

            // Allocate the array to hold sensitivity gradients at a single point

            mGradientsArray = new double[gradientsSize]();

            // Allocate a DataStore and variables to hold gradients for the simulation

            mGradientsDataStore = new DataStore::DataStore(runtime->cellmlFile()->xmlBase() + "/gradients");

            mGradientVariables = mGradientsDataStore->addVariables(gradientsSize, mGradientsArray);

            mGradientsDataStore->createArrays(simulationSize);

            // All is well so remember in a sorted vector the constant indices
            // for which we will have gradients

            mGradientIndices = gradientIndices.toList().toVector();
            std::sort(mGradientIndices.begin(), mGradientIndices.end());

            // Customise our gradient variables

            for (int i = 0; i < gradientsCount; ++i) {
                DataStore::DataStoreVariable *constant = mConstantVariables[mGradientIndices[i]];

                for (int j = 0; j < statesCount; ++j) {
                    DataStore::DataStoreVariable *state = mStateVariables[j];
                    DataStore::DataStoreVariable *gradient = mGradientVariables[i*statesCount + j];

                    // Gradient is of state variable wrt each constant

                    gradient->setUri(state->uri() + "/gradient_with/" + constant->uri());
                    gradient->setLabel("d(" + state->label() + ")/d(" + constant->label() + ")");
                }
            }
        } catch (...) {
            delete mGradientsArray;
            mGradientsArray = 0;

            delete mGradientsDataStore;
            mGradientsDataStore = 0;
            mGradientIndices = QVector<int>();

            return false;
        }
    }

    return true;
}

//==============================================================================

double * SimulationData::gradients() const
{
    // Return our algebraic array

    return mGradientsArray;
}

//==============================================================================

int SimulationData::gradientsCount() const
{
    // Return the number of gradient indices

    return mGradientIndices.size();
}

//==============================================================================

DataStore::DataStore * SimulationData::gradientsDataStore() const
{
    return mGradientsDataStore;
}

//==============================================================================

int * SimulationData::gradientIndices()
{
    // Return our gradient's indices

    return mGradientIndices.data();
}

//==============================================================================

DataStore::DataStoreVariables SimulationData::gradientVariables() const
{
    return mGradientVariables;
}

//==============================================================================

SimulationResults::SimulationResults(Simulation *pSimulation) :
    mSimulation(pSimulation),
    mDataStore(pSimulation->data()->resultsDataStore()),
    mGradientsDataStore(0),
    mPointVariable(pSimulation->data()->pointVariable()),
    mConstantVariables(pSimulation->data()->constantVariables()),
    mRateVariables(pSimulation->data()->rateVariables()),
    mStateVariables(pSimulation->data()->stateVariables()),
    mAlgebraicVariables(pSimulation->data()->algebraicVariables())
{
}

//==============================================================================

SimulationResults::~SimulationResults()
{
    // Delete some internal objects

    deleteDataStoreArrays();
}

//==============================================================================

bool SimulationResults::createDataStoreArrays()
{
    // Note: the boolean value we return is true if we have had no problem
    //       allocating our data store arrays, false otherwise. This is the
    //       reason, for example, we return true when there is either no
    //       runtime or if the simulation size is zero...

    // Delete the previous data store arrays, if any

    mDataStore->deleteArrays();

    // Make sure that we have a runtime

    // Make sure that we have a runtime

    CellMLSupport::CellmlFileRuntime *runtime = mSimulation->runtime();

    if (!runtime)
        return true;

    // Retrieve the size of our data and make sure that it is valid

    qulonglong simulationSize = qulonglong(mSimulation->size());

    if (!simulationSize)
        return true;

    // Allocate our data store arrays

    try {
        mDataStore->createArrays(simulationSize);

    } catch (...) {
        mDataStore->deleteArrays();

        return false;
    }

    return true;
}

//==============================================================================

void SimulationResults::deleteDataStoreArrays()
{
    // Deallocate our data store arrays

    mDataStore->deleteArrays();
}

//==============================================================================

bool SimulationResults::createGradientsDataStore()
{
    bool result = mSimulation->data()->createGradientsDataStore();

    mGradientsDataStore = mSimulation->data()->gradientsDataStore();

    return result;
}

//==============================================================================

void SimulationResults::reload()
{
    // Reload ourselves by deleting our data store arrays

    deleteDataStoreArrays();
}

//==============================================================================

bool SimulationResults::reset(const bool &pAllocateArrays)
{
    // Reset our data store arrays

    if (pAllocateArrays) {
        return createDataStoreArrays();
    } else {
        deleteDataStoreArrays();

        return true;
    }
}

//==============================================================================

void SimulationResults::addPoint(const double &pPoint)
{
    // Add the data to our data store

    mDataStore->addValues(pPoint);

    // Add sensitivity gradients to their data store

    if (mGradientsDataStore)
        mGradientsDataStore->addValues(pPoint);
}

//==============================================================================

qulonglong SimulationResults::size() const
{
    // Return our size

    return mDataStore?mDataStore->size():0;
}

//==============================================================================

DataStore::DataStore * SimulationResults::dataStore() const
{
    // Return our data store

    return mDataStore;
}

//==============================================================================

double * SimulationResults::points() const
{
    // Return our points

    return mPointVariable?mPointVariable->values():0;
}

//==============================================================================

double * SimulationResults::constants(const int &pIndex) const
{
    // Return our constants data at the given index

    return mConstantVariables.isEmpty()?0:mConstantVariables[pIndex]->values();
}

//==============================================================================

double * SimulationResults::rates(const int &pIndex) const
{
    // Return our rates data at the given index

    return mRateVariables.isEmpty()?0:mRateVariables[pIndex]->values();
}

//==============================================================================

double * SimulationResults::states(const int &pIndex) const
{
    // Return our states data at the given index

    return mStateVariables.isEmpty()?0:mStateVariables[pIndex]->values();
}

//==============================================================================

double * SimulationResults::algebraic(const int &pIndex) const
{
    // Return our algebraic data at the given index

    return mAlgebraicVariables.isEmpty()?0:mAlgebraicVariables[pIndex]->values();
}

//==============================================================================

Simulation::Simulation(const QString &pFileName) :
    mFileName(pFileName),
    mWorker(0)
{
    // Retrieve our file details

    retrieveFileDetails();

    // Create our data and results objects, now that we are all set

    mData = new SimulationData(this);
    mResults = new SimulationResults(this);

    // Keep track of any error occurring in our data

    connect(mData, SIGNAL(error(const QString &)),
            this, SIGNAL(error(const QString &)));
}

//==============================================================================

Simulation::~Simulation()
{
    // Stop our worker

    stop();

    // Delete some internal objects

    delete mResults;
    delete mData;
}

//==============================================================================

QString Simulation::fileName() const
{
    // Return our file name

    return mFileName;
}

//==============================================================================

void Simulation::retrieveFileDetails()
{
    // Retrieve our CellML and SED-ML files, as well as COMBINE archive

    mCellmlFile = CellMLSupport::CellmlFileManager::instance()->cellmlFile(mFileName);
    mSedmlFile = mCellmlFile?0:SEDMLSupport::SedmlFileManager::instance()->sedmlFile(mFileName);
    mCombineArchive = mSedmlFile?0:COMBINESupport::CombineFileManager::instance()->combineArchive(mFileName);

    // Determine the type of our file

    mFileType = mCellmlFile?CellmlFile:mSedmlFile?SedmlFile:CombineArchive;

    // We have a COMBINE archive, so we need to retrieve its corresponding
    // SED-ML file

    if (mCombineArchive)
        mSedmlFile = mCombineArchive->sedmlFile();

    // We have a SED-ML file (either a direct one or through a COMBINE archive),
    // so we need to retrieve its corresponding CellML file

    if (mSedmlFile)
        mCellmlFile = mSedmlFile->cellmlFile();

    // Keep track of our runtime, if any

    mRuntime = mCellmlFile?mCellmlFile->runtime(true):0;
}

//==============================================================================

QString Simulation::fileName() const
{
    // Return our file name

    return mFileName;
}

//==============================================================================

void Simulation::save()
{
    // Retrieve our file details

    retrieveFileDetails();
}

//==============================================================================

void Simulation::reload()
{
    // Stop our worker
    // Note: we don't need to delete mWorker since it will be done as part of
    //       its thread being stopped...

    stop();

    // Retrieve our file details

    retrieveFileDetails();

    // Ask our data and results to update themselves

    mData->reload();
    mResults->reload();
}

//==============================================================================

void Simulation::rename(const QString &pFileName)
{
    // Rename ourselves by simply updating our file name

    mFileName = pFileName;
}

//==============================================================================

CellMLSupport::CellmlFileRuntime * Simulation::runtime() const
{
    // Return our runtime

    return mRuntime;
}

//==============================================================================

Simulation::FileType Simulation::fileType() const
{
    // Return our file type

    return mFileType;
}

//==============================================================================

CellMLSupport::CellmlFile * Simulation::cellmlFile() const
{
    // Return our CellML file object

    return mCellmlFile;
}

//==============================================================================

SEDMLSupport::SedmlFile * Simulation::sedmlFile() const
{
    // Return our SED-ML file object

    return mSedmlFile;
}

//==============================================================================

COMBINESupport::CombineArchive * Simulation::combineArchive() const
{
    // Return our COBMINE archive object

    return mCombineArchive;
}

//==============================================================================

SimulationData * Simulation::data() const
{
    // Return our data

    return mData;
}

//==============================================================================

SimulationResults * Simulation::results() const
{
    // Return our results

    return mResults;
}

//==============================================================================

bool Simulation::isRunning() const
{
    // Return whether we are running

    return mWorker?mWorker->isRunning():false;
}

//==============================================================================

bool Simulation::isPaused() const
{
    // Return whether we are paused

    return mWorker?mWorker->isPaused():false;
}

//==============================================================================

double Simulation::currentPoint() const
{
    // Return our current point

    return mWorker?mWorker->currentPoint():data()->startingPoint();
}

//==============================================================================

int Simulation::delay() const
{
    // Return our delay

    return mData->delay();
}

//==============================================================================

void Simulation::setDelay(const int &pDelay)
{
    // Set our delay

    mData->setDelay(pDelay);
}

//==============================================================================

bool Simulation::simulationSettingsOk(const bool &pEmitSignal)
{
    // Check and return whether our simulation settings are sound

    if (mData->startingPoint() == mData->endingPoint()) {
        if (pEmitSignal)
            emit error(tr("the starting and ending points cannot have the same value"));

        return false;
    } else if (mData->pointInterval() == 0.0) {
        if (pEmitSignal)
            emit error(tr("the point interval cannot be equal to zero"));

        return false;
    } else if (   (mData->startingPoint() < mData->endingPoint())
             && (mData->pointInterval() < 0.0)) {
        if (pEmitSignal)
            emit error(tr("the ending point is greater than the starting point, so the point interval should be greater than zero"));

        return false;
    } else if (   (mData->startingPoint() > mData->endingPoint())
             && (mData->pointInterval() > 0.0)) {
        if (pEmitSignal)
            emit error(tr("the ending point is smaller than the starting point, so the point interval should be smaller than zero"));

        return false;
    } else {
        return true;
    }
}

//==============================================================================

double Simulation::size()
{
    // Return the size of our simulation (i.e. the number of data points that
    // should be generated), if possible
    // Note: we return a double rather than a qulonglong in case the simulation
    //       requires an insane amount of memory...

    if (simulationSettingsOk(false))
        return ceil((mData->endingPoint()-mData->startingPoint())/mData->pointInterval())+1.0;
    else
        return 0.0;
}

//==============================================================================

bool Simulation::run()
{
    if (!mRuntime)
        return false;

    // Initialise our worker, if not active

    if (mWorker) {
        return false;
    } else {
        // Make sure that the simulation settings we were given are sound

        if (!simulationSettingsOk())
            return false;

        // Make sure we have a data store in which to store results

        if (!mResults->dataStore()) {
            emit error(tr("no datastore for results"));

            return false;
        }

        // Make sure the data store is big enough to store our results

        if (size() > (mResults->dataStore()->capacity() - mResults->dataStore()->size())) {
            emit error(tr("datastore doesn't have enough capacity"));

            return false;
        }

        // Make sure we have a valid solver

        if ((mRuntime->needOdeSolver() && !mData->odeSolverInterface())
         || (!mRuntime->needOdeSolver() && !mData->daeSolverInterface())
         || (mRuntime->needNlaSolver() && !mData->nlaSolverInterface())) {
            emit error(tr("no valid solvers"));

            return false;
        }

        // Create our worker

        mWorker = new SimulationWorker(this, mWorker);

        if (!mWorker) {
            emit error(tr("the simulation worker could not be created"));

            return false;
        }

        // Create a few connections

        connect(mWorker, SIGNAL(running(const bool &)),
                this, SIGNAL(running(const bool &)));
        connect(mWorker, SIGNAL(paused()),
                this, SIGNAL(paused()));

        connect(mWorker, SIGNAL(finished(const qint64 &)),
                this, SIGNAL(stopped(const qint64 &)));

        connect(mWorker, SIGNAL(error(const QString &)),
                this, SIGNAL(error(const QString &)));

        // Start our worker

        return mWorker->run();
    }
}

//==============================================================================

bool Simulation::pause()
{
    // Pause our worker

    return mWorker?mWorker->pause():false;
}

//==============================================================================

bool Simulation::resume()
{
    // Resume our worker

    return mWorker?mWorker->resume():false;
}

//==============================================================================

bool Simulation::stop()
{
    // Stop our worker

    return mWorker?mWorker->stop():false;
}

//==============================================================================

bool Simulation::reset()
{
    // Reset our data

    mData->reset();

    // Reset our worker

    return mWorker?mWorker->reset():false;
}

//==============================================================================

}   // namespace SimulationSupport
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
