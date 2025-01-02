#!/bin/sh

echo "Running aclocal..."
aclocal
echo "Running autoconf..."
autoreconf --install
echo "Running automake..."
automake --add-missing
