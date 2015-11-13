#!/bin/bash
# Install script (dependencies) for OS X.

# Echo the command and exit when a command returns non-zero
set -ev

# "Installing FontForge dependencies..."
brew install Caskroom/cask/xquartz
brew install libspiro --HEAD
brew install libuninameslist --HEAD
brew install --only-dependencies fontforge --with-x11 --with-collab

# Get the FreeType source for the debugger
wget http://download.savannah.gnu.org/releases/freetype/freetype-2.6.1.tar.bz2
tar -xf freetype-2.6.1.tar.bz2

# "Installing packaging utility..."
git clone https://github.com/auriamg/macdylibbundler
pushd macdylibbundler && make && sudo make install && popd
