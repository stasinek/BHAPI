@echo off
rmdir /s/q windows\objchk_wxp_x86
rmdir /s/q windows\objchk_w2k_x86
rmdir /s/q linux\fs\objchk_wxp_x86
rmdir /s/q linux\fs\objchk_w2k_x86
rmdir /s/q linux\mm\objchk_wxp_x86
rmdir /s/q linux\mm\objchk_w2k_x86
rmdir /s/q linux\lib\objchk_wxp_x86
rmdir /s/q linux\lib\objchk_w2k_x86
rmdir /s/q fs\minix\objchk_wxp_x86
rmdir /s/q fs\minix\objchk_w2k_x86
rmdir /s/q fs\vfat\objchk_wxp_x86
rmdir /s/q fs\vfat\objchk_w2k_x86
rmdir /s/q fs\ext2\objchk_wxp_x86
rmdir /s/q fs\ext2\objchk_w2k_x86
del /q buildchk_wxp_x86.log
del /q buildchk_w2k_x86.log
del /q buildchk_wxp_x86.err
del /q buildchk_w2k_x86.err

rmdir /s/q windows\objfre_wxp_x86
rmdir /s/q windows\objfre_w2k_x86
rmdir /s/q linux\fs\objfre_wxp_x86
rmdir /s/q linux\fs\objfre_w2k_x86
rmdir /s/q linux\mm\objfre_wxp_x86
rmdir /s/q linux\mm\objfre_w2k_x86
rmdir /s/q linux\lib\objfre_wxp_x86
rmdir /s/q linux\lib\objfre_w2k_x86
rmdir /s/q fs\minix\objfre_wxp_x86
rmdir /s/q fs\minix\objfre_w2k_x86
rmdir /s/q fs\vfat\objfre_wxp_x86
rmdir /s/q fs\vfat\objfre_w2k_x86
rmdir /s/q fs\ext2\objfre_wxp_x86
rmdir /s/q fs\ext2\objfre_w2k_x86
del /q buildfre_wxp_x86.log
del /q buildfre_w2k_x86.log
del /q buildfre_wxp_x86.err
del /q buildfre_w2k_x86.err

rmdir /s/q i386
rmdir /s/q Debug Release
rmdir /s/q tools\test\Debug tools\test\Release
del /q vfs.ncb
