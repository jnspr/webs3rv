#!/bin/bash

# Exit on non-zero status code
set -e

# Change into this script's directory
PROJECT_DIR=$(dirname "${BASH_SOURCE[@]}")
cd "$PROJECT_DIR"

# Bring down the database container
echo ':: Running `docker-compose down`'
docker-compose down 2>/dev/null >/dev/null

# Build and run a temporary container for database deletion
echo ':: Building temporary image (nonroot_cleanup)'
cd nonroot_cleanup
    docker build -t nonroot_cleanup . 2>/dev/null >/dev/null
cd ..

echo ':: Running temporary container, deletion log:'
docker run --rm -it -v ./mariadb:/var/lib/mysql nonroot_cleanup

# Delete the temporary container's image
echo ':: Removing temporary image'
docker image rm nonroot_cleanup 2>/dev/null >/dev/null
