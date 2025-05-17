include $(MAKEDIR)/env.mk

DIRS = [ \
       basename \
       beep \
       cat \
       chgrp \
       chmod \
       chown \
       clear \
       colors \
       cp \
       date \
       dhclient \
       diff \
       dig \
       dirname \
       echo \
       egrep \
       env \
       false \
       fetch \
       fgrep \
       getent \
       gunzip \
       gzip \
       halt \
       head \
       hexdump \
       hibernate \
       ifconfig \
       init \
       ipcmk \
       ipcrm \
       ipcs \
       jdb \
       jpeg \
       kill \
       kmadd \
       kmdel \
       kmls \
       ldd \
       ln \
       ls \
       lspci \
       mkdir \
       mkfifo \
       mknod \
       mktemp \
       more \
       mount \
       mv \
       named \
       nc \
       nice \
       nm \
       nmap \
       ping \
       pngview \
       pwd \
       readlink \
       realpath \
       reboot \
       rgrep \
       rm \
       sh \
       sleep \
       sort \
       stat \
       statfs \
       strace \
       strings \
       su \
       suspend \
       tail \
       tee \
       test \
       tests \
       tests.sh \
       time \
       top \
       touch \
       tr \
       traceroute \
       true \
       uname \
       unlink \
       uptime \
       virgl \
       wc \
       which \
       whoami \
       yes \

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
