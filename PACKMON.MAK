#
# Borland C++ IDE generated makefile
#
.AUTODEPEND


#
# Borland C++ tools
#
IMPLIB  = Implib
BCC     = Bcc +BccW16.cfg 
TLINK   = TLink
TLIB    = TLib
BRC     = Brc
TASM    = Tasm
#
# IDE macros
#


#
# Options
#
IDE_LFLAGS =  -L..\..\..\..\LIB
IDE_RFLAGS = 
LLATW16_packmondexe =  -LC:\BC45\LIB -Twe -C -c
RLATW16_packmondexe =  -31
BLATW16_packmondexe = 
CNIEAT_packmondexe = -IC:\BC45\INCLUDE -D_RTLDLL;_BIDSDLL;_OWLDLL;_OWLALLPCH;__DEBUG=2;__TRACE=1;__WARN=1;
LNIEAT_packmondexe = -x
LEAT_packmondexe = $(LLATW16_packmondexe)
REAT_packmondexe = $(RLATW16_packmondexe)
BEAT_packmondexe = $(BLATW16_packmondexe)
CLATW16_packmondcpp = 
LLATW16_packmondcpp =  -LC:\BC45\LIB
RLATW16_packmondcpp = 
BLATW16_packmondcpp = 
CEAT_packmondcpp = $(CEAT_packmondexe) $(CLATW16_packmondcpp)
CNIEAT_packmondcpp = -IC:\BC45\INCLUDE -D_RTLDLL;_BIDSDLL;_OWLDLL;_OWLALLPCH;__DEBUG=2;__TRACE=1;__WARN=1;
LNIEAT_packmondcpp = -x
LEAT_packmondcpp = $(LEAT_packmondexe) $(LLATW16_packmondcpp)
REAT_packmondcpp = $(REAT_packmondexe) $(RLATW16_packmondcpp)
BEAT_packmondcpp = $(BEAT_packmondexe) $(BLATW16_packmondcpp)
CLATW16_ddeclidrc = 
LLATW16_ddeclidrc =  -LC:\BC45\LIB
RLATW16_ddeclidrc = 
BLATW16_ddeclidrc = 
CEAT_ddeclidrc = $(CEAT_packmondexe) $(CLATW16_ddeclidrc)
CNIEAT_ddeclidrc = -IC:\BC45\INCLUDE -D_RTLDLL;_BIDSDLL;_OWLDLL;_OWLALLPCH;__DEBUG=2;__TRACE=1;__WARN=1;
LNIEAT_ddeclidrc = -x
LEAT_ddeclidrc = $(LEAT_packmondexe) $(LLATW16_ddeclidrc)
REAT_ddeclidrc = $(REAT_packmondexe) $(RLATW16_ddeclidrc)
BEAT_ddeclidrc = $(BEAT_packmondexe) $(BLATW16_ddeclidrc)
CLATW16_pmddedcpp = 
LLATW16_pmddedcpp =  -LC:\BC45\LIB
RLATW16_pmddedcpp = 
BLATW16_pmddedcpp = 
CEAT_pmddedcpp = $(CEAT_packmondexe) $(CLATW16_pmddedcpp)
CNIEAT_pmddedcpp = -IC:\BC45\INCLUDE -D_RTLDLL;_BIDSDLL;_OWLDLL;_OWLALLPCH;__DEBUG=2;__TRACE=1;__WARN=1;
LNIEAT_pmddedcpp = -x
LEAT_pmddedcpp = $(LEAT_packmondexe) $(LLATW16_pmddedcpp)
REAT_pmddedcpp = $(REAT_packmondexe) $(RLATW16_pmddedcpp)
BEAT_pmddedcpp = $(BEAT_packmondexe) $(BLATW16_pmddedcpp)

#
# Dependency List
#
Dep_packmon = \
   packmon.exe

packmon : BccW16.cfg $(Dep_packmon)
  echo MakeNode 

Dep_packmondexe = \
   c:\bc45\lib\default.def\
   packmon.obj\
   ddecli.res\
   pmdde.obj

packmon.exe : $(Dep_packmondexe)
  $(TLINK)   @&&|
 /v $(IDE_LFLAGS) $(LEAT_packmondexe) $(LNIEAT_packmondexe) +
C:\BC45\LIB\c0wl.obj+
packmon.obj+
pmdde.obj
$<,$*
owldwi.lib+
bidsdi.lib+
C:\BC45\LIB\import.lib+
C:\BC45\LIB\crtldll.lib
c:\bc45\lib\default.def
|
   $(BRC) ddecli.res $<

packmon.obj :  packmon.cpp
  $(BCC)   -c @&&|
 $(CEAT_packmondcpp) $(CNIEAT_packmondcpp) -o$@ packmon.cpp
|

ddecli.res :  ddecli.rc
  $(BRC) $(IDE_RFLAGS) $(REAT_ddeclidrc) $(CNIEAT_ddeclidrc) -R -FO$@ ddecli.rc

inputdia.res :  ..\..\..\bc45\include\owl\inputdia.rc
  $(BRC) $(IDE_RFLAGS) $(REAT_ddeclidrc) $(CNIEAT_ddeclidrc) -R -FO$@ ..\..\..\bc45\include\owl\inputdia.rc

pmdde.obj :  pmdde.cpp
  $(BCC)   -c @&&|
 $(CEAT_pmddedcpp) $(CNIEAT_pmddedcpp) -o$@ pmdde.cpp
|

# Compiler configuration file
BccW16.cfg : 
   Copy &&|
-R
-v
-vi
-H
-H=..\..\..\..\LIB\OWLWI.CSM
-H"OWL\OWLPCH.H"
-ml
-WS
-H"owl\owlpch.h"
| $@


