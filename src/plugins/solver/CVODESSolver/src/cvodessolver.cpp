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
// CVODES solver
//==============================================================================

#include "cvodessolver.h"

//==============================================================================

#include "cvodes/cvodes.h"
#include "cvodes/cvodes_bandpre.h"
#include "cvodes/cvodes_diag.h"
#include "cvodes/cvodes_direct.h"
#include "cvodes/cvodes_spils.h"
#include "sunlinsol/sunlinsol_band.h"
#include "sunlinsol/sunlinsol_dense.h"
#include "sunlinsol/sunlinsol_spbcgs.h"
#include "sunlinsol/sunlinsol_spgmr.h"
#include "sunlinsol/sunlinsol_sptfqmr.h"

//==============================================================================

namespace OpenCOR {
namespace CVODESSolver {

//==============================================================================

int rhsFunction(double pVoi, N_Vector pStates, N_Vector pRates, void *pUserData)
{
    // Compute the RHS function

    CvodesSolverUserData *userData = static_cast<CvodesSolverUserData *>(pUserData);

    userData->computeRates()(pVoi, userData->constants(),
                             N_VGetArrayPointer_Serial(pRates),
                             N_VGetArrayPointer_Serial(pStates),
                             userData->algebraic());

    return 0;
}

//==============================================================================

void errorHandler(int pErrorCode, const char *pModule, const char *pFunction,
                  char *pErrorMessage, void *pUserData)
{
    Q_UNUSED(pModule);
    Q_UNUSED(pFunction);

    // Forward errors to our CvodesSolver object

    if (pErrorCode != CV_WARNING)
        static_cast<CvodesSolver *>(pUserData)->emitError(pErrorMessage);
}

//==============================================================================

CvodesSolverUserData::CvodesSolverUserData(double *pConstants, double *pAlgebraic,
                                           Solver::OdeSolver::ComputeRatesFunction pComputeRates) :
    mConstants(pConstants),
    mAlgebraic(pAlgebraic),
    mComputeRates(pComputeRates)
{
}

//==============================================================================

double * CvodesSolverUserData::constants() const
{
    // Return our constants array

    return mConstants;
}

//==============================================================================

double * CvodesSolverUserData::algebraic() const
{
    // Return our algebraic array

    return mAlgebraic;
}

//==============================================================================

Solver::OdeSolver::ComputeRatesFunction CvodesSolverUserData::computeRates() const
{
    // Return our compute rates function

    return mComputeRates;
}

//==============================================================================

CvodesSolver::CvodesSolver() :
    mSolver(0),
    mStatesVector(0),
    mMatrix(0),
    mLinearSolver(0),
    mUserData(0),
    mInterpolateSolution(InterpolateSolutionDefaultValue)
{
}

//==============================================================================

CvodesSolver::~CvodesSolver()
{
    // Make sure that the solver has been initialised

    if (!mSolver)
        return;

    // Delete some internal objects

    N_VDestroy_Serial(mStatesVector);
    SUNLinSolFree(mLinearSolver);
    SUNMatDestroy(mMatrix);

    CVodeFree(&mSolver);

    delete mUserData;
}

//==============================================================================

void CvodesSolver::initialize(double pVoi, int pRatesStatesCount,
                              double *pConstants, double *pRates,
                              double *pStates, double *pAlgebraic,
                              ComputeRatesFunction pComputeRates)
{
    // Retrieve our properties

    double maximumStep = MaximumStepDefaultValue;
    int maximumNumberOfSteps = MaximumNumberOfStepsDefaultValue;
    QString integrationMethod = IntegrationMethodDefaultValue;
    QString iterationType = IterationTypeDefaultValue;
    QString linearSolver = LinearSolverDefaultValue;
    QString preconditioner = PreconditionerDefaultValue;
    int upperHalfBandwidth = UpperHalfBandwidthDefaultValue;
    int lowerHalfBandwidth = LowerHalfBandwidthDefaultValue;
    double relativeTolerance = RelativeToleranceDefaultValue;
    double absoluteTolerance = AbsoluteToleranceDefaultValue;

    if (mProperties.contains(MaximumStepId)) {
        maximumStep = mProperties.value(MaximumStepId).toDouble();
    } else {
        emit error(tr("the \"Maximum step\" property value could not be retrieved"));

        return;
    }

    if (mProperties.contains(MaximumNumberOfStepsId)) {
        maximumNumberOfSteps = mProperties.value(MaximumNumberOfStepsId).toInt();
    } else {
        emit error(tr("the \"Maximum number of steps\" property value could not be retrieved"));

        return;
    }

    if (mProperties.contains(IntegrationMethodId)) {
        integrationMethod = mProperties.value(IntegrationMethodId).toString();
    } else {
        emit error(tr("the \"Integration method\" property value could not be retrieved"));

        return;
    }

    if (mProperties.contains(IterationTypeId)) {
        iterationType = mProperties.value(IterationTypeId).toString();

        if (!iterationType.compare(NewtonIteration)) {
            // We are dealing with a Newton iteration, so retrieve and check its
            // linear solver

            if (mProperties.contains(LinearSolverId)) {
                linearSolver = mProperties.value(LinearSolverId).toString();

                bool needUpperAndLowerHalfBandwidths = false;

                if (   !linearSolver.compare(DenseLinearSolver)
                    || !linearSolver.compare(DiagonalLinearSolver)) {
                    // We are dealing with a dense/diagonal linear solver, so
                    // nothing more to do
                } else if (!linearSolver.compare(BandedLinearSolver)) {
                    // We are dealing with a banded linear solver, so we need
                    // both an upper and a lower half bandwidth

                    needUpperAndLowerHalfBandwidths = true;
                } else {
                    // We are dealing with a GMRES/Bi-CGStab/TFQMR linear
                    // solver, so retrieve and check its preconditioner

                    if (mProperties.contains(PreconditionerId)) {
                        preconditioner = mProperties.value(PreconditionerId).toString();
                    } else {
                        emit error(tr("the \"Preconditioner\" property value could not be retrieved"));

                        return;
                    }

                    if (!preconditioner.compare(BandedPreconditioner)) {
                        // We are dealing with a banded preconditioner, so we
                        // need both an upper and a lower half bandwidth

                        needUpperAndLowerHalfBandwidths = true;
                    }
                }

                if (needUpperAndLowerHalfBandwidths) {
                    if (mProperties.contains(UpperHalfBandwidthId)) {
                        upperHalfBandwidth = mProperties.value(UpperHalfBandwidthId).toInt();

                        if (upperHalfBandwidth >= pRatesStatesCount) {
                            emit error(tr("the \"Upper half-bandwidth\" property must have a value between 0 and %1").arg(pRatesStatesCount-1));

                            return;
                        }
                    } else {
                        emit error(tr("the \"Upper half-bandwidth\" property value could not be retrieved"));

                        return;
                    }

                    if (mProperties.contains(LowerHalfBandwidthId)) {
                        lowerHalfBandwidth = mProperties.value(LowerHalfBandwidthId).toInt();

                        if (lowerHalfBandwidth >= pRatesStatesCount) {
                            emit error(tr("the \"Lower half-bandwidth\" property must have a value between 0 and %1").arg(pRatesStatesCount-1));

                            return;
                        }
                    } else {
                        emit error(tr("the \"Lower half-bandwidth\" property value could not be retrieved"));

                        return;
                    }
                }
            } else {
                emit error(tr("the \"Linear solver\" property value could not be retrieved"));

                return;
            }
        }
    } else {
        emit error(tr("the \"Iteration type\" property value could not be retrieved"));

        return;
    }

    if (mProperties.contains(RelativeToleranceId)) {
        relativeTolerance = mProperties.value(RelativeToleranceId).toDouble();
    } else {
        emit error(tr("the \"Relative tolerance\" property value could not be retrieved"));

        return;
    }

    if (mProperties.contains(AbsoluteToleranceId)) {
        absoluteTolerance = mProperties.value(AbsoluteToleranceId).toDouble();
    } else {
        emit error(tr("the \"Absolute tolerance\" property value could not be retrieved"));

        return;
    }

    if (mProperties.contains(InterpolateSolutionId)) {
        mInterpolateSolution = mProperties.value(InterpolateSolutionId).toBool();
    } else {
        emit error(tr("the \"Interpolate solution\" property value could not be retrieved"));

        return;
    }

    // Initialise our ODE solver

    OdeSolver::initialize(pVoi, pRatesStatesCount, pConstants, pRates, pStates,
                          pAlgebraic, pComputeRates);

    // Create our states vector

    mStatesVector = N_VMake_Serial(pRatesStatesCount, pStates);

    // Create our CVODES solver

    bool newtonIteration = !iterationType.compare(NewtonIteration);

    mSolver = CVodeCreate(!integrationMethod.compare(BdfMethod)?CV_BDF:CV_ADAMS,
                          newtonIteration?CV_NEWTON:CV_FUNCTIONAL);

    // Use our own error handler

    CVodeSetErrHandlerFn(mSolver, errorHandler, this);

    // Initialise our CVODES solver

    CVodeInit(mSolver, rhsFunction, pVoi, mStatesVector);

    // Set our user data

    mUserData = new CvodesSolverUserData(pConstants, pAlgebraic,
                                         pComputeRates);

    CVodeSetUserData(mSolver, mUserData);

    // Set our maximum step

    CVodeSetMaxStep(mSolver, maximumStep);

    // Set our maximum number of steps

    CVodeSetMaxNumSteps(mSolver, maximumNumberOfSteps);

    // Set our linear solver, if needed

    if (newtonIteration) {
        if (!linearSolver.compare(DenseLinearSolver)) {
            mMatrix = SUNDenseMatrix(pRatesStatesCount, pRatesStatesCount);
            mLinearSolver = SUNDenseLinearSolver(mStatesVector, mMatrix);

            CVDlsSetLinearSolver(mSolver, mLinearSolver, mMatrix);
        } else if (!linearSolver.compare(BandedLinearSolver)) {
            mMatrix = SUNBandMatrix(pRatesStatesCount,
                                    upperHalfBandwidth, lowerHalfBandwidth,
                                    upperHalfBandwidth+lowerHalfBandwidth);
            mLinearSolver = SUNBandLinearSolver(mStatesVector, mMatrix);

            CVDlsSetLinearSolver(mSolver, mLinearSolver, mMatrix);
        } else if (!linearSolver.compare(DiagonalLinearSolver)) {
            CVDiag(mSolver);
        } else {
            // We are dealing with a GMRES/Bi-CGStab/TFQMR linear solver

            if (!preconditioner.compare(BandedPreconditioner)) {
                if (!linearSolver.compare(GmresLinearSolver)) {
                    mLinearSolver = SUNSPGMR(mStatesVector, PREC_LEFT, 0);

                    CVSpilsSetLinearSolver(mSolver, mLinearSolver);
                } else if (!linearSolver.compare(BiCgStabLinearSolver)) {
                    mLinearSolver = SUNSPBCGS(mStatesVector, PREC_LEFT, 0);

                    CVSpilsSetLinearSolver(mSolver, mLinearSolver);
                } else {
                    mLinearSolver = SUNSPTFQMR(mStatesVector, PREC_LEFT, 0);

                    CVSpilsSetLinearSolver(mSolver, mLinearSolver);
                }

                CVBandPrecInit(mSolver, pRatesStatesCount, upperHalfBandwidth, lowerHalfBandwidth);
            } else {
                if (!linearSolver.compare(GmresLinearSolver)) {
                    mLinearSolver = SUNSPGMR(mStatesVector, PREC_NONE, 0);

                    CVSpilsSetLinearSolver(mSolver, mLinearSolver);
                } else if (!linearSolver.compare(BiCgStabLinearSolver)) {
                    mLinearSolver = SUNSPBCGS(mStatesVector, PREC_NONE, 0);

                    CVSpilsSetLinearSolver(mSolver, mLinearSolver);
                } else {
                    mLinearSolver = SUNSPTFQMR(mStatesVector, PREC_NONE, 0);

                    CVSpilsSetLinearSolver(mSolver, mLinearSolver);
                }
            }
        }
    }

    // Set our relative and absolute tolerances

    CVodeSStolerances(mSolver, relativeTolerance, absoluteTolerance);
}

//==============================================================================

void CvodesSolver::reinitialize(double pVoi)
{
    // Reinitialise our CVODES object

    CVodeReInit(mSolver, pVoi, mStatesVector);
}

//==============================================================================

void CvodesSolver::solve(double &pVoi, double pVoiEnd) const
{
    // Solve the model

    if (!mInterpolateSolution)
        CVodeSetStopTime(mSolver, pVoiEnd);

    CVode(mSolver, pVoiEnd, mStatesVector, &pVoi, CV_NORMAL);

    // Compute the rates one more time to get up to date values for the rates
    // Note: another way of doing this would be to copy the contents of the
    //       calculated rates in rhsFunction, but that's bound to be more time
    //       consuming since a call to CVode() is likely to generate at least a
    //       few calls to rhsFunction(), so that would be quite a few memory
    //       transfers while here we 'only' compute the rates one more time...

    mComputeRates(pVoiEnd, mConstants, mRates,
                  N_VGetArrayPointer_Serial(mStatesVector), mAlgebraic);
}

//==============================================================================

}   // namespace CVODESSolver
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
