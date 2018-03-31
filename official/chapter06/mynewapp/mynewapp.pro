qtopia_project(qtopia app) # see buildsystem.html for more project keywords
TARGET=mynewapp
# main.cpp uses the QTOPIA_ADD_APPLICATION/QTOPIA_MAIN macros
CONFIG+=qtopia_main
# Despite using the QTOPIA_MAIN macro, do not build this app as a
# quicklaunch plugin unless -force-quicklaunch was passed to configure
CONFIG+=no_quicklaunch
# Do not build this app into a singleexec binary
CONFIG+=no_singleexec

# Specify the languages that make lupdate should produce .ts files for
AVAILABLE_LANGUAGES=en_US
# Specify the langauges we want to install translations for
LANGUAGES=$$AVAILABLE_LANGUAGES

# These are the source files that get built to create the application
FORMS=mynewappbase.ui
HEADERS=mynewapp.h
SOURCES=main.cpp mynewapp.cpp

# Install the launcher item. The metadata comes from the .desktop file
# and the path here.
desktop.files=mynewapp.desktop
desktop.path=/apps/Applications
desktop.trtarget=mynewapp-nct
desktop.hint=nct desktop
INSTALLS+=desktop

# Install some pictures.
pics.files=pics/*
pics.path=/pics/mynewapp
pics.hint=pics
INSTALLS+=pics

# Install help files
help.source=help
help.files=mynewapp.html
help.hint=help
INSTALLS+=help

# SXE information
target.hint=sxe
target.domain=trusted

# Package information (used for make packages)
pkg.name=mynewapp
pkg.desc=MyNewApp Application
pkg.version=1.0.0-1
pkg.maintainer=Trolltech (www.trolltech.com)
pkg.license=Commercial
