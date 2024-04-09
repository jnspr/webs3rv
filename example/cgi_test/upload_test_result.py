#!/usr/bin/env python3

import cgi, base64

def main() -> None:
    print("Content-Type: text/html\r\n\r\n", end='')
    print("<html>")
    print("<head>")
    print("<title>webserv - CGI form test result</title>")
    print("</head>")
    print("<body>")
    print("<h1>CGI form test result</h1>")

    form = cgi.FieldStorage()
    form.getvalue('file')
    value = form.getvalue('file')
    data_url = (b'data:image/jpg;base64,' + base64.b64encode(value)).decode('utf-8')
    print(f'<img src="{data_url}">')

    print("</body>")
    print("</html>")

if __name__ == "__main__":
    main()
