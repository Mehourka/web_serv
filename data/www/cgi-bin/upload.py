#!/usr/bin/env python3

# Upload File Script

import os
import sys
import cgi, cgitb

cgitb.enable()

def response_headers():
	print("Content-Type: text/html")
	print()

def upload_file(field_item, upload_dir= os.getenv('UPLOAD_DIR')):
	file_name = os.path.basename(field_item.filename)
	file_path = os.path.join(upload_dir, file_name)
	os.makedirs(upload_dir, exist_ok=True)
	count = 0
	with open(file_path, 'wb+') as file:
		while True:
			count += 1
			chunk = field_item.file.read(65536)
			if not chunk:
				break
			file.write(chunk)
	print(f"</br>- {count} chunk(s) Wrote to {file_path}</br>")
	return file_path

def main():

	# Populate form cgi data
	form = cgi.FieldStorage()
	header = os.getenv('ROOT') + '/template/header.html'
	with open(header, 'r') as f:
		print(f.read())
	f.close()
	if not form:
		print("<h1>Upload: No files found!</h1>")
		exit(0)

	footer = os.getenv('ROOT') + '/template/footer.html'
	print("<h1>Upload successful!</h1>")
	print('<div class="content_small">')
	# Create folder if it does not exist
	for key in form.keys():
		field_item = form[key]
		if field_item.filename:
			upload_file(field_item)
		else:
			print(f"NO FILE FOUND!")
		print("</br>")
	print('</div>')
	with open(footer, 'r') as f:
		print(f.read())
	f.close()
	exit (0)

if __name__ == "__main__":
	main()
