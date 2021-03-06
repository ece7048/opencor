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
// Second-order Runge-Kutta solver
//==============================================================================

#pragma once

//==============================================================================

#include "solverinterface.h"

//==============================================================================

namespace OpenCOR {
namespace SecondOrderRungeKuttaSolver {

//==============================================================================

static const auto StepId = QStringLiteral("Step");

//==============================================================================

static const double StepDefaultValue = 1.0;

//==============================================================================

class SecondOrderRungeKuttaSolver : public OpenCOR::Solver::OdeSolver
{
    Q_OBJECT

public:
    explicit SecondOrderRungeKuttaSolver();
    ~SecondOrderRungeKuttaSolver() override;

    void initialize(double pVoi, int pRatesStatesCount, double *pConstants,
                    double *pRates, double *pStates, double *pAlgebraic,
                    ComputeRatesFunction pComputeRates) override;

    void solve(double &pVoi, double pVoiEnd) const override;

private:
    double mStep;

    double *mYk1;
};

//==============================================================================

}   // namespace SecondOrderRungeKuttaSolver
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
