qtopia_project(qtopia app) 
TARGET=mixer
CONFIG+=qtopia_main
CONFIG+=no_singleexec
CONFIG+=no_quicklaunch
CONFIG+=no_tr

FORMS=mixerbase.ui
HEADERS=mixer.h
SOURCES=main.cpp mixer.cpp

target.hint=sxe
target.domain=trusted
