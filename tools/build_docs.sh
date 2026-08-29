#!/bin/bash
set -euo pipefail

mkdir -p build/docs

if command -v dot >/dev/null 2>&1; then
        doxygen Doxyfile
else
        echo "Note: graphviz (the 'dot' command) isn't installed -- generating docs" >&2
        echo "      without call/caller graphs. Install graphviz for those (e.g." >&2
        echo "      'apt install graphviz' / 'brew install graphviz')." >&2
        (cat Doxyfile; echo "HAVE_DOT = NO") | doxygen -
fi