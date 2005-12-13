## Process this file with automake to produce Makefile.in

bin_PROGRAMS=IExtract

DEFS = @DEFS@ -Wall

IExtract_LDADD=@YMP_LIBS@ @LTLIBINTL@
IExtract_SOURCES=Utility.h ParseHTML.cpp ParseHTML.h ParseMSOffice.cpp	 \
	         ParseMSOffice.h Writer.cpp Writer.h ParseJPG.cpp	 \
		 ParseJPG.h Properties.h ParseSOffice.cpp ParseSOffice.h \
		 Options.h ParsePDF.h ParsePDF.cpp ParseMP3.cpp		 \
		 ParseMP3.h ParseOOffice.cpp ParseOOffice.h ParseRTF.cpp \
		 ParseRTF.h ParseOGG.cpp ParseOGG.h ParseAbiword.cpp	 \
		 ParseAbiword.h ParsePNG.cpp ParsePNG.h ParseGIF.cpp	 \
		 ParseGIF.h IExtract.cpp

EXTRA_DIST=Options.meta

Options.meta: Options.h
	mgeni.pl Options.h>Options.meta
