## Process this file with automake to produce Makefile.in

SUBDIRS = intl src doc po

dist-hook:
	# Add files for windows; don't make own Makefile[.am], as a
	# Windows-compiler might need its own Makefile
	mkdir $(distdir)/Windows
	cp -p $(srcdir)/Windows/*.ds? $(srcdir)/Windows/*.mak $(distdir)/Windows
	cp -p  $(srcdir)/Make.bat $(distdir)/
