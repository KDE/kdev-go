package main

import "fmt"


type functest func() int


func main() {

    var testvar functest
    var fvar mystruct2
    var fvar2 mystruct
    auto2 := functest();

    functest(functest(fvar))

    fvar = functest(fvar)
    fvar.abc = functest(fvar.str.var1, functest(fvar2.var1));

    pop, nop := fvar.mymethod(fvar2);
    ftg := fvar.abc
    fvar.mymethod();


    fmt.Println(fvar.abc);

    var nmake map[int]func();

    mmake := make(map[rune]mystruct, fvar)

    var sttest interface { Area() int }
}

type Newtype int;

func (i mystruct2) mymethod(s string) (myint int, mybool bool) {
    i.abc;
    s = myint + mybool;
}

type mystruct2 struct { abc rune; str mystruct; }

type mystruct struct { var1 int }
