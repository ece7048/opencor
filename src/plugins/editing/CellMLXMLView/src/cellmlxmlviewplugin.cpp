#include "cellmlxmlviewplugin.h"

#include <QTabBar>

namespace OpenCOR {
namespace CellMLXMLView {

PLUGININFO_FUNC CellMLXMLViewPluginInfo()
{
    QMap<QString, QString> descriptions;

    descriptions.insert("en", "A plugin to edit <a href=\"http://www.cellml.org/\">CellML</a> files using an XML editor");
    descriptions.insert("fr", "Une extension pour �diter des fichiers <a href=\"http://www.cellml.org/\">CellML</a> � l'aide d'un �diteur XML");

    return PluginInfo(PluginInterface::V001, PluginInfo::Gui,
                      PluginInfo::Editing, true,
                      QStringList() << "CoreCellMLEditing" << "QScintilla",
                      descriptions);
}

Q_EXPORT_PLUGIN2(CellMLXMLView, CellMLXMLViewPlugin)

CellMLXMLViewPlugin::CellMLXMLViewPlugin() :
    GuiInterface("CellMLXMLView")
{
    // Set our settings

    mSettings->addView(GuiViewSettings::Editing);
}

void CellMLXMLViewPlugin::retranslateUi()
{
    // Retranslate our view's tab

    GuiViewSettings *view = mSettings->views().at(0);

    view->tabBar()->setTabText(view->tabIndex(), tr("Raw"));
}

} }
