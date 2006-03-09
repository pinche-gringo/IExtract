## Process this file with automake to produce Makefile.in

SRC_ABIWORD=ParseAbiword.cpp ParseAbiword.h
if SUPPORT_ABIWORD
   USE_ABIWORD=$(SRC_ABIWORD)
endif

SRC_GIF=ParseGIF.cpp ParseGIF.h
if SUPPORT_GIF
   USE_GIF=$(SRC_GIF)
endif
SRC_JPEG=ParseJPG.cpp ParseJPG.h
if SUPPORT_JPEG
   USE_JPEG=$(SRC_JPEG)
endif

SRC_HTML=ParseHTML.cpp ParseHTML.h
if SUPPORT_HTML
   USE_HTML=$(SRC_HTML)
endif

SRC_MP3=ParseMP3.cpp ParseMP3.h
if SUPPORT_MP3
   USE_MP3=$(SRC_MP3)
endif

SRC_MSOFFICE=ParseMSOffice.cpp ParseMSOffice.h
if SUPPORT_MSOFFICE
   USE_MSOFFICE=$(SRC_MSOFFICE)
endif

SRC_OGG=ParseOGG.cpp ParseOGG.h
if SUPPORT_OGG
   USE_OGG=$(SRC_OGG)
endif

SRC_OO=ParseSOffice.cpp ParseSOffice.h ParseOOffice.cpp ParseOOffice.h
if SUPPORT_OO
   USE_OO=$(SRC_OO)
endif

SRC_PDF=ParsePDF.cpp ParsePDF.h
if SUPPORT_PDF
   USE_PDF=$(SRC_PDF)
endif

SRC_PNG=ParsePNG.cpp ParsePNG.h
if SUPPORT_PNG
   USE_PNG=$(SRC_PNG)
endif

SRC_RTF=ParseRTF.cpp ParseRTF.h
if SUPPORT_RTF
   USE_RTF=$(SRC_RTF)
endif

bin_PROGRAMS=IExtract

DEFS=@DEFS@ -Wall

IExtract_LDADD=@YMP_LIBS@ @LTLIBINTL@
IExtract_SOURCES=Utility.h Writer.cpp Writer.h Properties.h Options.h	\
$(USE_HTML) $(USE_GIF) $(USE_JPEG) $(USE_OO) $(USE_PDF) $(USE_PNG)	\
$(USE_MP3) $(USE_OGG) $(USE_RTF) $(USE_ABIWORD) $(USE_MSOFFICE)		\
IExtract.cpp

EXTRA_DIST=Options.meta $(SRC_HTML) $(SRC_MSOFFICE) $(SRC_JPEG)		\
$(SRC_OO) $(SRC_PDF) $(SRC_MP3) $(SRC_OGG) $(SRC_RTF) $(SRC_ABIWORD)	\
$(SRC_PNG) $(SRC_GIF)

Options.meta: Options.h
	mgeni.pl Options.h>Options.meta
