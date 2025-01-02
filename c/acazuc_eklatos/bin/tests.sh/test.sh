#!/bin/sh

rm -f /tmp/a || echo KO_rm
touch /tmp/a || echo KO_touch

echo "[regular file]"

[ -b /tmp/a ] && echo "KO -b" || echo "OK -b"
[ -c /tmp/a ] && echo "KO -c" || echo "OK -c"
[ -d /tmp/a ] && echo "KO -d" || echo "OK -d"
[ -e /tmp/a ] && echo "OK -e" || echo "KO -e"
[ -f /tmp/a ] && echo "OK -f" || echo "KO -f"
[ -g /tmp/a ] && echo "KO -g" || echo "OK -g"
[ -h /tmp/a ] && echo "KO -h" || echo "OK -h"
[ -k /tmp/a ] && echo "KO -k" || echo "OK -k"
[ -p /tmp/a ] && echo "KO -p" || echo "OK -p"
[ -r /tmp/a ] && echo "OK -r" || echo "KO -r"
[ -s /tmp/a ] && echo "KO -s" || echo "OK -s"
[ -u /tmp/a ] && echo "KO -u" || echo "OK -u"
[ -w /tmp/a ] && echo "OK -w" || echo "KO -w"
[ -x /tmp/a ] && echo "KO -x" || echo "OK -x"
[ -L /tmp/a ] && echo "KO -L" || echo "OK -L"
[ -O /tmp/a ] && echo "OK -O" || echo "KO -O"
[ -G /tmp/a ] && echo "OK -G" || echo "KO -G"
[ -S /tmp/a ] && echo "KO -S" || echo "OK -S"

echo
echo "[non-empty file]"
echo "test" > /tmp/a

[ -s /tmp/a ] && echo "OK -s" || echo "KO -s"

echo
echo "[non-existing file]"
rm -f /tmp/a || echo KO_rm

[ -e /tmp/a ] && echo "KO -e" || echo "OK -e"
[ -f /tmp/a ] && echo "KO -f" || echo "OK -f"
[ -r /tmp/a ] && echo "KO -r" || echo "OK -r"
[ -w /tmp/a ] && echo "KO -w" || echo "OK -w"
[ -O /tmp/a ] && echo "KO -O" || echo "OK -O"
[ -G /tmp/a ] && echo "KO -G" || echo "OK -G"

echo
echo "[fifo]"
mkfifo /tmp/a || echo KO_mkfifo

[ -e /tmp/a ] && echo "OK -e" || echo "KO -e"
[ -f /tmp/a ] && echo "KO -f" || echo "OK -f"
[ -p /tmp/a ] && echo "OK -p" || echo "KO -p"

rm -f /tmp/a || echo KO_rm

echo
echo "[block device]"
mknod /tmp/a b 1 1 || echo KO_mknod_b

[ -e /tmp/a ] && echo "OK -e" || echo "KO -e"
[ -b /tmp/a ] && echo "OK -b" || echo "KO -b"
[ -f /tmp/a ] && echo "KO -f" || echo "OK -f"

rm -f /tmp/a || echo KO_rm

echo
echo "[char device]"
mknod /tmp/a c 1 1 || echo KO_mknod_c

[ -e /tmp/a ] && echo "OK -e" || echo "KO -e"
[ -c /tmp/a ] && echo "OK -c" || echo "KO -c"
[ -f /tmp/a ] && echo "KO -f" || echo "OK -f"

rm -f /tmp/a || echo KO_rm

echo
echo "[directory]"
mkdir /tmp/a || echo KO_mkdir

[ -e /tmp/a ] && echo "OK -e" || echo "KO -e"
[ -d /tmp/a ] && echo "OK -d" || echo "KO -d"
[ -f /tmp/a ] && echo "KO -f" || echo "OK -f"

rm -d /tmp/a || echo KO_rm

echo
echo "[dangling symbolic link]"
ln -s /tmp/b /tmp/a || echo KO_ln

rm -f /tmp/b || echo KO_rm

[ -e /tmp/a ] && echo "KO -e" || echo "OK -e"
[ -L /tmp/a ] && echo "OK -L" || echo "KO -L"
[ -f /tmp/a ] && echo "KO -f" || echo "OK -f"
[ -r /tmp/a ] && echo "KO -r" || echo "OK -r"

echo
echo "[symbolic link]"
touch /tmp/b || echo KO_touch

[ -e /tmp/a ] && echo "OK -e" || echo "KO -e"
[ -L /tmp/a ] && echo "OK -L" || echo "KO -L"
[ -f /tmp/a ] && echo "OK -f" || echo "KO -f"
[ -r /tmp/a ] && echo "OK -r" || echo "KO -r"

rm -f /tmp/b || echo KO_rm
rm -f /tmp/a || echo KO_rm

echo
echo "[regular file]"
touch /tmp/a || echo KO_touch

[ -e /tmp/a ] && echo "OK -e" || echo "KO -e"
[ -f /tmp/a ] && echo "OK -f" || echo "KO -f"

echo
echo "[chmod 0000]"
chmod 000 /tmp/a || echo KO_chmod

