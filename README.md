(Ubuntu 16.04 blank)
(use default sources.list)

Tools:
```xml
apt get update ; apt install -y ccache lzma-dev liblzma-dev qemu u-boot-tools gtk-doc-tools uuid-dev cmake libglib2.0-dev build-essential liblzo2-dev xsltproc libstdc++5 patch bison gcc m4 gperf lzma binutils-dev libncurses5 intltool docbook-xsl-* autoconf autopoint libncurses5-dev m4 bison gawk flex device-tree-compiler gengetopt zlib1g-dev mtd-utils shtool autogen texinfo python-minimal dos2unix libtool lib32z1 lib32stdc++6 libc6-i386 lib32ncurses5 libc6-dev-i386
```
Repos:
```xml
cd /opt ; git clone https://github.com/rp-n12/buildroot-gcc342.git ; cd ~ ; git clone https://github.com/rp-n12/Source.git
```
Create config and select RP-N12
```xml
make menuconfig
```
Build
```xml
make
```

Enjoy
