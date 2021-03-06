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
// KINSOL solver plugin
//==============================================================================

#include "kinsolsolver.h"
#include "kinsolsolverplugin.h"

//==============================================================================

namespace OpenCOR {
namespace KINSOLSolver {

//==============================================================================

PLUGININFO_FUNC KINSOLSolverPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("a plugin that uses <a href=\"http://computation.llnl.gov/projects/sundials/kinsol\">KINSOL</a> to solve non-linear systems."));
    descriptions.insert("fr", QString::fromUtf8("une extension qui utilise <a href=\"http://computation.llnl.gov/projects/sundials/kinsol\">KINSOL</a> pour résoudre des systèmes non-linéaires."));

    return new PluginInfo(PluginInfo::Solver, true, false,
                          QStringList() << "SUNDIALS",
                          descriptions);
}

//==============================================================================
// I18n interface
//==============================================================================

void KINSOLSolverPlugin::retranslateUi()
{
    // We don't handle this interface...
    // Note: even though we don't handle this interface, we still want to
    //       support it since some other aspects of our plugin are
    //       multilingual...
}

//==============================================================================
// Solver interface
//==============================================================================

Solver::Solver * KINSOLSolverPlugin::solverInstance() const
{
    // Create and return an instance of the solver

    return new KinsolSolver();
}

//==============================================================================

QString KINSOLSolverPlugin::id(const QString &pKisaoId) const
{
    // Return the id for the given KiSAO id

    if (!pKisaoId.compare("KISAO:0000282"))
        return solverName();
    else if (!pKisaoId.compare("KISAO:0000486"))
        return MaximumNumberOfIterationsId;
    else if (!pKisaoId.compare("KISAO:0000477"))
        return LinearSolverId;
    else if (!pKisaoId.compare("KISAO:0000479"))
        return UpperHalfBandwidthId;
    else if (!pKisaoId.compare("KISAO:0000480"))
        return LowerHalfBandwidthId;

    return QString();
}

//==============================================================================

QString KINSOLSolverPlugin::kisaoId(const QString &pId) const
{
    // Return the KiSAO id for the given id

    if (!pId.compare(solverName()))
        return "KISAO:0000282";
    else if (!pId.compare(MaximumNumberOfIterationsId))
        return "KISAO:0000486";
    else if (!pId.compare(LinearSolverId))
        return "KISAO:0000477";
    else if (!pId.compare(UpperHalfBandwidthId))
        return "KISAO:0000479";
    else if (!pId.compare(LowerHalfBandwidthId))
        return "KISAO:0000480";

    return QString();
}

//==============================================================================

Solver::Type KINSOLSolverPlugin::solverType() const
{
    // Return the type of the solver

    return Solver::Nla;
}

//==============================================================================

QString KINSOLSolverPlugin::solverName() const
{
    // Return the name of the solver

    return "KINSOL";
}

//==============================================================================

Solver::Properties KINSOLSolverPlugin::solverProperties() const
{
    // Return the properties supported by the solver

    Descriptions MaximumNumberOfIterationsDescriptions;
    Descriptions LinearSolverDescriptions;
    Descriptions UpperHalfBandwidthDescriptions;
    Descriptions LowerHalfBandwidthDescriptions;

    MaximumNumberOfIterationsDescriptions.insert("en", QString::fromUtf8("Maximum number of iterations"));
    MaximumNumberOfIterationsDescriptions.insert("fr", QString::fromUtf8("Nombre maximum d'itérations"));

    LinearSolverDescriptions.insert("en", QString::fromUtf8("Linear solver"));
    LinearSolverDescriptions.insert("fr", QString::fromUtf8("Solveur linéaire"));

    UpperHalfBandwidthDescriptions.insert("en", QString::fromUtf8("Upper half-bandwidth"));
    UpperHalfBandwidthDescriptions.insert("fr", QString::fromUtf8("Demi largeur de bande supérieure"));

    LowerHalfBandwidthDescriptions.insert("en", QString::fromUtf8("Lower half-bandwidth"));
    LowerHalfBandwidthDescriptions.insert("fr", QString::fromUtf8("Demi largeur de bande inférieure"));

    QStringList LinearSolverListValues = QStringList() << DenseLinearSolver
                                                       << BandedLinearSolver
                                                       << GmresLinearSolver
                                                       << BiCgStabLinearSolver
                                                       << TfqmrLinearSolver;

    return Solver::Properties() << Solver::Property(Solver::Property::IntegerGt0, MaximumNumberOfIterationsId, MaximumNumberOfIterationsDescriptions, QStringList(), MaximumNumberOfIterationsDefaultValue, false)
                                << Solver::Property(Solver::Property::List, LinearSolverId, LinearSolverDescriptions, LinearSolverListValues, LinearSolverDefaultValue, false)
                                << Solver::Property(Solver::Property::IntegerGe0, UpperHalfBandwidthId, UpperHalfBandwidthDescriptions, QStringList(), UpperHalfBandwidthDefaultValue, false)
                                << Solver::Property(Solver::Property::IntegerGe0, LowerHalfBandwidthId, LowerHalfBandwidthDescriptions, QStringList(), LowerHalfBandwidthDefaultValue, false);
}

//==============================================================================

QMap<QString, bool> KINSOLSolverPlugin::solverPropertiesVisibility(const QMap<QString, QString> &pSolverPropertiesValues) const
{
    // Return the visibility of our properties based on the given properties
    // values

    QMap<QString, bool> res = QMap<QString, bool>();

    QString linearSolver = pSolverPropertiesValues.value(LinearSolverId);

    if (!linearSolver.compare(BandedLinearSolver)) {
        // Banded linear solver

        res.insert(UpperHalfBandwidthId, true);
        res.insert(LowerHalfBandwidthId, true);
    } else {
        // Dense/GMRES/Bi-CGStab/TFQMR linear solver

        res.insert(UpperHalfBandwidthId, false);
        res.insert(LowerHalfBandwidthId, false);
    }

    return res;
}

//==============================================================================

}   // namespace KINSOLSolver
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
