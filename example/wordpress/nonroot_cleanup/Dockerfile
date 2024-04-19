FROM alpine
ENTRYPOINT [ "find", "/var/lib/mysql", "-mindepth", "1", "!", "-name", ".gitkeep", "-print", "-delete" ]
