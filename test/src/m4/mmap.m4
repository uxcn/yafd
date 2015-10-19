setup:

  $ . $TESTDIR/setup

mmap i/o:

  $ if test -z $mmap; then exit 80; fi
  $ yafd -m < files/basic/list| sets | sort
esyscmd(sort files/basic/sets | sed s/^/"  "/)
