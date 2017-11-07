# cgo

Cgo(https://golang.org/cmd/cgo/) enables the creation of Go packages that call C code.

## Using cgo with the go command 

To use cgo write normal Go code that imports a pseudo(伪)-package "C".

The Go code can then refer to types such as **C.size_t**, variables such as **C.stdout**, or functions such as **C.putchar**.

---

在import "C"之前的这些注释称为前言，用于引入C相关的头文件，还有其他与编译相关的宏：如CFLAGS, CPPFLAGS, CXXFLAGS, FFLAGS和LDFLAGS

简单的头文件
``` go
// #include <stdio.h>
// #include <errno.h>
import "C"
```
编译选项
``` go
// #cgo CFLAGS: -DPNG_DEBUG=1
// #cgo amd64 386 CFLAGS: -DX86=1
// #cgo LDFLAGS: -lpng
// #include <png.h>
import "C"
import "C"
```
使用pkg-config
``` go
// #cgo pkg-config: png cairo
// #include <png.h>
import "C
```
当然编译选项也可以通过CGO_CFLAGS, CGO_CPPFLAGS, CGO_CXXFLAGS, CGO_FFLAGS和CGO_LDFLAGS，这些环境变量来统一指定。


``` go
/*
int PlusOne(int n)
{
	return n + 1;
}
*/
import "C"
func main() {
	var n int = 10
	var m int = int(C.PlusOne(C.int(n))) // 类型要转换
	fmt.Println(m)                       // 11
}
```
----

前言中还可以包含函数和变量声明，这些函数和变量将视为"C"包的一部分。允许静态函数，不允许静态变量。


* 变量
* 函数
* 结构体
* 联合体
* 回调函数
* 动态链接库

1. 变量

使用C的变量很简单，比方说，要使用int，只要在Go代码中写C.int就可以了。

``` go
package main

import (
	"fmt"
)

import "C"

func main() {
	var n C.int
	n = 5
	fmt.Println(n) // 5

	var m1 int
	// Go不认为C.int与int、int32等类型相同
	// 所以必须进行转换
	m1 = int(n + 3)
	fmt.Println(m1) // 8

	var m2 int32
	m2 = int32(n + 20)
	fmt.Println(m2) // 25
}
```

2. 函数

在Go中调用C的函数也不困难。

``` go
package main

import (
	"fmt"
)

/*
int PlusOne(int n)
{
	return n + 1;
}
*/
import "C"

func main() {
	var n int = 10
	var m int = int(C.PlusOne(C.int(n))) // 类型要转换
	fmt.Println(m)                       // 11
}
```

3. 结构体

``` go
package main

import (
	"fmt"
)

/*
typedef struct _POINT
{
	double x;
	double y;
}POINT;
*/
import "C"

func main() {
	var p C.POINT
	p.x = 9.45
	p.y = 23.12
	fmt.Println(p) // {9.45 23.12}
}
```

4. 联合体

Go中使用C的联合体是比较少见而奇怪的事情，而且稍显麻烦，因为Go将C的联合体视为字节数组。比方说，下面的联合体LARGE_INTEGER被视为[8]byte。


``` c
typedef long LONG;
typedef unsigned long DWORD;
typedef long long LONGLONG;

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    };
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;
```

所以，如果一个C的函数的某个参数的类型为LARGE_INTEGER，我们可以给它一个[8]byte类型的实参，反之亦然。

``` go
package main

import (
	"fmt"
)

/*
typedef long LONG;
typedef unsigned long DWORD;
typedef long long LONGLONG;

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    };
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

void Show(LARGE_INTEGER li)
{
	li.u.LowPart = 1;
	li.u.HighPart = 4;
}
*/
import "C"

func main() {
	var li C.LARGE_INTEGER // 等价于： var li [8]byte
	var b [8]byte = li     // 正确，因为[8]byte和C.LARGE_INTEGER相同
	C.Show(b)              // 参数类型为LARGE_INTEGER，可以接收[8]byte
	li[0] = 75
	fmt.Println(li) // [75 0 0 0 0 0 0 0]
	li[4] = 23
	Test(li) // 参数类型为[8]byte，可以接收C.LARGE_INTEGER
}


func Test(b [8]byte) {
	fmt.Println(b)
}
```


5. 回调函数

有些C函数的参数是回调函数，比方说：
``` c
typedef UINT_PTR(__stdcall* GIRL_PROC)(int);
typedef UINT_PTR(__cdecl* GIRL_PROC_CDECL)(int);

UINT_PTR Func1(int n, GIRL_PROC gp)
{
	if (gp == NULL)
	{
		return 0;
	}
	return (*gp)(n);
}

UINT_PTR Func2(int n, GIRL_PROC_CDECL gp)
{
	if (gp == NULL)
	{
		return 0;
	}
	return (*gp)(n);
}
```

syscall包中有如下两个函数：

syscall.NewCallback

syacall.NewCallbackCDecl

其中，第一个函数接收一个Go函数（这个Go函数的返回值必须只有一个，而且类型为uintptr），并生成一个__stdcall调用约定的C函数，并将生成的函数的地址以uintptr的形式返回；第二个函数的作用与之类似，但生成的函数的调用约定是__cdecl。

一个值得注意的问题是：C的指向函数的指针在Go中被视为*[0]byte，所以要转换一下才能用。这里演示一下__stdcall调用约定的函数的用法，__cdecl类似。

``` go
package main

import (
	"fmt"
	"syscall"
	"unsafe"
)

/*
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef UINT_PTR(__stdcall* GIRL_PROC)(int);
typedef UINT_PTR(__cdecl* GIRL_PROC_CDECL)(int);

UINT_PTR Func1(int n, GIRL_PROC gp)
{
	if (gp == NULL)
	{
		return 0;
	}
	return (*gp)(n);
}

UINT_PTR Func2(int n, GIRL_PROC_CDECL gp)
{
	if (gp == NULL)
	{
		return 0;
	}
	return (*gp)(n);
}
*/
import "C"

func GirlProc(n int32) uintptr {
	return uintptr(n + 97)
}

func main() {
	gp := syscall.NewCallback(GirlProc)
	fmt.Println(gp)
	gop := (*[0]byte)(unsafe.Pointer(gp))
	var t C.UINT_PTR = C.Func1(C.int(29), gop)
	fmt.Println(t) // 126
}
```

6 动态链接库

go是不能使用C++相关的东西的，比如标准库或者C++的面向对象特性。
将c++的功能函数封装成C接口，然后编译成动态库，或者是功能较为简单的可以直接嵌入到go源文件中。 

动态库头文件：**myfuns.h**

``` c
#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

void fun1();

void fun2(int a);

int func3(void **b);

// others
```

动态库名：**myfuns.so**

项目简化结构：

    |-project
    |  |-lib
    |  |  |-myfuns.so
    |  |-include
    |  |  |-myfuns.h
    |  |-src
    |  |  |-main.go
    |  |-pkg
    |  |-bin

go链接动态库：**main.go**

``` go
package main

/*
#cgo CFLAGS : -I../include
#cgo LDFLAGS: -L../lib -lmyfuns

#include "myfuns.h"
*/
import "C"

import (
    "fmt"
)

func main() {
    // 调用动态库函数fun1
    C.fun1()
    // 调用动态库函数fun2
    C.fun2(C.int(4))
    // 调用动态库函数fun3
    var pointer unsafe.Pointer
    ret := C.fun3(&pointer)
    fmt.Println(int(ret))
}
```

通过CFLAGS配置编译选项，通过LDFLAGS来链接指定目录下的动态库。这里需要注意的一个地方就是import "C"是紧挨着注释的，没有空行。