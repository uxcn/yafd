setup:

  $ . $TESTDIR/setup
  $ mkdir -p files/link
  $ touch files/link/l0 files/link/l1 files/link/l2 files/link/l3

link:

  $ yafd -z -l -f files/link/l[0-3]
  files/link/l* ...link (glob)
  files/link/l* ...link (glob)
  files/link/l* ...link (glob)
  files/link/l* ...link (glob)

  $ ls -l files/link | tail -n +2 | awk '{ print $2 }'
  4
  4
  4
  4

cleanup:

  $ rm -rf files/link
