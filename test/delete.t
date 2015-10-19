setup:

  $ . $TESTDIR/setup
  $ mkdir -p files/delete
  $ touch files/delete/d0 files/delete/d1 files/delete/d2 files/delete/d3 

delete:

  $ yafd -z -d -f files/delete/d[0-3]
  files/delete/d* ...none (glob)
  files/delete/d* ...delete (glob)
  files/delete/d* ...delete (glob)
  files/delete/d* ...delete (glob)

  $ ls files/delete
  d* (glob)

cleanup:

  $ rm -rf files/delete
