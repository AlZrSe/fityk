#!/bin/bash 

version=1.2.0
WEB="iris.unipress.waw.pl:www/fityk2/"

MINGW_DIR=mingw-build
ALL_WIN_FILES=all_files #inside of $MINGW_DIR

win_setup_filename=$MINGW_DIR/all_files/Output/fityk-$version-setup.exe
tarball_filename=fityk-$version.tar.bz2

if [ $# -eq 0 ]; then 
 echo Version in this script is set to $version
 echo usage: $0 step_nr
 echo steps:
 echo 0. prepare new version and increase version number 
 echo 1. run samples
 echo "2. cd builds/ && make all"
 echo 3. make tarball, update daily shapshots
 echo 4. compile windows version and make installer
 echo 5. https://build.opensuse.org/project/show?project=home%3Awojdyr
 echo 8. SourceForge release
 echo 9. put docs on www
 echo "10. http://freshmeat.net/projects/fityk/releases/new "
 echo "    http://www.gnomefiles.org/devs/index.php?login    "
 echo "11. (optional) update xyconvert"
 echo "12. git tag -a v$version -m 'version $version'; git push --tags"
 exit
fi

echo
echo        Step $1 of the release procedure...         
echo
echo -n '===>' 



if [ $1 -eq 0 ]; then
 echo now the version in this script is: $version
 echo configure.ac:
 grep AC_INIT configure.ac
 echo doc/conf.py:
 grep 'version =' doc/conf.py
 echo doc/index.rst:
 grep 'Version ' doc/index.rst
 echo NEWS:
 head -4 NEWS | grep version
 echo


elif [ $1 -eq 1 ]; then
 echo run samples...
 cd ./samples
 ../cli/cfityk nacl01.fit
 ../cli/cfityk test_syntax.fit
 ../wxgui/fityk nacl01.fit
 ../wxgui/fityk SiC_Zn.fit
 make && ./hello


elif [ $1 -eq 2 ]; then
 cd builds/ && make all
 
elif [ $1 -eq 3 ]; then
 echo  make tarball
 make dist-bzip2
 cd builds/
 make daily

elif [ $1 -eq 4 ]; then
 echo Building MS Windows version
 SRC_DIR=fityk-$version/
 MDIR=$HOME/local/mingw32
 #BOOST_DIR=$HOME/local/src/boost_1_42_0/
 BOOST_DIR=/usr/local/boost
# rm -rf $MINGW_DIR
# mkdir -p $MINGW_DIR
 cd $MINGW_DIR
# tar xjf ../$tarball_filename || exit 1
# # host: MinGW from .deb: i586-mingw32msvc, built locally: i586-pc-mingw32
# #       from Fedora: i686-w64-mingw32
# $SRC_DIR/configure --host=i686-w64-mingw32 \
#   CPPFLAGS="-I$BOOST_DIR -I$MDIR/include/" \
#   CXXFLAGS="-O3" LDFLAGS="-s -L$MDIR/lib" \
#   --without-readline --with-wx-config=$MDIR/bin/wx-config \
#   --disable-static --enable-shared
#   #--enable-static --disable-shared
# make || exit
 mkdir -p $ALL_WIN_FILES/samples $ALL_WIN_FILES/fityk
 cp fityk.iss $SRC_DIR/fityk.url $SRC_DIR/COPYING $SRC_DIR/TODO \
    $SRC_DIR/NEWS $ALL_WIN_FILES
 cp -r $SRC_DIR/doc/html/ $ALL_WIN_FILES/
 cp $SRC_DIR/samples/*.fit $SRC_DIR/samples/*.dat $SRC_DIR/samples/*.lua \
    $SRC_DIR/samples/README $ALL_WIN_FILES/samples/
 #cp wxgui/fityk.exe cli/cfityk.exe $ALL_WIN_FILES/fityk/
 cp wxgui/.libs/fityk.exe cli/.libs/cfityk.exe fityk/.libs/libfityk-*.dll \
    $MDIR/bin/libxy-*.dll $MDIR/bin/xyconv.exe $MDIR/bin/lua5*.dll \
    $ALL_WIN_FILES/fityk/
 cp /usr/i686-w64-mingw32/sys-root/mingw/bin/libgcc_s_sjlj-1.dll \
    /usr/i686-w64-mingw32/sys-root/mingw/bin/libstdc++-6.dll \
    $ALL_WIN_FILES/fityk/
 echo everything is in: `pwd`/$ALL_WIN_FILES
 

elif [ $1 -eq 8 ]; then
 echo  SF release
 echo uploading files...
 [ ! -e $tarball_filename ] && echo "File not found: $tarball_filename" && exit
 [ ! -e $win_setup_filename ] && echo "File not found: $win_setup_filename" \
                                                                       && exit
 mkdir $version || exit
 cp $tarball_filename $win_setup_filename $version/
 scp -r $version "wojdyr,fityk@frs.sourceforge.net:/home/frs/project/f/fi/fityk/fityk/"
 rm -r $version/
 echo now you may go to:
 echo https://sourceforge.net/project/admin/explorer.php?group_id=79434
 echo or to http://sourceforge.net/projects/fityk/files/
 

elif [ $1 -eq 9 ]; then
 echo  putting docs on www   
 echo destination: $WEB
 cd doc/ 
 make all pdf
 echo "sending PDF manual..."
 scp latex/fityk-manual.pdf $WEB/
 #echo sending html
 #scp -r html/* $WEB/
 echo generating doxygen docs...
 cd ../doxygen/
 doxygen
 echo sending doxygen docs... 
 scp -r html/ $WEB/doxygen/
 cd ..
 

elif [ $1 -eq 10 ]; then
 echo announce: freshmeat.net gnomefiles.com
 
elif [ $1 -eq 11 ]; then
 echo "11. (optional) update xyconvert"
 xylib_version=0.6
 cp -f $MINGW_DIR/wxgui/xyconvert.exe . || exit 1
 rm -f xyconvert-$xylib_version.zip
 zip xyconvert-$xylib_version.zip xyconvert.exe
 rm -f xyconvert.exe
 (cd wxgui/ && make xyconvert.html)
 scp wxgui/xyconvert.html $WEB/xyconvert/index.html
 scp xyconvert-$xylib_version.zip $WEB/xyconvert/

else
 echo unexpected step number: $1
fi

