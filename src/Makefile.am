## Process this file with automake to produce Makefile.in

bin_PROGRAMS=IExtract

IExtract_LDADD=@YMP_LIBS@
IExtract_SOURCES=IExtract.cpp ParseHTML.cpp ParseHTML.h ParseWord.cpp \
	         ParseWord.h Writer.cpp Writer.h ParseJPG.cpp ParseJPG.h\
		 Properties.h ParseSOffice.cpp ParseSOffice.h Options.h\
		 ParsePDF.h ParsePDF.cpp ParseMP3.cpp ParseMP3.h \
		 ParseOOfice.cpp ParseOOfice.h

EXTRA_DIST=Options.meta

Options.meta: Options.h
	mgeni Options.h Options.meta
