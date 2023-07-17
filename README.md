## TODOs

- [ ] **Better errors**. Most errors except for in the lexer or parser are reported via `printf` with no relevant context. Either attach a token to every node in the AST or just print an error on the line and make the message as easy to understand as possible. 
- [ ] **A proper test suite**. Not only does this include actually creating a way to run tests (both compatibility and regression tests), but it also requires that the code be made more testable. This means we should be able to observe the state of the lexer, parser, etc. via high-level functions without much boilerplate.
- [ ] **Documentation**. This project's primary purpose, aside from teaching myself compiler internals, is to become more familiar with modern C++ concepts (no pun intended). Writing documentation for non-obvious choices will help cement the ideas that I learn.