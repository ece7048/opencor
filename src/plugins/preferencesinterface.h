/*******************************************************************************

Copyright The University of Auckland

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*******************************************************************************/

//==============================================================================
// Preferences interface
//==============================================================================

#pragma once

//==============================================================================

#include <QWidget>

//==============================================================================

class QSettings;

//==============================================================================

namespace OpenCOR {
namespace Preferences {

//==============================================================================

class PreferencesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreferencesWidget(QObject *pPluginInstance, QWidget *pParent);
    ~PreferencesWidget();

    virtual void resetPreferences() = 0;
    virtual void savePreferences() = 0;

protected:
    QSettings *mSettings;
};

//==============================================================================

}   // namespace Preferences

//==============================================================================

class PreferencesInterface
{
public:
#define INTERFACE_DEFINITION
    #include "preferencesinterface.inl"
#undef INTERFACE_DEFINITION
};

//==============================================================================

}   // namespace OpenCOR

//==============================================================================

Q_DECLARE_INTERFACE(OpenCOR::PreferencesInterface, "OpenCOR::PreferencesInterface")

//==============================================================================
// End of file
//==============================================================================