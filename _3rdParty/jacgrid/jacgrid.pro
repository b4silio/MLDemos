include(../libs.pri)

TEMPLATE = lib
CONFIG -= qt moc

# Input
HEADERS += jacgrid.h atom.h cell_table.h surface.h grid.h plane.h
SOURCES += grid.cpp atom.cpp isosurface.cpp surface.cpp plane.cpp
