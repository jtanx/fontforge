#!/bin/bash
# Build script for OS X.

# Source common parameters
. ./travis-scripts/common.sh
# Echo the command and exit when a command returns non-zero
set -ev

LOGFILE=/tmp/travisci-osx-brewlog.txt

brew install --verbose fontforge --HEAD --with-x11 --with-collab
fontforge -version
python2 -c "import fontforge; print fontforge.__version__"
./travis-scripts/create-osx-app.sh