#!/bin/sh
progname=`basename $PWD`
full_vername=$progname-`git describe --tags | sed s/^v//`
git tar-tree HEAD $full_vername | gzip -c > ../$full_vername.tar.gz
