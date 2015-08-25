INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/nilibddc.h\
        $$PWD/QTDM.h\
        $$PWD/TdmFileViewer.h \
        $$PWD/tdmfiletablemodel.h \
    nilib/QTDMFile.h \
    nilib/QTDMGroup.h \
    nilib/QTDMChannel.h

SOURCES +=  $$PWD/QTDM.cpp\
        $$PWD/TdmFileViewer.cpp \ 
    nilib/QTDMFile.cpp \
    nilib/QTDMGroup.cpp \
    nilib/QTDMChannel.cpp

LIBS += -L$$PWD -lnilibddc
