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

working with LESS files:

    react ./less/bootstrap.less "lessc ./less/bootstrap.less > bootstrap.css"

todo
====

* currently we're only watching file content modifications, handle other types as well (open, close, attr move, delete)
* what do if file moves? keep watching? exit? probably an option for either.

