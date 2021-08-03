TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lgdal -lsqlite3 -lboost_system -lboost_filesystem -lboost_program_options -lnetcdf_c++ -lcurlpp

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
    irmdata.cpp \
        main.cpp\



HEADERS+=\
        date.h\ \
    irmdata.h








