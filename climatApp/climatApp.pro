TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS = -Wno-c++11-narrowing

LIBS += -lgdal -lboost_system -lboost_filesystem -lboost_program_options -lnetcdf_c++ -lboost_iostreams
# -lsqlite3  pour ouvrir la BD de l'OWSF

LIBS += -L$$PWD/usr/include/gdal/ -lgdal
INCLUDEPATH += $$PWD/../../../../../../../..//usr/include/gdal/
DEPENDPATH += $$PWD/../../../../../../../../usr/include/gdal/

# pour boost
INCLUDEPATH += $$PWD/../../../../../../../../usr/lib/x86_64-linux-gnu
DEPENDPATH += $$PWD/../../../../../../../../usr/lib/x86_64-linux-gnu

# pour date
INCLUDEPATH += $$PWD/../date/include/date/
DEPENDPATH += $$PWD/../date/include/date/
INCLUDEPATH += $$PWD/../date/include/
DEPENDPATH += $$PWD/../date/include/

TARGET = climatApp

SOURCES += \
    ecaddata.cpp \
    irmdata.cpp \
    main.cpp\
    safrandata.cpp \
    utils.cpp

HEADERS+=\
        date.h\
    ecaddata.h \
    irmdata.h \
    safrandata.h \
    utils.h








