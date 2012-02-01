TARGET      =   Encapsulation
TEMPLATE    =   lib

DEFINES    +=   ENCAPSULATION_LIBRARY

SOURCES    +=   encapsulationplugin.cpp \
                encapsulationsettingspage.cpp

HEADERS    +=   encapsulationplugin.h\
                encapsulation_global.h\
                encapsulationconstants.h \
                encapsulationsettingspage.h

FORMS      +=   encapsulationsettingswidget.ui

OTHER_FILES =   Encapsulation.pluginspec

isEmpty(QTCREATOR_SOURCES):error(Set QTCREATOR_SOURCES variable!)
isEmpty(IDE_BUILD_TREE):error(Set IDE_BUILD_TREE varaible!)

PROVIDER = arturo182

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)
include($$QTCREATOR_SOURCES/src/plugins/coreplugin/coreplugin.pri)
include($$QTCREATOR_SOURCES/src/plugins/cpptools/cpptools.pri)

LIBS += -L$$QTCREATOR_SOURCES/lib/qtcreator/plugins/Nokia
