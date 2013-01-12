       PROGRAM TESTF_NP
C
       IMPLICIT NONE
       INTEGER i, ACEgrOpenf
       CHARACTER*64 buf
C
C      Start xmgr with a buffer size of 2048 and open the pipe
C
       IF (ACEgrOpenf(2048) .EQ. -1) THEN
           WRITE (*,*) 'Can\'t run xmgr.'
           CALL EXIT (1)
       ENDIF
C
C      Send some initialization commands to xmgr
C
       CALL ACEgrCommandf ('world xmax 100')
       CALL ACEgrCommandf ('world ymax 10000')
       CALL ACEgrCommandf ('xaxis tick major 20')
       CALL ACEgrCommandf ('xaxis tick minor 10')
       CALL ACEgrCommandf ('yaxis tick major 2000')
       CALL ACEgrCommandf ('yaxis tick minor 1000')
       CALL ACEgrCommandf ('sets symbol 2')
       CALL ACEgrCommandf ('sets symbol fill 1')
       CALL ACEgrCommandf ('sets symbol size 0.3')
C
C      Display sample data
C
       DO i = 1, 100, 1
           WRITE (buf, 1) i, i
           CALL ACEgrCommandf (buf)
           WRITE (buf, 2) i, i**2
           CALL ACEgrCommandf (buf)
C
C          Update the xmgr display after every ten steps
C
           IF (10*(i / 10) .EQ. I) THEN
               CALL ACEgrCommandf ('redraw')
C                Wait a second, just to simulate some time needed for
C                   calculations. Your real application shouldn't wait.
               CALL SLEEP (1)
           ENDIF
       ENDDO

C
C      Tell xmgr to save the data
C
       CALL ACEgrCommandf ('saveall "sample.gr"')
C
C      Flush the output buffer and close the pipe
C
       CALL ACEgrClosef ()
C
C      We are done
C
       CALL EXIT (0)
C      
 1     FORMAT ('g0.s0 point ', I6, ' , ', I6)
 2     FORMAT ('g0.s1 point ', I6, ' , ', I6)
C
       END

