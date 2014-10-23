INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/nilibddc.h\
        $$PWD/QTDM.h\
        $$PWD/TdmFileViewer.h \
        $$PWD/tdmfiletablemodel.h

SOURCES +=  $$PWD/QTDM.cpp\
        $$PWD/TdmFileViewer.cpp 

LIBS += -L$$PWD -lnilibddc
