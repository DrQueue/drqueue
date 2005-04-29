#!/bin/sh
#
# $Id$
#

usage () {
	cat <<END
DrQueue .tgz builder. Jorge Daza Garcia-Blanes
Usage: $0 <version>

Example: $0 0.50rc4
END
}

if [ $# -lt 1 ]; then
	usage
	exit 1
fi

if [ ! -w common.h ]; then
	echo "No common.h . Wrong directory ? No ? Check permissions."
	exit 1
fi

VERSION=$1

awk '/VERSION/ { print $1,$2," \"SED_VERSION_CHANGE_ME\""; next; }; /.*/ { print }' common.h > common.h.tmp
sed -e "s/SED_VERSION_CHANGE_ME/$VERSION/g" common.h.tmp > common.h
rm -f common.h.tmp

read -p "You are about to commit changes to common.h . Are you sure ? (y/n) " SURE

if [ "$SURE" = "y" ]; then
	echo "Commiting changes..."
	svn ci -m "New version number on common.h: $VERSION" common.h
else
	echo "Not commiting."
	read -p "Still want to build the package ? (y/n) " BUILD
	if [ "$BUILD" != "y" ]; then
		echo "Aborting..."
		exit 1
	fi
fi

# Create Changelog
read -p "Do you want to recreate and commit the ChangeLog ? (y/n) " CHLG
if [ "$CHLG" != "n" ]; then
	echo "Creating ChangeLog"
	svn log -v | ./svn-scripts/svn2cl.pl > ChangeLog # Dump log to ChangeLog
	echo "Created !"
	echo "Commiting ChangeLog"
	svn ci -m "ChangeLog commited by build_package.sh" ChangeLog
fi

# Creating tag
read -p "Do you want to create a tag for this package ? (y/n) " CRTTAG
if [ "$CRTTAG" = "y" ]; then
	echo "Creating tag $VERSION"
	(cd ..; svn cp -m "Tag $VERSION created by build_release.sh" https://ssl.drqueue.org/svn/trunk/drqueue \
                                                               https://ssl.drqueue.org/svn/tags/$VERSION)
	echo "Created !"
  # Update Revision
  echo "Updating Revision"
  svn update > Revision
  echo "Commiting new Revision"
  svn ci -m "New Revision commited by build_package.sh" Revision
fi

# Build package
echo "Building package"
make clean > /dev/null
(
cd ..
svn co https://ssl.drqueue.org/svn/tags/$VERSION drqueue-$VERSION
tar zcvf drqueue.$VERSION.tgz --exclude="*/.svn" drqueue-$VERSION
rm -fR drqueue-$VERSION
) > /dev/null

# Package ChangeLog
read -p "Do you want to create a package ChangeLog compared to previous tag ? (y/n) " CHLG
if [ "$CHLG" = "y" ]; then
	read -p "From tag ?: " OLDTAG
	OLDREV=`svn log --stop-on-copy https://ssl.drqueue.org/svn/tags/$OLDTAG | awk '/^r[0-9]+/ { sub (/r/,"",$1); print $1; }'`
	echo "Old Revision number: $OLDREV"
	echo "Creating ChangeLog.$VERSION"
	(svn log -v -r HEAD:$OLDREV | ./svn-scripts/svn2cl.pl > ../ChangeLog.$VERSION )
fi
