/* 
  REXX-Script to configure & build xmgr 4.1.x (and above) for OS/2
  (980509)
*/

/*
   Todo:
       - Ask/check for Xbae, libhelp, g77, ...
       - Error handling ?
       - distinguish between Debug- & Production builds
       - Add OS/2 support to configure script and withdraw this script ;-)
*/

trace n

Parse Arg param

True  = 1
False = 0

options = ''
param2 = param
do while param <> ''
  Parse Var param tp param
  tp = Translate(tp)
  if (Left(tp,1) = '-') & (Length(tp) = 2) then
    select
      when tp = 'D' then
        do
        say 'Option: -d'
        option = option'D'
        end
      when tp = 'O' then
        do
        say 'Option: -o'
        option = option'O'
        end
      when (tp = '?') then
        do
        call ShowHelp
        SIGNAL FIN
        end
      otherwise
        do
        call ShowHelp
        SIGNAL FIN
        end
    end /* select */
end /* do while */

cf_make_conf = 'Make.conf'
cf_config_h  = 'config.h'

curdir = Directory()
x11root = Value('X11ROOT', , 'OS2ENVIRONMENT')
if x11root <> '' then
  do
  x11path=x11root'\XFree86'
  newdir = Directory(x11path)
  call Directory(curdir)
  if Translate(newdir) <> Translate(x11path) then
    do
    say 'XFree/2 is not properly installed!'
    say 'At least the X11ROOT environment variable is not set correctly'
    end
  end
else
  do
  say 'XFree/2 is not properly installed!'
  say 'At least the X11ROOT environment variable is missing'
  SIGNAL FIN
  end

systemdir = Strip(curdir, 'T', '\')'\arch\os2'

/* Install Make.conf */
if FileExists(cf_make_conf) = True then
  do
  say cf_make_conf' is already installed!'
  call CharOut , 'Install default file instead ? (y/n) '
  Parse Upper Pull answer
  if answer = 'Y' then
    do
    call FileCopy systemdir'\'cf_make_conf'.os2', curdir'\'cf_make_conf
    call Execute 'touch 'curdir'\'cf_make_conf
    end
  end
else 
  do 
  call FileCopy systemdir'\'cf_make_conf'.os2', curdir'\'cf_make_conf
  call Execute 'touch 'curdir'\'cf_make_conf
  end

/* Install config.h */
if FileExists(cf_config_h) = True then
  do
  say cf_config_h' is already installed!'
  call CharOut , 'Install default file instead ? (y/n) '
  Parse Upper Pull answer
  if answer = 'Y' then
    do
    call FileCopy systemdir'\'cf_config_h'.os2', curdir'\'cf_config_h
    call Execute 'touch 'curdir'\'cf_config_h
    end
  end
else 
  do 
  call FileCopy systemdir'\'cf_config_h'.os2', curdir'\'cf_config_h
  call Execute 'touch 'curdir'\'cf_config_h
  end

/*
Building dlopen.a
*/
'cd .\arch\os2'
'x11make.exe -f dlfcn.mak all'
'cd ..\..'


/* Calling x11make.exe cause make.cmd won't work here */
say 'Start compiling ...'
call Execute 'x11make'

call FileCopy systemdir'\dotest.cmd', curdir'\examples\dotest.cmd'

say 'configos2 has finished!'

FIN:
exit

/* ######################################################################## */

Execute:
Parse Arg cmd
Address CMD '@'cmd
return

FileCopy:
Parse Arg par1, par2
call Execute '@copy 'par1' 'par2' >nul'
return

FileExists:
Parse Arg fe_file
rs = Stream(fe_file, 'C', 'QUERY EXISTS')
if rs = '' then
  return False
else
  return True

ShowHelp:
say 'Valid options for configos2.cmd:'
/*
say ' -d : create binaries for debugging'
say ' -o : optimize'
*/
return

rtest:
_CR     = D2C(13)
_LF     = D2C(10)
_CRLF   = _CR''_LF
rc = LineIn (rfile,1,0)
rlength = Chars(rfile)
rcontent = CharIn(rfile, rlength)
call LineOut rfile
rcount = 0
do while rcontent <> ''
  Parse Var rcontent rline (_CRLF) rcontent
  rline = Strip(rline, 'T')
  if rline <> '' then
    do
    rcount = rcount+1
    rnew.rcount = rline
    say rcount'. 'rline
    end
end /* do while */
rnew.0 = rcount
return
