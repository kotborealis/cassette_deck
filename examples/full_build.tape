OUTPUT demo/full_build.gif
THEME default_dark

SLEEP 1000

TYPE cd ~
ENTER
AWAIT
TYPE git clone --recursive git@github.com:kotborealis/cassette_deck.git
ENTER
AWAIT

TYPE cd cassette_deck
ENTER
AWAIT

TYPE cmake -B./build
ENTER
AWAIT

TYPE cmake --build ./build
ENTER
AWAIT

SLEEP 5000
