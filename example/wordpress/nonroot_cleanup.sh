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
echo ':: Building temporary image (nonroot_helper)'
cd nonroot_helper
    docker build -t nonroot_helper . 2>/dev/null >/dev/null
cd ..
echo ':: Running temporary container'
docker run --rm -it -v ./mariadb:/var/lib/mysql nonroot_helper

# Delete the temporary container's image
echo ':: Removing temporary image'
docker image rm nonroot_helper 2>/dev/null >/dev/null

# Re-create the initial database directory
echo ':: Re-creating the initial database directory'
rm -rf mariadb
mkdir mariadb
touch mariadb/.gitkeep
