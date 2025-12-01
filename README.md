# miniFSM

A lightweight, thread-safe finite state machine (FSM) implemented in C++.

## build
> mkdir build
> cd build
> cmake ..
> make
> # ctest or make test
> # ./tests/test_{test_name}

## testing coverage
> ...
> cmake .. -DENABLE_COVERAGE=ON
> ...
> make coverage
> # open coverage_html/index.html in browser to see coverage report