REM === UUPC ===
SET UUPCSYSRC=local.rc
SET UUPCUSRRC=local.rc
SET LOGNAME=%1
::SET HOME=d:\user\%1
SET HOME=.
SET TERM=ansi
SET TERMCAP=termcap.dat
::SET MAIL=d:\user\mail\%1.mai
SET MAIL=.\%1.mai
REM ============
REM nntp support needs a TMP environment variable (so does Warp TCPIP!)
REM and a NNTPSERVER environment variable!
rem SET TMP=E:\TEMP
set NNTPSERVER=newshost.pcug.org.au

