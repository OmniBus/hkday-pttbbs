REFERENCE
=========

https://opensvn.csie.org/pttbbs/trunk/pttbbs/docs/FAQ
https://opensvn.csie.org/pttbbs/trunk/pttbbs/docs/INSTALL
http://blog.roodo.com/shishimaru/archives/10437789.html

ENVIRONMENT
===========
Ubuntu LTS 12.04 (x86-64) clean

INIT
====

* as root

#############
# Setup Apt
apt-get update

#############
# Setup compiler , versioning , debugger
apt-get install build-essential git-core gdb

#############
# Basic tools ( encrypt , archiving , decoding )
apt-get install pgp zip libiconv-hook-dev

#############
# Setup package configuration, cacher, more compiler
apt-get install pkg-config ccache clang pmake

mv /usr/bin/make /usr/bin/make-linux
ln -s /usr/bin/pmake /usr/bin/make

#############
# Install event library
apt-get install libevent-dev libevent-core-2.0-5

#############
# Add BBS group
echo ¡§bbs:x:99:¡¨ >> /etc/group

#############
# Setup Shared Memory in Kernel ( Need reboot to take effect )
vim /etc/sysctl.conf
	# add rows:
	kernel.shmmax=104857600
	kernel.shmall=25600

#############
# Setup unix Accounts for BBS ( use passwd to update password for bbsadm )
vipw
	# add rows:
bbs::9999:99:HKDAY.NET:/home/bbs:/home/bbs/bin/bbsrf
bbsadm:x:9999:99:HKDAY.NET:/home/bbs:/bin/bash

#############
# Setup home in hard drive
mkdir /home/bbs
chown bbs.bbs /home/bbs
chmod 700 /home/bbs

#############
# Change to BBSadm 
su - bbsadm
cd~

#############
# Get source
git clone https://github.com/bencrox/hkday-pttbbs.git

#############
# Make the binaries
cd hkday-pttbbs
make OSTYPE=linux BBSHOME=/home/bbs all install

#############
# WARNING!!! Do this only for 1st setup in your environment!
cd sample
make install
cd ~
bin/initbbs -DoIt

#############
# initialize the shared memory
bin/shmctl init

#############
# back to root, optional, use other ports > 1024 if not root
exit

	now as root

cd ~bbs
bin/mbbsd -p 23 -d

#############
# Done~!  if not , report to lxb at hkday dot net 
