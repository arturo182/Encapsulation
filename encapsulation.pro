
TARGET      =   Encapsulation
TEMPLATE    =   lib

DEFINES    +=   ENCAPSULATION_LIBRARY

PROVIDER = arturo182


SOURCES    +=   encapsulationplugin.cpp \
                encapsulationsettingspage.cpp

HEADERS    +=   encapsulationplugin.h\
                encapsulation_global.h\
                encapsulationconstants.h \
                encapsulationsettingspage.h

FORMS      +=   encapsulationsettingswidget.ui

OTHER_FILES =   Encapsulation.pluginspec

# Qt Creator linking

isEmpty(QTCREATOR_SOURCES):error(Set QTCREATOR_SOURCES variable!)
isEmpty(IDE_BUILD_TREE):error(Set IDE_BUILD_TREE varaible!)

## uncomment to build plugin into user config directory
## <localappdata>/plugins/<ideversion>
##    where <localappdata> is e.g.
##    "%LOCALAPPDATA%\Nokia\qtcreator" on Windows Vista and later
##    "$XDG_DATA_HOME/Nokia/qtcreator" or "~/.local/share/Nokia/qtcreator" on Linux
##    "~/Library/Application Support/Nokia/Qt Creator" on Mac
#USE_USER_DESTDIR = yes

LIBS += -L$$IDE_PLUGIN_PATH/QtProject

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)


