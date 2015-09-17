setup:

  $ . /$TESTDIR/setup

basic (CRC32):

  $ yafd -C < files/basic/list | sets | sort
esyscmd(sort files/basic/sets | sed s/^/"  "/)

basic (FNV1a 32):

  $ yafd -N < files/basic/list | sets | sort
esyscmd(sort files/basic/sets | sed s/^/"  "/)

basic (FNV1a 64):

  $ yafd -V < files/basic/list | sets | sort
esyscmd(sort files/basic/sets | sed s/^/"  "/)

basic (CityHash 64):

  $ yafd -T < files/basic/list | sets | sort
esyscmd(sort files/basic/sets | sed s/^/"  "/)

basic (CityHash 128):

  $ yafd -Y < files/basic/list | sets | sort
esyscmd(sort files/basic/sets | sed s/^/"  "/)

basic (SppokyHash 64):

  $ yafd -S < files/basic/list | sets | sort
esyscmd(sort files/basic/sets | sed s/^/"  "/)

basic (SppokyHash 128):

  $ yafd -P < files/basic/list | sets | sort
esyscmd(sort files/basic/sets | sed s/^/"  "/)

basic (large offset):

  $ yafd -o 1073741824 < files/basic/list | sets | sort
esyscmd(sort files/basic/sets | sed s/^/"  "/)

basic (large negative offset):

  $ yafd -o -1073741824 < files/basic/list | sets | sort
esyscmd(sort files/basic/sets | sed s/^/"  "/)

basic (large bytes):

  $ yafd -b 1073741824 < files/basic/list | sets | sort
esyscmd(sort files/basic/sets | sed s/^/"  "/)

basic (large blocksize):

  $ yafd -k 1073741824 < files/basic/list | sets | sort
esyscmd(sort files/basic/sets | sed s/^/"  "/)

basic (threads):

  $ yafd -t 4 < files/basic/list | sets | sort
esyscmd(sort files/basic/sets | sed s/^/"  "/)

basic (mmap i/o):

  $ yafd -m < files/basic/list | sets | sort
esyscmd(sort files/basic/sets | sed s/^/"  "/)
