setup:

  $ . $TESTDIR/setup
  $ mkdir -p files/delete
  $ touch files/delete/d0 files/delete/d1 files/delete/d2 files/delete/d3 

delete:

  $ yafd -z -d -f files/delete/d[0-3]
  files/delete/d0 ...none
  files/delete/d1 ...delete
  files/delete/d2 ...delete
  files/delete/d3 ...delete

  $ ls files/delete
  d0

cleanup:

  $ rm -rf files/delete
