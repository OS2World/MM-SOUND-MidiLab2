/*  This EXEC, executed from the 755CD, establishes an environment for      */
/*  running the 760CD's C compiler via a peer hookup.                       */
/*  The local LIBPATH on the 755CD must contain entries for the toolkit and */
/*  compiler DLLs on the 760CD                                              */

Address CMD

'SET PATH=K:\TOOLKIT\BIN\PMEISTER;K:\TOOLKIT\BIN\OSARED;K:\TOOLKIT\BIN;K:\IBMCPP\BIN;E:\OS2;E:\OS2\SYSTEM;E:\OS2\INSTALL;E:\;E:\MMOS2;D:\OS2UTIL;'
'SET DPATH=K:\TOOLKIT\BIN\OSARED;K:\TOOLKIT\BOOK;K:\TOOLKIT\MSG;K:\IBMCPP\HELP;K:\IBMCPP;K:\IBMCPP\LOCALE;E:\OS2;E:\OS2\SYSTEM;E:\OS2\INSTALL;E:\;'
'SET LOCPATH=K:\IBMCPP\LOCALE;C:\TCPIP\LOCALE;E:\IBMI18N\LOCALE;E:\LANGUAGE\LOCALE'
'SET VACPP_SHARED=FALSE'
'SET CPPHELP_INI=E:\OS2\SYSTEM'
'SET CPPLOCAL=K:\IBMCPP'
'SET CPPMAIN=K:\IBMCPP'
'SET CPPWORK=K:\IBMCPP'
'SET INCLUDE=K:\TOOLKIT\INC;K:\TOOLKIT\H;K:\IBMCPP\INCLUDE;K:\IBMCPP\INCLUDE\SOM;K:\TOOLKIT\H\LIBC;'
'SET CLREF=CPPCLRF.INF+CPPDAT.INF+CPPAPP.INF+CPPWIN.INF+CPPCTL.INF+CPPADV.INF+CPP2DG.INF+CPPDDE.INF+CPPDM.INF+CPPMM.INF+CPPCLRB.INF'
'SET LIB=K:\TOOLKIT\SAMPLES\MM\LIB;K:\TOOLKIT\LIB;K:\TOOLKIT\SOM\LIB;K:\IBMCPP\LIB'
'SET HELPNDX=EPMKWHLP.NDX+CPP.NDX+CPPBRS.NDX'
