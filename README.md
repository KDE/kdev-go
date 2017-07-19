KDevelop Go Language support plugin
-------------------------------------------------

This plugin introduces Go language support for KDevelop. Go is a programming language developed by Google, KDevelop is a free and open-source IDE developed by KDevelop community, available on most Unix-like systems and Microsoft Windows.

Features
--------------------
**NOTE**: It is still a WIP, so only a part of all Go language features is supported at this time. 
Some of the most important features include:
 -   Code highlighting(currently some of the language constructions are not supported)
 -   KDevelop navigation widgets(information about type, declaration and so on)
 -   Code completion(struct and interface members, type methods and package imports)

**HOWTO install this plugin**

KDevelop from git is required.

1) Follow KDevelop installation instructions [here](https://community.kde.org/KDevelop/HowToCompile_v5)

2) Install KDevelop-PG-Qt from your distribution repository or from git:
```
git clone git://anongit.kde.org/kdevelop-pg-qt
mkdir kdevelop-pg-qt/build && cd kdevelop-pg-qt/build
cmake -DCMAKE_PREFIX_PATH=`your KDevelop install dir` 
 -DCMAKE_INSTALL_PREFIX=`your KDevelop install dir` ../
make && make install
```

3) Install Go(if not already installed)

4) Install Go Plugin

Download this repository,
``` 
mkdir kdev-go/build && cd kdev-go/build
cmake -DCMAKE_PREFIX_PATH=`your KDevelop install dir` 
 -DCMAKE_INSTALL_PREFIX=`your KDevelop install dir` ../
make && make install
```


**HOWTO use this plugin**

I recommend following official code organization suggestions for example like [here](http://golang.org/doc/code.html) or [here](http://www.youtube.com/watch?v=XCsL89YtqCs). In that case you will need to set $GOPATH environment variable to the root of your project before starting KDevelop. Plugin can also try to find path to your project automatically, but it can fail. For go standart library to work you need the path to your 'go' binary be in $PATH environment variable, which if you installed Go normally(with your package manager) should already be there. After that just start KDevelop and open your Go project. If plugin doesn't work(e.g. highlighting is disabled) check if it's loaded in KDevelop(Help->Loaded Plugins). If not - check if it's installed for example like this: find 'KDevelop installation path' -name "*kdevgo*". If it is installed and loaded but still not working consider opening an issue on github or contacting me directly at onehundredof@gmail.com.

For building and executing you can use Go project manager installed as part of Go plugin. See example [here](https://www.youtube.com/watch?v=KxIy53i0RK0).

Also I should mention that there is a very poor support for debugging. GDB can be used to debug Go programs, but it doesn't work well, as stated here http://golang.org/doc/gdb. As a result you can debug Go programs with KDevelop, but it works only as good as it works with gdb. If you are really desperate, somewhat better results can be achieved with gccgo toolchain http://golang.org/doc/install/gccgo.


Implementation details
---------------------------
**Parser**
Plugin uses KDevelop-PG-Qt for parsing Go source code. Complete Go language grammar was written in accordance with official Go language specification available at http://golang.org/ref/spec. Plugin includes separate application for testing parser, located at parser/go_parser. First(and only) argument to the exec is file name, containing go code. If you find correct go source code, which this parser fails to recognize I strongly suggest you contact me at  onehundredof@gmail.com, because fixing grammar by yourself can be tricky.

**DUChain**
Definition-Use chain code is organized like most other language plugins for KDevelop organize it. DeclarationBuilder currently opens declarations and types of variables, functions, methods, packages and imports. UseBuilder builds uses in almost all kinds of expressions(assignments, conditions and so on). ExpressionVisitor can evaluate type of simple expressions, containing variables, basic literals, function calls, comparisons and so on. Complex literals, like function literals are unsupported for now. If you want to contribute to the project you can look at the grammar and see what parts are still not implemented.

**Completion**
Completion code is mostly based on completion for KDevelop QmlJS plugin, so you can use that for details. Currently only variable and function names, struct and interface members, methods, and package imports completion is available.

Road map
-----------------------
If you want to contribute to the project look at this list.
- Implementing rest of Go language features(complex literals, const declarations and so on)
- Implementing smarter completion(try to match type that is needed by expression or function parameter)
- Writing tests and documentation together with improving overall stability of the plugin
