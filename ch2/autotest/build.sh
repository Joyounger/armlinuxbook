#!/bin/bash
#apt-cyg install automake
autoscan
#https://stackoverflow.com/questions/41053012/autoscan-unescaped-left-brace-in-regex-is-deprecated-passed-through-in-regex-m
#Unescaped left brace in regex is deprecated here (and will be fatal in Perl 5.30)
#Unescaped left brace in regex is deprecated here (and will be fatal in Perl 5.30)
aclocal --verbose
autoconf --verbose
autoheader --verbose
