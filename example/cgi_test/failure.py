#!/usr/bin/env python3

# failure.py: Tests a script that exits with a non-zero status code to see if the server catches this and returns a 502 error

import sys
sys.exit(1)
