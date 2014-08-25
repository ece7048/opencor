/*******************************************************************************

Licensed to the OpenCOR team under one or more contributor license agreements.
See the NOTICE.txt file distributed with this work for additional information
regarding copyright ownership. The OpenCOR team licenses this file to you under
the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

*******************************************************************************/

//==============================================================================
// CSV data store class
//==============================================================================

#include "coredatastore.h"
#include "coredatastorevariable.h"
#include "csvdatastore.h"

//==============================================================================

#include <QFile>
#include <QTextStream>

//==============================================================================

namespace OpenCOR {
namespace CSVDataStore {

//==============================================================================

bool exportDataStore(const CoreDataStore::CoreDataStore *pDataStore,
                     const QString &pFileName)
{
    // Export the given data store to a CSV file with the given file name

    QFile file(pFileName);

    if (!file.open(QIODevice::WriteOnly)) {
        // The file can't be opened, so...

        file.remove();

        return false;
    }

    // Write out the contents of the data store to the file

    QTextStream out(&file);

    // Header

    static const QString Header = "%1 (%2)";

    CoreDataStore::CoreDataStoreVariable *voi = pDataStore->voi();
    CoreDataStore::CoreDataStoreVariables variables = pDataStore->variables();

    out << Header.arg(voi->getUri().replace("/prime", "'").replace("/", " | "),
                      voi->getUnits());

    auto variableBegin = variables.begin();
    auto variableEnd = variables.end();

    for (auto variable = variableBegin; variable != variableEnd; ++variable)
        out << "," << Header.arg((*variable)->getUri().replace("/prime", "'").replace("/", " | "),
                                 (*variable)->getUnits());

    out << "\n";

    // Data itself

   for (qulonglong i = 0;  i < pDataStore->size(); ++i) {
       out << voi->getPoint(i);

       for (auto variable = variableBegin; variable != variableEnd; ++variable)
           out << "," << (*variable)->getPoint(i);

       out << "\n";
   }

   // We are done...

   file.close();

   return true;
}

//==============================================================================

}   // namespace CSVDataStore
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================