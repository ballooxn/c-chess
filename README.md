I've decided to start creating a chess game and engine in C.
I've already made one in Python, but I've decided to challenge myself by making it in C.

ENGINE LEVEL:
Beat nelson bot (1300)
Drew Li by repetition  (2000)


To-do:
50 move rule
3 move repitition
test insufficient material
Increase depth when only a few pieces left
add transposition table

Count in mobility (only slightly) (mobility of engine - mobility of opponent)
Count in isolated/doubled/blocked pawns
Add piece-square tables
King safety


Add opening book

ideas:
make move struct just integers (prob wont affect much in terms of performance)