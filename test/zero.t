setup:

  $ . $TESTDIR/setup
  $ mkdir -p files/zero
  $ touch files/zero/z0 files/zero/z1

zero size (match):

  $ yafd -z files/zero/z[01]
  files/zero/z* (glob)
  files/zero/z* (glob)

zero size (no match):

  $ yafd files/zero/z[01]

cleanup:

  $ rm -rf files/zero
