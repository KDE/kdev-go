package _builtins

/*
The append built-in function appends elements to the end of a slice. If it has sufficient capacity,
the destination is resliced to accommodate the new elements. If it does not, a new underlying array
will be allocated. Append returns the updated slice. It is therefore necessary to store the result
of append, often in the variable holding the slice itself:

slice = append(slice, elem1, elem2)
slice = append(slice, anotherSlice...)

As a special case, it is legal to append a string to a byte slice, like this:

slice = append([]byte("hello "), "world"...)
*/
func append(slice []Type, elems ...Type) []Type {}

/*
The cap built-in function returns the capacity of v, according to its type:

Array: the number of elements in v (same as len(v)).
Pointer to array: the number of elements in *v (same as len(v)).
Slice: the maximum length the slice can reach when resliced;
if v is nil, cap(v) is zero.
Channel: the channel buffer capacity, in units of elements;
if v is nil, cap(v) is zero.

*/
func cap(v Type) int {}

/*
The close built-in function closes a channel, which must be either bidirectional or send-only. It
should be executed only by the sender, never the receiver, and has the effect of shutting down the
channel after the last sent value is received. After the last value has been received from a closed
channel c, any receive from c will succeed without blocking, returning the zero value for the
channel element. The form

x, ok := <-c

will also set ok to false for a closed channel.
*/
func close(c chan<- Type) {}

/*
The complex built-in function constructs a complex value from two floating-point values. The real
and imaginary parts must be of the same size, either float32 or float64 (or assignable to them),
and the return value will be the corresponding complex type (complex64 for float32, complex128 for float64).
*/
func complex(r, i float64) complex128 {}

/*
The copy built-in function copies elements from a source slice into a destination slice. (As a special
case, it also will copy bytes from a string to a slice of bytes.) The source and destination may overlap.
Copy returns the number of elements copied, which will be the minimum of len(src) and len(dst).
*/
func copy(dst, src []Type) int {}

/*
The delete built-in function deletes the element with the specified key (m[key]) from the map. If m is nil
or there is no such element, delete is a no-op.
*/
func delete(m map[Type]Type1, key Type) {}

/*
The imag built-in function returns the imaginary part of the complex number c. The return value will be
floating point type corresponding to the type of c.
*/
func imag(c complex128) float64 {}

/*
 The len built-in function returns the length of v, according to its type:

Array: the number of elements in v.
Pointer to array: the number of elements in *v (even if v is nil).
Slice, or map: the number of elements in v; if v is nil, len(v) is zero.
String: the number of bytes in v.
Channel: the number of elements queued (unread) in the channel buffer;
if v is nil, len(v) is zero.
*/
func len(v Type) int {}

/*
The make built-in function allocates and initializes an object of type slice, map,
or chan (only). Like new, the first argument is a type, not a value. Unlike new,
make's return type is the same as the type of its argument, not a pointer to it.
The specification of the result depends on the type:

Slice: The size specifies the length. The capacity of the slice is
equal to its length. A second integer argument may be provided to
specify a different capacity; it must be no smaller than the
length, so make([]int, 0, 10) allocates a slice of length 0 and
capacity 10.
Map: An initial allocation is made according to the size but the
resulting map has length 0. The size may be omitted, in which case
a small starting size is allocated.
Channel: The channel's buffer is initialized with the specified
buffer capacity. If zero, or the size is omitted, the channel is
unbuffered.
*/
func make(Type, size) Type {}

/*
The new built-in function allocates memory. The first argument is a type, not a value,
and the value returned is a pointer to a newly allocated zero value of that type.
*/
func new(Type) *Type {}

/*
The panic built-in function stops normal execution of the current goroutine. When a
function F calls panic, normal execution of F stops immediately. Any functions whose
execution was deferred by F are run in the usual way, and then F returns to its caller.
To the caller G, the invocation of F then behaves like a call to panic, terminating G's
execution and running any deferred functions. This continues until all functions in
the executing goroutine have stopped, in reverse order. At that point, the program is
terminated and the error condition is reported, including the value of the argument to panic.
This termination sequence is called panicking and can be controlled by the built-in function recover.
*/
func panic(v interface{}) {}

/*
The print built-in function formats its arguments in an implementation- specific way
and writes the result to standard error. Print is useful for bootstrapping and debugging;
it is not guaranteed to stay in the language.
*/
func print(args ...Type) {}

/*
The println built-in function formats its arguments in an implementation- specific way
and writes the result to standard error. Spaces are always added between arguments and
a newline is appended. Println is useful for bootstrapping and debugging; it is not guaranteed to stay in the language.
*/
func println(args ...Type) {}

/*
The real built-in function returns the real part of the complex number c. The return value will be
floating point type corresponding to the type of c.
*/
func real(c complex128) float64 {}

/*
The recover built-in function allows a program to manage behavior of a panicking goroutine.
Executing a call to recover inside a deferred function (but not any function called by it)
stops the panicking sequence by restoring normal execution and retrieves the error value
passed to the call of panic. If recover is called outside the deferred function it will not
stop a panicking sequence. In this case, or when the goroutine is not panicking, or if the
argument supplied to panic was nil, recover returns nil. Thus the return value from recover
reports whether the goroutine is panicking.
*/
func recover() interface {} {}

/*
The error built-in interface type is the conventional interface for representing an error
condition, with the nil value representing no error.

type error interface {
        Error() string
}
*/
type error interface {
    Error() string
}
