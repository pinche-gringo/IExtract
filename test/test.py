import os.path
import subprocess

import unittest


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
        return self.execute(os.path.join("src", "IExtract"), *args)

    def test(self):
        output = self.run_extract("test")[0].strip().splitlines()

        expected = ("abiword.abw - Title Author Description", "releasenotes.pdf -      ",
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
                    "exif.jpg -     EXIF-Image (comment)")

        # self.assertEqual(len(output), len(expected), output)
        for check, expect in zip(output, expected):
            check = check.rsplit(" ", 2)[0]
            self.assertEqual(check, expect)

