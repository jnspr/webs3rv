#!/usr/bin/env python3

import cgi, os

def html_escape(text: str) -> str:
    return text.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")

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
        print("key-field: ", field)
        value = form.getvalue(field)
        print("<tr>")
        print("<th>{}</th>".format(field))
        print("<td>{}</td>".format(value))
        print("</tr>")

    print("</table>")

    print('<ul>')
    for key, value in os.environ.items():
        print(f"<li>{html_escape(key)}: {html_escape(value)}</li>")
    print('</ul>')

    print("</body>")
    print("</html>")

if __name__ == "__main__":
    main()
