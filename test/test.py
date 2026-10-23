import os.path
import subprocess

import unittest


class ExecuteError(Exception):
    pass


class TestExtract(unittest.TestCase):
    @staticmethod
    def execute(*args):
        proc = subprocess.Popen(list(args), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        info, error = proc.communicate()
        if proc.returncode:
            execerror = "Error {error} executing {prog}".format(error=proc.returncode, prog=u" ".join(args))
            raise ExecuteError("{prog}\n{info}\n{error}".format(prog=execerror, info=info, error=error))
        return info.decode("utf-8"), error.decode("utf-8")

    def run_extract(self, *args):
        return self.execute(os.environ.get("IEXTRACT", os.path.join("src", "IExtract")), *args)

    EXPECTED = ("abiword.abw - Title Author Description", "releasenotes.pdf -   OpenJade Project  ",
                "Excel2007.xlsx - Titel Markus Schwab Kommentar",
                "artsaudiomanager.png - Made with Sodipodi Unknown abcdefghijklmnopqr",
                "PowerPoint.pptx - Titel Markus Schwab Kommentar",
                "StarWrite.sdw - Title Markus Schwab Comment",
                "test.gif -     Comment",
                "VM-PKE-Remote Control-QS3 PrSpez-QS4 PrProz-V1-12.pdf - Microsoft Word - VM-PKE-Remote Control-QS3 PrSpez-QS4 PrProz-V1-12.doc 03kuhene  ",
                "Zeitaufzeichnung.xls - Zeiterfassung 2000 Markus Schwab  ",
                "1-Buggy.jpg - Title\0 Author\0 Comments\0",
                "MSOffice2007.docx - Titel Markus Schwab Kommentar",
                "TestGermanWord90.doc - Titel Rainer Klute (Autor) This is a document for testing HPSF",
                "jfif.jpg -     JFIF-Image (comment)",
                "Gewerbeaufnahme.doc - Markus Schwab Markus Schwab  ",
                "MSOfficeXP.doc - Titel\0\0 Markus Schwab\0\0 Kommentar\0\0",
                "rtf.rtf - RTF-Test title Markus Schwab Document for testing RTF-Parser (of IExtract)",
                "exif.jpg -     EXIF-Image (comment)",
                # Info dictionary (title) and compressed XMP metadata of the catalog (author, subject)
                "xmp.pdf - PDF info title PDF XMP author PDF XMP description",
                # JPEG with XMP only (language alternative, several creators, entities)
                "xmp.jpg - XMP & JPEG title First author, Second author XMP <description>",
                # zTXt (title), compressed iTXt (author) and XMP (description)
                "itxt.png - PNG compressed title PNG international \u00c4 author PNG XMP description",
                # ImageDescription and Artist; title from XMP
                "tiff.tif - TIFF title TIFF artist TIFF description",
                # Big endian; Windows XP tags override ImageDescription
                "tiff-be.tiff - BE XP title\0 BE artist BE XP comment\0",
                # EXIF (description, artist) and XMP (title)
                "webp.webp - WebP title WebP artist WebP description",
                # Title and description from the elements; author from the RDF metadata
                "svg.svg - SVG title SVG author SVG description & more",
                "epub.epub - EPUB title EPUB author <p>EPUB description</p>",
                "vorbis.ogg - Vorbis title Vorbis artist Vorbis album",
                "opus.opus - Opus title Opus artist Opus album",
                "flac.flac - FLAC title FLAC artist FLAC album",
                # iTunes-style metadata (moov behind mdat)
                "m4a.m4a - M4A title M4A artist M4A album",
                # QuickTime-style user data
                "quicktime.mov - QuickTime title QuickTime artist QuickTime comment")

    def check_output(self, *args):
        output = self.run_extract(*args)[0].strip().splitlines()

        # Directory order depends on the filesystem, so compare independent of order
        output = sorted(line.rsplit(" ", 2)[0] for line in output)
        self.assertEqual(output, sorted(self.EXPECTED))

    def test(self):
        self.check_output("test")

    def test_content(self):
        """Determines the type of the files by their content"""
        self.check_output("-M", "Content", "test")
