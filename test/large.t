setup:

  $ . $TESTDIR/setup
  $ mkdir -p files/large
  $ dd bs=4096 count=4096 if=/dev/zero of=files/large/l0 >/dev/null 2>&1
  $ cp files/large/l0 files/large/l1

large file (match):

  $ yafd -b 16777216 files/large/l[01]
  files/large/l* (glob)
  files/large/l* (glob)

cleanup:

  $ rm -rf files/large
