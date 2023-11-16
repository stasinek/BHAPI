#!/bin/sh
for i in `find . -name "Makefile"` ; do mv $i __tmp; cat __tmp |tr -d  > $i; rm __tmp; done
for i in `find . -name "Sources"` ; do mv $i __tmp; cat __tmp |tr -d  > $i; rm __tmp; done
for i in `find . -name "dirs"` ; do mv $i __tmp; cat __tmp |tr -d  > $i; rm __tmp; done
for i in `find . -name "*.[ch]"` ; do mv $i __tmp; cat __tmp |tr -d  > $i; rm __tmp; done

