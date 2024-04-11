#!/usr/bin/env python3

# oom.py: Tests an infinite loop with a large output to see if the server catches large CGI outputs and avoids OOM

large = '0123456789abcdef' * 1024 * 1024 * 4
while True:
    print(large)
