#!/bin/sh
progname=`basename $PWD`
full_vername=$progname-`git describe --tags | sed s/^v//`
cd ..
cp -RP $progname $full_vername
tar czvf $full_vername.tar.gz --exclude '*/.*' $full_vername
rm -rf $full_vername
