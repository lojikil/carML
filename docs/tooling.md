# Overview

I think the most important thing for new languages is not the language itself,
but rather that the **ecosystem** _surounding_ the language is strong. To that
end, there's a few tools I'd like carML to include as we get nearer to release:

- static analysis
- symbolic execution/abstract interpretation
- package/project configuration & management
- base set of libraries

# Static Analysis

carML is meant to be simpler to analyze than C proper, because it is 
smaller, with less ambigious syntax, and restricted program space, so
static analysis internally at release would be useful.

# SE/AI

This one I'm less set on, but it would be nice to have a Frama-C
work-alike.

# Packaging

I think having reasonable package & library management is pretty 
essential nowadays. I'd like to add a tool, `carmlize`, to the base
install that basically handles the core setup of projects, libraries
and the like. My thinking is that `use` and `import` would check
some set of paths, such as `./lib:~/.carml/lib:/usr/lib/carml:/usr/local/lib/carml:$CARML_LIB`
so that clients can easily bundle, have virtual environments, &c. 
all via default install.

`carmlize [init|install|ginstall|bundle|dep-install...]```

# Base Libs

- HTTP (including WebDAV, CalDAV)
- TLS
- SMTP
- FTP
- tar/zip
- data structures (including png/jpeg/gif/&c.)
- image
- CLI/GUI
