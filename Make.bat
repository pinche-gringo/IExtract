Rem @Echo Off
If "%1" == "MSC" Goto MSC
If "%1" == "GCC" Goto GCC
Goto Default

:GCC
Shift
:Default
cd src
Call Make -f ..\Windows\GCCExtract.mak %1 %2 %3 %4 %5 %6 %7 %8 %9
Goto End

:MSC
Shift
cd Windows
Call NMake /f VCExtract.mak %1 %2 %3 %4 %5 %6 %7 %8 %9
Goto End

:End
cd ..
