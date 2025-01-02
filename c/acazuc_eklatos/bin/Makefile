include $(MAKEDIR)/env.mk

DIRS = sh \
       cat \
       init \
       ls \
       touch \
       sleep \
       colors \
       tests \
       env \
       mkdir \
       chmod \
       chown \
       date \
       stat \
       mkfifo \
       readlink \
       ln \
       echo \
       mknod \
       mv \
       cp \
       unlink \
       rm \
       chgrp \
       wc \
       true \
       false \
       [ \
       which \
       pwd \
       uname \
       head \
       sort \
       test \
       realpath \
       strings \
       mount \
       tail \
       truncate \
       statfs \
       su \
       whoami \
       clear \
       strace \
       kmadd \
       kmdel \
       yes \
       tests.sh \
       ipcs \
       beep \
       ipcmk \
       ipcrm \
       gunzip \
       nm \
       ping \
       traceroute \
       getent \
       dig \
       dhclient \
       more \
       ifconfig \
       nmap \
       nice \
       pngview \
       top \
       hexdump \
       uptime \
       named \
       nc \
       fetch \
       kill \
       gzip \
       jdb \
       tee \
       ldd \
       time \
       egrep \
       fgrep \
       rgrep \
       diff \
       mktemp \
       kmls \
       reboot \
       halt \
       suspend \
       hibernate \
       basename \
       dirname \
       jpeg \
       tr \
       virgl \
       lspci \

ifneq ($(WITH_BINUTILS), yes)
DIRS += readelf \
        objdump
endif

ifneq ($(WITH_GREP), yes)
DIRS+= grep
endif

ifeq ($(WITH_JKX), yes)
DIRS+= fractal \
       h261
endif

include $(MAKEDIR)/dir.mk
