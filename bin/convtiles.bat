@echo off

SET WLAPATH=%SAMBINPATH%wla\
SET BMP2TILEPATH=%SAMBINPATH%bmp2tile\

del "*(tiles).pscompr"
del "*(tile numbers).bin"
del "*(palette).bin"

for %%f in (*.png) do %BMP2TILEPATH%bmp2tile %%f -palsms -savetiles %%~dpnf.tiles.pscompr -savetilemap %%~dpnf.tilemap.bin -savepalette %%~dpnf.palette.bin -exit

rem Renames the files to their final names (Yeah, REN alone won't cut it when there's composite file extensions...)
for /f "delims=." %%f in ('dir /b *.tiles.pscompr') do ren "%%f.tiles.pscompr" "%%f (tiles).pscompr"
for /f "delims=." %%f in ('dir /b *.tilemap.bin') do ren "%%f.tilemap.bin" "%%f (tile numbers).bin"
for /f "delims=." %%f in ('dir /b *.palette.bin') do ren "%%f.palette.bin" "%%f (palette).bin"
