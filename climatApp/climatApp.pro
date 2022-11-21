TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS = -Wno-c++11-narrowing

LIBS += -lgdal -lboost_system -lboost_filesystem -lboost_program_options -lboost_iostreams -lnetcdf_c++
# -lsqlite3  pour ouvrir la BD de l'OWSF

LIBS += -L$$PWD/usr/include/gdal/ -lgdal
INCLUDEPATH += /usr/include/gdal/
DEPENDPATH += /usr/include/gdal/

# pour date
INCLUDEPATH += $$PWD/../date/include/date/
DEPENDPATH += $$PWD/../date/include/date/
INCLUDEPATH += $$PWD/../date/include/
DEPENDPATH += $$PWD/../date/include/

INCLUDEPATH += $$PWD/../netcdf-cxx-4.2/cxx/
DEPENDPATH += $$PWD/../netcdf-cxx-4.2/cxx/

TARGET = climatApp

SOURCES += \
    ecaddata.cpp \
    irmdata.cpp \
    main.cpp\
    mar.cpp \
    safrandata.cpp \
    utils.cpp

HEADERS+=\
        date.h\
    ecaddata.h \
    irmdata.h \
    mar.h \
    safrandata.h \
    utils.h
