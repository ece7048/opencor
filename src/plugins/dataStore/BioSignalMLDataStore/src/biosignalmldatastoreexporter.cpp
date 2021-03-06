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
// BioSignalML data store exporter
//==============================================================================

#include "biosignalmldatastoredata.h"
#include "biosignalmldatastoreexporter.h"

//==============================================================================

#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QUrl>

//==============================================================================

#include "biosignalml/biosignalml.h"
#include "biosignalml/data/hdf5.h"

//==============================================================================

namespace OpenCOR {
namespace BioSignalMLDataStore {

//==============================================================================

BiosignalmlDataStoreExporter::BiosignalmlDataStoreExporter(DataStore::DataStoreData *pDataStoreData) :
    DataStore::DataStoreExporter(pDataStoreData)
{
}

//==============================================================================

void BiosignalmlDataStoreExporter::execute(QString &pErrorMessage) const
{
    // Determine the number of steps to export everything

    BiosignalmlDataStoreData *dataStoreData = static_cast<BiosignalmlDataStoreData *>(mDataStoreData);
    DataStore::DataStore *dataStore = dataStoreData->dataStore();
    int nbOfRuns = dataStore->runsCount();
    int nbOfSteps = 0;

    for (int i = 0; i < nbOfRuns; ++i)
        nbOfSteps += dataStore->size(i);

    double oneOverNbOfSteps = 1.0/nbOfSteps;
    int stepNb = 0;

    // Export the given data store to a BioSignalML file

    bsml::HDF5::Recording *recording = 0;

    try {
        // Create and populate a recording
        // Note: although a VOI may not officially be visible to a user, we
        //       still need to export it since it's needed by BioSignalML...

        DataStore::DataStoreVariable *voi = dataStore->voi();
        std::string recordingUri = QUrl::fromLocalFile(dataStoreData->fileName()).toEncoded().toStdString();
        std::string baseUnits = dataStore->uri().toStdString()+"/units#";

        recording = new bsml::HDF5::Recording(recordingUri, dataStoreData->fileName().toStdString(), true);

        recording->add_prefix(rdf::Namespace("units", baseUnits));

        recording->set_label(dataStoreData->name().toStdString());
        recording->set_investigator(rdf::Literal(dataStoreData->author().toStdString()));
        recording->set_description(dataStoreData->description().toStdString());
        recording->set_comment(dataStoreData->comment().toStdString());

        for (int i = 0; i < nbOfRuns; ++i) {
            // Create and populate a clock

            std::string runNb = (nbOfRuns == 1)?"":"/"+QString::number(i+1).toStdString();
            bsml::HDF5::Clock::Ptr clock = recording->new_clock(recordingUri+"/clock/"+voi->uri().toStdString()+runNb,
                                                                rdf::URI(baseUnits+voi->unit().toStdString()),
                                                                voi->values(i),
                                                                voi->size(i));

            clock->set_label(voi->label().toStdString());

            // Determine what should be exported (minus the VOI, which should
            // always be exported in the case of a BioSignalML file)

            DataStore::DataStoreVariables variables = mDataStoreData->variables();

            variables.removeOne(dataStore->voi());

            // Retrieve some information about the different variables that are
            // to be exported

            std::vector<std::string> uris;
            std::vector<rdf::URI> units;

            foreach (DataStore::DataStoreVariable *variable, variables) {
                uris.push_back(recordingUri+"/signal/"+variable->uri().toStdString()+runNb);
                units.push_back(rdf::URI(baseUnits+variable->unit().toStdString()));
            }

            // Create and populate a signal array

            enum {
                BufferRows = 50000
            };

            bsml::HDF5::SignalArray::Ptr signalArray = recording->new_signalarray(uris, units, clock);
            int j = -1;

            foreach (DataStore::DataStoreVariable *variable, variables)
                (*signalArray)[++j]->set_label(variable->label().toStdString());

            double *data = new double[variables.count()*BufferRows] {};
            double *dataPointer = data;
            int rowCount = 0;

            for (quint64 j = 0, jMax = dataStore->size(i); j < jMax; ++j) {
                foreach (DataStore::DataStoreVariable *variable, variables)
                    *dataPointer++ = variable->value(j, i);

                ++rowCount;

                if (rowCount >= BufferRows) {
                    signalArray->extend(data, variables.count()*BufferRows);

                    dataPointer = data;

                    rowCount = 0;
                }

                emit progress(++stepNb*oneOverNbOfSteps);
            }

            signalArray->extend(data, variables.count()*rowCount);

            delete[] data;
        }
    } catch (bsml::data::Exception &exception) {
        // Something went wrong, so retrieve the error message and delete our
        // BioSignalML file

        pErrorMessage = tr("The data could not be exported to BioSignalML (%1).").arg(exception.what());

        QFile::remove(dataStoreData->fileName());
    }

    // Close and delete our recording, if any

    if (recording) {
        recording->close();

        delete recording;
    }
}

//==============================================================================

}   // namespace BioSignalMLDataStore
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
