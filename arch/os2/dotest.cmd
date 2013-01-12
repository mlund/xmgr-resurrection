/* dotest.cmd, the REXX-based equivalent to shellscript 'dotest'*/
/* demos for ACE/gr     */
/* define the location  */

curdir = Directory()
curdir   = Strip(curdir, 'T', '\') /* handle worst case: root */
guess = Stream('..\src\xmgr.exe', 'C', 'QUERY EXISTS')
if guess ='' then
  do
  x11root = Value('X11ROOT', , 'OS2ENVIRONMENT')
  guess = Stream(x11root'\xfree86\bin\xmgr.exe', 'C', 'QUERY EXISTS')
  end
if guess <> '' then 
  ACEGR = guess
else
  do
  say 'No binary found'
  exit
  end

guess = Stream(curdir'\..\doc\xmgr.html', 'C', 'QUERY EXISTS')
if guess <> '' then
  GR_HOME = Value('X11ROOT', curdir'\..' , 'OS2ENVIRONMENT')

/* command line parameters */
call ExecuteCmd ACEGR' -usage'
/* call ExecuteCmd sleep 3 */
/* don't ask stupid questions */
ACEGR = ACEGR' -noask'

/* explain the row of single character buttons and a few other things */
ACEGR' explain.par'

/* display the various axes available */
ACEGR' -p axes.par'

/* display the symbols and line styles */
'ACEGR symslines.dat'

/* display more symbols */
'ACEGR moresyms.dat'

/* display various fill styles */
'ACEGR fills.dat'

/* some graph stuff and ticks */
'ACEGR -p graphs.par'

/* some graph stuff and ticks */
'ACEGR props.gr'

/* demonstration of many graphs */
'ACEGR -maxgraph 36 manygraphs.d'

/* some graph stuff and ticks */
'ACEGR brw.dat -p regions.par'

/* test of a graph inset */
'ACEGR tinset.d'

/* some time and date formats */
'ACEGR times.dat'

/* some more tick label formats */
'ACEGR -p tforms.par'

/* some more tick label formats */
'ACEGR au.d'

/* display fonts and font mappings */
'ACEGR -p tfonts.par'

/* example of world stack */
'ACEGR tstack.dat'

/* a graph with a parameter file */
'ACEGR -p test1.par -a xy test.dat'

/* a graph with a parameter file in reverse video */
'ACEGR -rvideo -p test1.par -a xy test.dat'

/* ? */
'ACEGR test2.d'

/* multiple graphs with a parameter file */
'ACEGR mlo.dat -graph 1 brw.dat -p co2.par'

/* multiple graphs created with arrange feature */
/* $ACEGR co2.all0.dat -graph 1 1.dat -graph 2 2.dat -graph 3 3.dat -graph 4 4.dat -p co2-3.par */
'ACEGR co2.all'

/* a graph with alternate axes */
'ACEGR -p altaxis.par test.dat -a xy'

/* a graph with error bars */
'ACEGR terr.d'

/* another graph with error bars */
'ACEGR terr2.d'

/* a graph with XY RADIUS format */
'ACEGR txyr.dat'

/* a graph with hilo data */
'ACEGR hilo.dat'

/* log plots */
'ACEGR -p logtest.par log.d -graph 1 log.d'

/* more log plots */
'ACEGR tlog.demo'

/* non-linear curve fitting */
'ACEGR logistic.d'

/* bar charts */

/* display all types of bar graphs */
'ACEGR bars.d'

/* a bar graph demonstrating specified ticks and tick labels */
'ACEGR bar.d'

/* a bar graph demonstrating specified ticks and tick labels */
'ACEGR bar2.d'

/* a bar graph demonstrating patterns */
'ACEGR tbar3.dat'

/* a stacked bar graph */
'ACEGR stackedb.d'

/* some interesting stuff */
ACEGR' -arrange 2 2 -b test.com'

/* a slideshow demo */
'ACEGR -pipe  < slideshow.d'

/* Test the -pipe option */
'ACEGR -pipe < tpipe.d'

/* need a program */
/* modified from previous versions, a thank you goes to Bruce Barnett */
/* this modification allows others without write permission */
/* to run the demos. */

say
rs = Stream('mc.exe', 'C', 'QUERY EXISTS')
if rs = '' then
  do
  say "Compiling a short program to test the -pipe option"
  say "Executing 'gcc tmc.c -o tmc.exe'"
  '@gcc tmc.c -o tmc.exe'
  say "Done compilation"
  say
  end

/* a graph with the -pipe option */
say "Testing -pipe option, executing './tmc | $ACEGR -pipe' "
'./tmc | 'ACEGR' -pipe'
call ExecuteCmd 'rm -f tmc.exe'

exit /* end if main program */


/* procedures */

ExecuteCmd: PROCEDURE
Parse Arg EC_param
_silent_pre = "@"
ADDRESS CMD _silent_pre""EC_param
return rc
