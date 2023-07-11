@echo off
set MATLAB=C:\Program Files\MATLAB\R2023a
"%MATLAB%\bin\win64\gmake" -f AbstractFuelControl_M1_LOKI.mk  OPTS="-DTID01EQ=0"
