project(ZincWindowPlugin)

# Add the plugin

add_plugin(ZincWindow
    SOURCES
        ../../i18ninterface.cpp
        ../../plugininfo.cpp
        ../../plugininterface.cpp
        ../../windowinterface.cpp

        src/zincwindowplugin.cpp
        src/zincwindowwindow.cpp
    HEADERS_MOC
        src/zincwindowplugin.h
        src/zincwindowwindow.h
    UIS
        src/zincwindowwindow.ui
    PLUGINS
        ZincWidget
)
