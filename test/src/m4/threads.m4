setup:

  $ . $TESTDIR/setup

threads:

  $ if test -z $threads; then exit 80; fi
  $ yafd -t 4 < files/basic/list | sets | sort
esyscmd(sort files/basic/sets | sed s/^/"  "/)
