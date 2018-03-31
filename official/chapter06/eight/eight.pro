
qtopia_project(qtopia app) # see buildsystem.html for more project keywords
TARGET=eight
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
HEADERS=eight.h
SOURCES=main.cpp eight.cpp

# SXE information
target.hint=sxe
target.domain=trusted

