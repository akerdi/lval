; 此行为注释
; buildin方法:
; + - * / head tail list eval concat def = \ > >= < <= == != if print load
(+ 1 2)
( eval { concat { 1 2 3 } { a b c } } )
( def { add } (\ { x y} { + x y }) )
( def { more } ( \ { x & y } { concat x y } ) )
( more { 1 2 3 } 4 5 6  7 )
( more { 1 2 3 } )
( if (> 1 2 ) { + 1 2 } { + 3 4 } )
( if (!= 8 9) { concat { a b c } { d e f } } { + 1 2 } )