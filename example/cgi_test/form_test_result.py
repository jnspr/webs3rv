#!/usr/bin/env python3

import cgi

def main() -> None:
    print("Content-Type: text/html\r\n\r\n", end='')
    print("<html>")
    print("<head>")
    print("<title>webserv - CGI form test result</title>")
    print("</head>")
    print("<body>")
    print("<h1>CGI form test result</h1>")
    print("<table>")

    form = cgi.FieldStorage()
    for field in form.keys():
        value = form.getvalue(field)
        print("<tr>")
        print("<th>{}</th>".format(field))
        print("<td>{}</td>".format(value))
        print("</tr>")

    print("</table>")
    print("</body>")
    print("</html>")

if __name__ == "__main__":
    main()
