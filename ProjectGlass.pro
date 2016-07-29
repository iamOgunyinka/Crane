APP_NAME = ProjectGlass

CONFIG += qt warn_on cascades10

QT  += network xml
LIBS    += -lbbdata
LIBS    += -lbbsystem
LIBS    += -lbbplatform
LIBS    += -lQtLocationSubset

include(config.pri)
