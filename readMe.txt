-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA256

# recfsusb2i   for Linux OSes
2016-02-20, Ver. 0.1.25


## What is this?

* record TS data from FSUSB2i (ISDB-T TV Tuner)
* monitor signal status, channel status



## System Requirements

Linux-based OS
usbdevfs (usbfs) with USB 2.0



## Files

src/
	* source code files (gcc C99)
	Makefile{,.dep}
	*.{c,h}

/
	readMe.txt
	doc.txt
		* instructions, usage, FAQ
	sha1sum.txt
	sha1sum.txt.sig
		* gpg --verify sha1sum.txt.sig



## User Agreement

USE AT YOUR OWN RISK, no warranty

You can ...
* use this package for any purposes.
* redistribute a copy of this package without modifying.
* modify this package.
* redistribute the modified package. You should write the modification clearly.

based on GPLv3



## How to compile and build?

I recommend the GCC C compiler. Please install "gcc".

  $ cd src
run GNU "make" command.
  $ make

* "make B25=1"  if you use the STD-B25 interface library

You will get a executable file "recfsusb2i".
Check to see if it is working properly.

Last, copy to ".../bin" directory.
  $ cp recfsusb2i /usr/local/bin/



## End

(c) 2015-2016 trinity19683
signed by "trinity19683.gpg-pub" 2015-12-13

You can verify this file by using the GnuPG key.

$ gpg --import trinity19683.gpg-pub
$ gpg --verify readMe.txt
-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1

iQEcBAEBCAAGBQJWyB8qAAoJEB0tpC02lUUYdBEH/3NpbkdxB8GoFJLP44rqV2Wq
ONCZi5HvjUd1l6CVLx2H7wyzzU62osfJ+ynHtyaALW0V1gxY8f/h9JVOTPNy5Jns
S04HHiAQx+IqyFHg+KIuDDeigs3fyVqb4jwVGI6V8/tQeIaXX8/oCBk9oTMzU8fi
jdXhm3l0wEx+ZEXVCW1IPOyDnyOVoFD1XPK/mqVtdw/wAE1DFedKZlCJUI7Kp0uJ
O6tLZh1ap8qDNX2xGIBUe4DSKijPxmaA2pUfWKTN/qQIFOrCDKB2idFB96rRDIyB
fRvSrOBY3w5YLbU3GFhq+QBZdXbAYbpCiYzn7lud+n4qSknIg+o5e8oRDZwbb/w=
=RPmt
-----END PGP SIGNATURE-----