[ -r /tmp/a ] && echo KO || echo OK
[ -w /tmp/a ] && echo KO || echo OK
[ -x /tmp/a ] && echo KO || echo OK

echo
echo "[chmod 0100]"
chmod 100 /tmp/a || echo KO_chmod

[ -r /tmp/a ] && echo KO || echo OK
[ -w /tmp/a ] && echo KO || echo OK
[ -x /tmp/a ] && echo OK || echo KO

echo
echo "[chmod 200]"
chmod 200 /tmp/a || echo KO_chmod

[ -r /tmp/a ] && echo KO || echo OK
[ -w /tmp/a ] && echo OK || echo KO
[ -x /tmp/a ] && echo KO || echo OK

echo
echo "[chmod 400]"
chmod 400 /tmp/a || echo KO_chmod

[ -r /tmp/a ] && echo OK || echo KO
[ -w /tmp/a ] && echo KO || echo OK
[ -x /tmp/a ] && echo KO || echo OK

echo
echo "[chmod 1000]"
chmod 1000 /tmp/a || echo KO_chmod

[ -u /tmp/a ] && echo KO || echo OK
[ -g /tmp/a ] && echo KO || echo OK
[ -k /tmp/a ] && echo OK || echo KO

echo
echo "[chmod 2000]"
chmod 2000 /tmp/a || echo KO_chmod

[ -u /tmp/a ] && echo KO || echo OK
[ -g /tmp/a ] && echo OK || echo KO
[ -k /tmp/a ] && echo KO || echo OK

echo
echo "[chmod 4000]"
chmod 4000 /tmp/a || echo KO_chmod

[ -u /tmp/a ] && echo OK || echo KO
[ -g /tmp/a ] && echo KO || echo OK
[ -k /tmp/a ] && echo KO || echo OK

rm -f /tmp/a || echo KO_rm

echo
echo "[strings]"
[ -z "" ] && echo OK || echo KO
[ -z a ] && echo KO || echo OK

[ -n "" ] && echo KO || echo OK
[ -n 1 ] && echo OK || echo KO

echo
echo "[strcmp]"
[ "test" = "test" ] && echo OK || echo KO
[ "" = "" ] && echo OK || echo KO
[ "test" = "tset" ] && echo KO || echo OK

[ "test" != "test" ] && echo KO || echo OK
[ "" != "" ] && echo KO || echo OK
[ "test" != "tset" ] && echo OK || echo KO

echo
echo "[numeric]"
[ 5 -lt 6 ] && echo OK || echo KO
[ 5 -lt 5 ] && echo KO || echo OK
[ 5 -lt 4 ] && echo KO || echo OK

[ 5 -le 6 ] && echo OK || echo KO
[ 5 -le 5 ] && echo OK || echo KO
[ 5 -le 4 ] && echo KO || echo OK

[ 5 -eq 6 ] && echo KO || echo OK
[ 5 -eq 5 ] && echo OK || echo KO
[ 5 -eq 4 ] && echo KO || echo OK

[ 5 -ne 6 ] && echo OK || echo KO
[ 5 -ne 5 ] && echo KO || echo OK
[ 5 -ne 4 ] && echo OK || echo KO

[ 5 -ge 6 ] && echo KO || echo OK
[ 5 -ge 5 ] && echo OK || echo KO
[ 5 -ge 4 ] && echo OK || echo KO

[ 5 -gt 6 ] && echo KO || echo OK
[ 5 -gt 5 ] && echo KO || echo OK
[ 5 -gt 4 ] && echo OK || echo KO

echo
echo "[invalid numbers]"
[ a -eq 6 ] 2>&- && echo KO || echo OK
[ 5 -eq a ] 2>&-  && echo KO || echo OK
[ "" -lt 0 ] 2>&- && echo KO || echo OK

echo
echo "[parenthesis]"
[ \( 5 -eq 5 \) ] && echo OK || echo KO
[ \( \( 5 -eq 5 \) \) ] && echo OK || echo KO

echo
echo "[logical and]"
[ \( 5 -eq 5 \) -a \( 6 -eq 6 \) ] && echo OK || echo KO
[ \( 5 -eq 6 \) -a \( 6 -eq 6 \) ] && echo KO || echo OK
[ \( 5 -eq 5 \) -a \( 6 -eq 5 \) ] && echo KO || echo OK
[ \( 5 -eq 6 \) -a \( 6 -eq 5 \) ] && echo KO || echo OK

echo
echo "[logical or]"
[ \( 5 -eq 5 \) -o \( 6 -eq 6 \) ] && echo OK || echo KO
[ \( 5 -eq 6 \) -o \( 6 -eq 6 \) ] && echo OK || echo KO
[ \( 5 -eq 5 \) -o \( 6 -eq 5 \) ] && echo OK || echo KO
[ \( 5 -eq 6 \) -o \( 6 -eq 5 \) ] && echo KO || echo OK

echo
echo "[not]"
[ ! 5 -eq 5 ] && echo KO || echo OK
[ ! 5 -eq 6 ] && echo OK || echo KO
[ \( ! \( 5 -eq 5 \) \) ] && echo KO || echo OK
