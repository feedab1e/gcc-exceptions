# Static exception specifications for C++

This repo adds a feature to specify and statically check exception specifications for functions.

This implementation follows [p3166](https://wg21.link/p3166r0), but aims to add typing to the existing C++ exceptions instead of adding a fresh mechanism for exception handling.

## Static exception specifiactions
It brings back the deprecated `throw(<type-list>)` that was deprecated in C++11 and removed in C++17, and gives them a new meaning of statically describing the set of exceptions that is allowed to be thrown.
Unlike the previous implementations of this idea, this feature also introduces deduction for the set of exceptions thrown (via `throw(auto)`), to alleviate the rigidity problem.

For example,
```cpp
auto foo() throw(int);
auto bar() throw(double);
auto deduce() throw(auto) /* deduces as int, double */ {
  foo(), bar();
}

auto catch_double() throw(int) {
  try { deduce(); }
  catch(double) {}
}
```
## Templated handlers
Given this static exception information, we can also use it to instantiate a template handler based on that, used like so:
```cpp
#include <print>
int main(int argc, char *argv[]) {
  try { argc ? throw 1 : throw 2.; }
  catch(auto exn) {
    std::println("the exception was: {}", exn); /* will either be an int or a double */ 
  }
}
```

## Exception inheritance
Of course, in C++, an exception of a derived type can be caught with a handler of a base class, and we account for that, both in handling, and in function pointer conversions.
However, there is a caveat that is related to how the C++ exception handling machinery interacts with multiple inheritance.
In particular, in the following example:
```cpp
struct base;
struct derived1: base {};
struct derived2: base {};
struct derived_final: base {};

void throw_derived() throw(derived_final);

void test_derived() throw() /* statically enforces `noexcept`-ness */ {
  try { throw_derived(); }
  catch (derived_final) {} /* OK */
}
void test_derived1() throw() {
  try { throw_derived(); }
  catch (derived1) {} /* OK */
}
void test_base() throw() {
  try { throw_derived(); }
  catch (base) {} /* Ill-formed */
}
```
The reason this is ill-formed is that there actually are two instances of `base` in `derived_final`, and the exception machinery fails to disambiguate between them, and therefore fails to invoke the handler.

This has implications on function pointer conversions, since if we were to apply the rules from `catch` directly, we would create a type system hole that would allow exceptions to leak:
```cpp
void throw_derived() throw(derived_final);
void (*throw_derived1)() throw(derived1) = throw_derived;

void catch_base() throw() {
  try { throw_derived1(); }
  catch (base) {}
}
```
Here, the cast in `throw_derived1`'s initializer erases the duplicate base information, which in turn means that the resulting exception cannot be reliably caught, so care must be taken that such conversions are forbidden.

## Future work
This implementation is still a proof of concepts and not finished; some parts need additional consideration and work.

### Querying exception information
It would be very convenient to introduce something like `declthrow(<expr>)`, which will evaluate to a pack of thrown exception types.
However, since `declthrow` takes an expression, this implies that these can be nested. The proplem with this is that since `declthrow` evaluates to a pack, this implies that packs can be nested now:
```cpp
auto foo() throw(auto) { ... } 
template<class T>
auto bar() throw(auto) { ... }

template<class ...Ts>
using template_param_sink = int;

using _ = template_param_sink<declthrow(bar<declthrow(foo())>()) ... ...>;
```

This is a completely new situation for C++, since pack expansion could never expand packs into packs, and therefore it needs to be carefully studied before being adopted.

### Optimization of exception machinery
It is possible to completely remove the overhead of allocating an exception and stack unwinding, if the compiler can see both the throw and the catch site:
```cpp
void foo(int val) {
  try { if (val) throw val /* can be optimized into a jump to the handler */; }
  catch(int x) {
    std::println("value is {}!", x);
  }
}
```
Unfortunately, the runitme exception functions are emitted in the frontend, before any optimization pass can notice them.
To solve that, a GIMPLE tuple for allocating and throwing an exception needs to be introduced, and inheritance information needs to be preserved for optimizations.
