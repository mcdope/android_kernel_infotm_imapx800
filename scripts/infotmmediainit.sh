#!/bin/bash

rm -rf drivers/InfotmMedia
svn co http://192.168.0.6/svn/midware/InfotmMedia drivers/InfotmMedia
cd drivers/InfotmMedia
./builder/kernel/linux_3_0/buildenv.sh all
cd -

cat drivers/Makefile | sed '/InfotmMedia/d' > __mk.tmp
mv __mk.tmp drivers/Makefile
echo "#add for InfotmMedia drviers" >> drivers/Makefile
echo "obj-\$(CONFIG_INFOTM_MEDIA_SUPPORT) += InfotmMedia/" >> drivers/Makefile
echo >> drivers/Makefile

