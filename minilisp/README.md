about minilisp
--------------

*minilisp* is my testbed for language design and memory management studies.

compile via *build.sh* and then run the *minilisp* executable for a REPL.

tested on debian linux and mac os x.

type *quit* to exit REPL.

minilisp will now start a web server on port 5554 (configured in http.c). this takes GET requests and forwards their Path header value to the function httpd-get in boot.l.

backspace in the REPL is currently dysfunctional because i switched to raw terminal. will fix asap. on the upside, this allows us to serve web requests in parallel to having REPL input. voilà: a runtime-inspectable and modifiable web server.

arithmetic
----------

    mini> (* 5555555555 44444)
    246911111086420
    mini> (* 5555555555 (+ 40000 4444))
    246911111086420

definition
----------

    mini> (def hey 1234)
    1234
    mini> (* hey 5)
    6170

functions
---------

    mini> (def double (fn (x) (+ x x)))
    (LAMBDA ((+ x x) ))
    mini> (double 222333)
    444666
    mini> (double (double 222333))
    889332

conditionals
------------

    mini> (if 1 0 1)
    0
    mini> (if 0 0 1)
    1
    mini> (if (< 1 222222222222222222222) 12 34)
    12

let syntax
----------

let doesn't use extra parens for each binding to provide for a more terse syntax:

    (let (a 1 b 2 c 3) (+ a b))

byte vectors
------------

byte vectors can be initialized with square brackets filled with hex digit pairs. white space is ignored:

    mini> [dead beef]
    [deadbeef]

    mini> (def beef [beefbeef])
    [beefbeef]
    mini> (get beef 0)
    190
    mini> (get beef 1)
    239
    mini> (set beef 0 255)
    255
    mini> beef
    [ffefbeef]

strings
-------

strings are work-in-progress, they don't support utf-8 natively *yet* but this is a to-do.

    mini> (substr "foobar" 2 3)
    "oba"
    mini> (substr [11223344] 0 2)
    [1122]
    mini> (substr "foobaz" 5)
    "z"
    mini> (concat 1 2 3)
    "123"
    mini> (concat (list "abc" 3 [65]))
    "abc3[65]"
    mini> (concat (list "abc" 3 (str [65])))
    "abc3e"
    mini> (= "foo" "foo")
    1
    mini> (= "foo" "baz")
    0

map
---

map works on lists as well as on strings and passes your function the current loop index as optional second parameter.

    mini> (map (fn (x) (concat x "o")) "hello")
    ("ho" "eo" "lo" "lo" "oo")
    mini> (map (fn (x i) (* i 2000)) (list 1 1 1 1))
    (0 2000 4000 6000)

run-time type checking
----------------------

(type x) gives you an integer with the minilisp tag of the given argument incremented by one, or 0 if x is nil (empty list).

    (def is-nil  (fn (x) (= 0 (type x))))
    (def exists  (fn (x) (> (type x) 0)))
    (def is-int  (fn (x) (= 1 (type x))))
    (def is-list (fn (x) (= 2 (type x))))
    (def is-str  (fn (x) (= 7 (type x))))

    mini> (is-nil ())
    1
    mini> (is-nil)
    1
    mini> (is-str "foo")
    1
    mini> (exists)
    0
    mini> (exists ())
    0
    mini> (exists (list 1))
    1
    mini> (exists 1)
    1

tips and tricks
---------------

number to string:

    mini> (concat 1138)
    "1138"

string to number:

    mini> (read "1138")
    1138

hex string to number:

    mini> (get (read "[be]") 0)
    190

