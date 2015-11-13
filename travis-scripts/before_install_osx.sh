#!/bin/bash
# Setup script for OS X.

# Echo the command and exit when a command returns non-zero
set -ev

# Update brew
brew update > /dev/null
brew config > /dev/null

# Copy our custom formulas
sed -i -e "s|{TRAVIS_PULL_REQUEST}|${TRAVIS_PULL_REQUEST}|g" $TRAVIS_BUILD_DIR/travis-scripts/fontforge.rb
cp $TRAVIS_BUILD_DIR/travis-scripts/{fontforge,pango,libuninameslist}.rb /usr/local/Library/Formula/

