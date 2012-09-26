react
=====

watch a file, when it changes, react.

usage
=====
    ./react [-fv] <file to watch> <command to run>
    
      -f  run in foreground
      -v  verbose logging


example
=======

working with bootstrap's LESS files, it's a pain to recompile/test/modify/repeat.
lets auto recompile on save:

    react ./less/bootstrap.less "lessc ./less/bootstrap.less > bootstrap.css"


installing
==========

    git clone git clone git://github.com/benlemasurier/react.git
    cd react
    cmake .
    sudo make install

todo
====

* currently we're only watching file content modifications, handle other types as well (open, close, attr move, delete)
* what do if file moves? keep watching? exit? probably an option for either.

