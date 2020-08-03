# The game of life
A simple implementation of "the game of life" using c and the gtk library for the graphic interface

The code can be compiled using [compile.bat](https://github.com/Ferraa96/GameOfLife/blob/master/compile.bat)

## The map
There are two different way of running the game

* Random map: the map is generated randomly using `Math.h` with the default size 30 x 60
* Input map: the map is deserialized from a file, in which are specified the number of rows, the number of columns and the effective map, composed by living elements "`1`" and dead elements "`0`".
  
Input map format example
```
14 38
00000000000000000000000000000000000000
00000000000000000000000001000000000000
00000000000000000000000101000000000000
00000000000001100000011000000000000110
00000000000010001000011000000000000110
01100000000100000100011000000000000000
01100000000100010110000101000000000000
00000000000100000100000001000000000000
00000000000010001000000000000000000000
00000000000001100000000000000000000000
00000000000000000000000000000000000000
00000000000000000000000000000000000000
00000000000000000000000000000000000000
00000000000000000000000000000000000000
```

![](https://github.com/Ferraa96/GameOfLife/Documentation/animation.gif)