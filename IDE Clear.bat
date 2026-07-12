@echo off 
echo IDE Clear
echo 李锦上 lijinshang@126.com
echo 支持Keil和IAR

::Keil清理
del /s *.crf
del /s *.d
del /s *.o
del /s *.i
del /s *.__i
del /s *._ii
del /s *._ia

del /s *.map
del /s *.lnp
del /s *.htm
del /s *.build_log.htm
del /s *.bak
del /s *.uvgui.*
del /s *.uvguix.*
del /s *.uvmpw.uvgui.*
del /s *.uvmpw.uvguix.*


::IAR清理
del /s *.o
del /s *.pbi
del /s *.pbi.xcl


del /s *.out
del /s *.map
del /s *.pbd
del /s *.pbd.browse
del /s *.pbd.linf
del /s *.pbw


exit
