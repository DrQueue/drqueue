; Convert an NSIS string to a form suitable for use by InstallOptions
; Usage:
;   Push <NSIS-string>
;   Call Nsis2Io
;   Pop <IO-string>
Function Nsis2Io
  Exch $0 ; The source
  Push $1 ; The output
  Push $2 ; Temporary char
  StrCpy $1 "" ; Initialise the output
loop:
  StrCpy $2 $0 1 ; Get the next source char
  StrCmp $2 "" done ; Abort when none left
    StrCpy $0 $0 "" 1 ; Remove it from the source
    StrCmp $2 "\" "" +3 ; Back-slash?
      StrCpy $1 "$1\\"
      Goto loop
    StrCmp $2 "$\r" "" +3 ; Carriage return?
      StrCpy $1 "$1\r"
      Goto loop
    StrCmp $2 "$\n" "" +3 ; Line feed?
      StrCpy $1 "$1\n"
      Goto loop
    StrCmp $2 "$\t" "" +3 ; Tab?
      StrCpy $1 "$1\t"
      Goto loop
    StrCpy $1 "$1$2" ; Anything else
    Goto loop
done:
  StrCpy $0 $1
  Pop $2
  Pop $1
  Exch $0
FunctionEnd

; Convert an InstallOptions string to a form suitable for use by NSIS
; Usage:
;   Push <IO-string>
;   Call Io2Nsis
;   Pop <NSIS-string>
Function Io2Nsis
  Exch $0 ; The source
  Push $1 ; The output
  Push $2 ; Temporary char
  StrCpy $1 "" ; Initialise the output
loop:
  StrCpy $2 $0 1 ; Get the next source char
  StrCmp $2 "" done ; Abort when none left
    StrCpy $0 $0 "" 1 ; Remove it from the source
    StrCmp $2 "\" +3 ; Escape character?
      StrCpy $1 "$1$2" ; If not just output
      Goto loop
    StrCpy $2 $0 1 ; Get the next source char
    StrCpy $0 $0 "" 1 ; Remove it from the source
    StrCmp $2 "\" "" +3 ; Back-slash?
      StrCpy $1 "$1\"
      Goto loop
    StrCmp $2 "r" "" +3 ; Carriage return?
      StrCpy $1 "$1$\r"
      Goto loop
    StrCmp $2 "n" "" +3 ; Line feed?
      StrCpy $1 "$1$\n"
      Goto loop
    StrCmp $2 "t" "" +3 ; Tab?
      StrCpy $1 "$1$\t"
      Goto loop
    StrCpy $1 "$1$2" ; Anything else (should never get here)
    Goto loop
done:
  StrCpy $0 $1
  Pop $2
  Pop $1
  Exch $0
FunctionEnd

