OUTPUT ./demo/meta.tape.gif

SLEEP 500
TYPE cat ./examples/meta.tape
ENTER
SLEEP 2000
TYPE # Next command takes a while, we don't know exactly how long
ENTER
SLEEP 500
TYPE # Instead of sleep, we'll use AWAIT command
ENTER
SLEEP 500
TYPE ./build/cassette_deck ./examples/hello.tape
ENTER
AWAIT

TYPE file ./demo/hello.tape.gif
ENTER
AWAIT
SLEEP 5000
