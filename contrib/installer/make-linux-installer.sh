#!/bin/bash
BINFILE=./freecraft-030311-linux.tar.gz
INSTALLERNAME=./freecraft-$(date +%y%m%d)-linux-installer.sh

FCMPVER=030311
FCMPONLINE=http://telia.dl.sf.net/freecraft/fcmp-$FCMPVER.tar.gz
FCMPLOCAL=fcmp-$FCDATE.tar.gz
FCMPFILE=./fcmp-$FCDATE.tar.gz

STARTLINE=`cat $0 | nl -ba | grep startpointer | grep -v STARTLINE | cut -f1`
ENDLINE=`cat $0 | nl -ba | grep endpointer | grep -v ENDLINE | cut -f1`
SKIP=`expr $ENDLINE - $STARTLINE - 2`

#do not edit this line or put any spaces below : startpointer
cat << EOF > $INSTALLERNAME
#!/bin/bash
SKIP=$SKIP
CURDIR=\`pwd\`
DEFAULTDIR=/usr/local/games
DEFAULTSTARTDIR=/usr/local/bin
if [ "\`id -u\`" != "0" ]; then
 echo "You are not running this installer as root."
 echo "Please only install to your HOME directory."
fi
echo;
echo "=================================="
echo "Welcome to the FreeCraft installer"
echo "=================================="
echo
echo -n "Would you like to [i]nstall or [u]ninstall FreeCraft? (i) "
read -n1 -s OPTION
echo
echo
if [ "\$OPTION" = "u" ] || [ "\$OPTION" = "U" ]; then
    if [ ! -f ~/.freecraft/freecraft.inst ] || [ "\`cat ~/.freecraft/freecraft.inst | grep -iv REMOVE\`" = "" ]; then
        echo "Cannot uninstall: FreeCraft does not appear to be installed."
	echo
        exit
    fi
    FILES=\`cat ~/.freecraft/freecraft.inst | grep -iv REMOVE\`
    echo "The following files/directories will be removed:"
    echo
    echo "\$FILES"
    echo
    echo -n "Would you like to continue? (n) "
    read -n1 -s CONTINUE
    echo
    if [ "\$CONTINUE" = "y" ] || [ "\$CONTINUE" = "Y" ]; then
	echo
	echo -n "Removing..."
	rm -rf \$FILES
	rm ~/.freecraft/freecraft.inst
	echo "Done"
	echo
    fi
    exit
fi
echo "Where would you like to install the 'freecraft' directory? "
echo -n "( \$DEFAULTDIR ) "
read DIR
if [ "\$DIR" = "" ]; then 
  DIR=\$DEFAULTDIR
fi
cd \$DIR
DIR=\`pwd\`
cd \$CURDIR
echo -n "PLEASE WAIT"
mkdir -p \$DIR
tail +\$SKIP \$0 | gzip -cd | tar -x -C \$DIR --exclude=fcmp
echo
echo
echo -n "Would you like to use [f]cmp or the [o]riginal Warcraft 2 data? (f) "
read -n1 -s DATA
echo
if [ "\$DATA" = "o" ] || [ "\$DATA" = "O" ]; then
  DATADIR=data.wc2
  echo
  echo -n "What is the device that the Warcraft 2 CD is in? ( /dev/cdrom ) "
  read DEVICE
    if [ "\$DEVICE" = "" ]; then
      DEVICE="/dev/cdrom"
    fi
  mkdir -p /tmp/cdrom.tmp
  CDROM=/tmp/cdrom.tmp
  mount \$DEVICE /tmp/cdrom.tmp
  if [ ! -f \$CDROM/data/rezdat.war ]; then
    echo
    echo "Error: data not found, make sure your cdrom is mounted."
    echo "Installation NOT complete."
    rm -rf \$DIR/freecraft
    echo
    exit
  fi
  echo -n "Please wait while data is being extracted, this will take several minutes..."
  echo
  mv \$DIR/freecraft/data \$DIR/freecraft/data.wc2
  sh \$DIR/freecraft/tools/build.sh -o \$DIR/freecraft/data.wc2 -C \$DIR/freecraft/contrib -p \$CDROM/data -T \$DIR/freecraft/tools
else
  DATADIR=data
  echo
  echo -n "Would you like [d]ownload FcMP or do you have a [l]ocal copy? (d) "
  read -n1 -s DOWN
  if [ "\$DOWN" = "l" ] || [ "\$DOWN" = "L" ]; then
    echo
    echo -n "Where is the FcMP located? ( ./fcmp-$FCMPVER.tar.gz ) "
    read FCMPLOCATION
    if [ "\$FCMPLOCATION" = "" ]; then
	FCMPLOCATION="./fcmp-$FCMPVER.tar.gz"
    fi
    if [ ! -f \$FCMPLOCATION ]; then
	echo
	echo "Error: file not found."
	echo "Installation NOT complete."
	rm -rf \$DIR/freecraft
	echo
	exit
    fi
    echo -n "PLEASE WAIT"
    tar -zxvf \$FCMPLOCATION -C \$DIR/freecraft >/dev/null
    echo
  else
    echo
    echo -n "PLEASE WAIT - DOWNLOADING"
    if [ "\`whereis curl | cut -f2 -d:\`" = "" ]; then
	wget -nv -O - $FCMPONLINE | tar -zxv -C \$DIR/freecraft
    else
	curl -s $FCMPONLINE | tar -zxv -C \$DIR/freecraft
    fi
  fi
  echo
fi;
echo
echo "Where would you like to put the 'freecraft' startup script?"
echo -n "( \$DEFAULTSTARTDIR ) "
read STARTDIR
if [ "\$STARTDIR" = "" ]; then
  STARTDIR=\$DEFAULTSTARTDIR
fi
cd \$STARTDIR
STARTDIR=\`pwd\`
cd \$CURDIR
mkdir -p \$STARTDIR
if [ "\`dirname \$STARTDIR/x\`" = "\`dirname \$DIR/x\`" ]; then
    STARTDIR=\$STARTDIR/freecraft
fi
if [ "\`dirname \$STARTDIR\`" != "\`dirname \$DIR/x\`" ]; then
    echo "\$DIR/freecraft/freecraft -d \$DIR/freecraft/\$DATADIR" > \$STARTDIR/freecraft
    chmod +x \$STARTDIR/freecraft
else
    echo
    echo "*** NOTE: you will need to 'cd \$DIR/freecraft' before running ./freecraft ***"
fi
mkdir -p ~/.freecraft
echo "THIS FILE REQUIRED FOR STRATAGUS UNINSTALLATION, DO NOT REMOVE" > ~/.freecraft/freecraft.inst
echo "\$DIR/freecraft \$STARTDIR/freecraft" >> ~/.freecraft/freecraft.inst
if [ -d /tmp/cdrom.tmp ]; then umount /tmp/cdrom.tmp; rm -rf /tmp/cdrom.tmp; fi
echo
echo "Congratulations! Installation of FreeCraft is complete."
echo
echo "To start the game, type the following command:"
echo
echo "\$STARTDIR/freecraft"
echo
echo -n "Would you like to start the game now? (n) "
read -n1 -s START
echo
if [ "\$START" = "y" ] || [ "\$START" = "Y" ]; then
  cd \$STARTDIR
  \$STARTDIR/freecraft
fi;
exit;
EOF
#do not edit this line or put any spaces above : endpointer

echo
echo =====================================

if [ -d .installer ]; then
rm -rf .installer
fi

mkdir .installer

tar -zxf $BINFILE -C .installer
mv .installer/`ls .installer | grep freecraft` .installer/freecraft

cd .installer
tar -hcO * | gzip -c >> ../$INSTALLERNAME
cd ..
chmod +x $INSTALLERNAME
rm -rf .installer
echo made $INSTALLERNAME
echo
