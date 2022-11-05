#!/usr/bin/python

# Import modules for CGI handling 
import cgi, cgitb, os


# Create instance of FieldStorage 
form = cgi.FieldStorage() 



# Get data from fields
first_name = form.getvalue('first_name')
last_name  = form.getvalue('last_name')

print ("Content-type:text/html\r\n\r\n")
print ("<html>")
print ("<head>")
print ("<title>Hello - Second CGI Program</title>")
print ("</head>")
print ("<body>")
print ("<h2>Hello %s %s</h2>" % (first_name, last_name))
print(form)
print("="*100);
print("PATH_INFO: %s" % os.getenv("PATH_INFO"));
print("<a href='/'>Go Back to Root</a>")
print ("</body>")
print ("</html>")
