## Process this file with automake to produce Makefile.in

SUBDIRS = m4  intl src doc po

# Add files for windows; don't make own Makefile[.am], as a
# Windows-compiler might need its own Makefile
dist-hook:
	mkdir $(distdir)/Windows
	cp -p $(srcdir)/Windows/*.ds? $(srcdir)/Windows/*.mak \
	      $(srcdir)/Windows/*.h $(distdir)/Windows
	cp -p  $(srcdir)/Make.bat $(distdir)/

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config.rpath
