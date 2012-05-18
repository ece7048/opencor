//==============================================================================
// CellML annotation view metadata details widget
//==============================================================================

#ifndef CELLMLANNOTATIONVIEWMETADATADETAILSWIDGET_H
#define CELLMLANNOTATIONVIEWMETADATADETAILSWIDGET_H

//==============================================================================

#include "cellmlannotationviewmetadataviewdetailswidget.h"
#include "commonwidget.h"

//==============================================================================

namespace Ui {
    class CellmlAnnotationViewMetadataDetailsWidget;
}

//==============================================================================

#include <QSplitter>

//==============================================================================

namespace OpenCOR {
namespace CellMLAnnotationView {

//==============================================================================

class CellmlAnnotationViewDetailsWidget;

//==============================================================================

class CellmlAnnotationViewMetadataDetailsWidget : public QSplitter,
                                                  public Core::CommonWidget
{
    Q_OBJECT

public:
    explicit CellmlAnnotationViewMetadataDetailsWidget(CellmlAnnotationViewDetailsWidget *pParent);
    ~CellmlAnnotationViewMetadataDetailsWidget();

    virtual void retranslateUi();

    CellmlAnnotationViewDetailsWidget * parent() const;

    void updateGui(const CellMLSupport::CellmlFileRdfTriples &pRdfTriples);
    void finalizeGui();

private:
    CellmlAnnotationViewDetailsWidget *mParent;

    Ui::CellmlAnnotationViewMetadataDetailsWidget *mGui;

    CellmlAnnotationViewMetadataViewDetailsWidget *mMetadataViewDetails;
};

//==============================================================================

}   // namespace CellMLAnnotationView
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
