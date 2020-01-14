gcc -Wall `pkg-config --cflags gtk+-3.0` -o ssi.out mail.c mail.h security_code.c security_code.h  passthrough_fh.c  -lcurl -lulockmgr `pkg-config --libs gtk+-3.0` `pkg-config fuse3 --cflags --libs`
