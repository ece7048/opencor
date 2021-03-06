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
// CellML file RDF triple
//==============================================================================

#pragma once

//==============================================================================

#include "cellmlfilerdftripleelement.h"
#include "cellmlsupportglobal.h"

//==============================================================================

#include <QStringList>

//==============================================================================

#include "cellmlapibegin.h"
    #include "cellml-api-cxx-support.hpp"
#include "cellmlapiend.h"

//==============================================================================

namespace OpenCOR {
namespace CellMLSupport {

//==============================================================================

static const auto ResourceRegEx = QStringLiteral("[0-9a-z]+((-|\\.)[0-9a-z]+)?");
static const auto IdRegEx       = QStringLiteral("[0-9A-Za-z\\.%-_:]+(:[0-9A-Za-z\\.%-_:]+)?");

//==============================================================================

class CellmlFile;

//==============================================================================

class CELLMLSUPPORT_EXPORT CellmlFileRdfTriple
{
public:
    enum Type {
        Unknown,
        BioModelsDotNetQualifier,
        Empty
    };

    enum ModelQualifier {
        ModelUnknown,
        ModelIs,
        ModelIsDerivedFrom,
        ModelIsDescribedBy,
        ModelIsInstanceOf,
        ModelHasInstance,
        FirstModelQualifier = ModelIs,
        LastModelQualifier = ModelHasInstance
    };

    enum BioQualifier {
        BioUnknown,
        BioEncodes,
        BioHasPart,
        BioHasProperty,
        BioHasVersion,
        BioIs,
        BioIsDescribedBy,
        BioIsEncodedBy,
        BioIsHomologTo,
        BioIsPartOf,
        BioIsPropertyOf,
        BioIsVersionOf,
        BioOccursIn,
        BioHasTaxon,
        FirstBioQualifier = BioEncodes,
        LastBioQualifier = BioHasTaxon
    };

    explicit CellmlFileRdfTriple(CellmlFile *pCellmlFile,
                                 iface::rdf_api::Triple *pRdfTriple);
    explicit CellmlFileRdfTriple(CellmlFile *pCellmlFile,
                                 const QString pSubject,
                                 const ModelQualifier &pModelQualifier,
                                 const QString &pResource, const QString &pId);
    explicit CellmlFileRdfTriple(CellmlFile *pCellmlFile,
                                 const QString pSubject,
                                 const BioQualifier &pBioQualifier,
                                 const QString &pResource, const QString &pId);
    ~CellmlFileRdfTriple();

    iface::rdf_api::Triple * rdfTriple() const;
    void setRdfTriple(iface::rdf_api::Triple *pRdfTriple);

    CellmlFileRdfTripleElement * subject() const;
    CellmlFileRdfTripleElement * predicate() const;
    CellmlFileRdfTripleElement * object() const;

    Type type() const;

    QString metadataId() const;

    QString qualifierAsString() const;

    ModelQualifier modelQualifier() const;
    QString modelQualifierAsString() const;
    static QString modelQualifierAsString(const ModelQualifier &pModelQualifier);

    BioQualifier bioQualifier() const;
    QString bioQualifierAsString() const;
    static QString bioQualifierAsString(const BioQualifier &pBioQualifier);

    static QStringList qualifiersAsStringList();

    QString resource() const;
    QString id() const;

    static bool decodeTerm(const QString &pTerm, QString &pResource, QString &pId);

private:
    CellmlFile *mCellmlFile;

    ObjRef<iface::rdf_api::Triple> mRdfTriple;

    CellmlFileRdfTripleElement *mSubject;
    CellmlFileRdfTripleElement *mPredicate;
    CellmlFileRdfTripleElement *mObject;

    Type mType;

    ModelQualifier mModelQualifier;
    BioQualifier mBioQualifier;

    QString mResource;
    QString mId;

    explicit CellmlFileRdfTriple(CellmlFile *pCellmlFile,
                                 iface::rdf_api::Triple *pRdfTriple,
                                 const Type &pType,
                                 const ModelQualifier &pModelQualifier,
                                 const BioQualifier &pBioQualifier,
                                 const QString &pResource, const QString &pId);
};

//==============================================================================

class CELLMLSUPPORT_EXPORT CellmlFileRdfTriples: public QList<CellmlFileRdfTriple *>
{
public:
    explicit CellmlFileRdfTriples(CellmlFile *pCellmlFile);

    CellmlFileRdfTriple::Type type() const;

    CellmlFileRdfTriples associatedWith(iface::cellml_api::CellMLElement *pElement) const;

    CellmlFileRdfTriple * add(CellmlFileRdfTriple *pRdfTriple);

    bool remove(CellmlFileRdfTriple *pRdfTriple);
    bool remove(iface::cellml_api::CellMLElement *pElement);
    bool removeAll();

    void updateOriginalRdfTriples();

private:
    CellmlFile *mCellmlFile;

    QStringList mOriginalRdfTriples;

    void recursiveAssociatedWith(CellmlFileRdfTriples &pRdfTriples,
                                 CellmlFileRdfTriple *pRdfTriple) const;

    bool removeRdfTriples(const CellmlFileRdfTriples &pRdfTriples);

    QStringList asStringList() const;

    void updateCellmlFileModifiedStatus();
};

//==============================================================================

}   // namespace CellMLSupport
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
