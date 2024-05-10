/* testing the correct usage static exceptions.  */
/* { dg-do compile { target c++26 } } */
/* { dg-options "-fstatic-exceptions" } */

template<class, class>
struct same;

template<class T>
struct same<T, T> {};

struct a{};
struct b : a{};

int test_empty_spec() throw() {
  return 1;
}

void helper_throws_int() throw(int) {}
void helper_throws_float() throw(float) {}

int test_empty_spec_throws() throw() {
  helper_throws_int();
  return 1;
} /* { dg-error "Invalid eh spec !!!" } */


int test_valid_throws_int() throw(int) {
  throw 1;
}

int test_mismatched_type() throw(float) {
  test_valid_throws_int();
  return 1;
} /* { dg-error "Invalid eh spec !!!" } */

int test_deduce() throw(auto) {
  throw 1.;
}

int test_check_deduce() throw(double) {
  test_deduce();
  return 1;
}
int test_check_deduce_not_noexcept() throw() {
  test_deduce();
  return 1;
} /* { dg-error "Invalid eh spec !!!" } */

void check_try_valid() noexcept {try {
    helper_throws_float();
}
  catch(float) {}
}

void check_try_invalid() noexcept {try {
    helper_throws_float();
}
  catch(int) {}
}/* { dg-error "Invalid eh spec !!!" } */
void check_try_all() noexcept {try {
    helper_throws_float();
}
  catch(...) {}
}

void check_catch_auto() noexcept {
  try {
    helper_throws_float();
    helper_throws_int();
  }
  catch (auto x) {
	same<decltype(x), int>{};/* { dg-error "invalid use of incomplete type" } */
	same<decltype(x), float>{};/* { dg-error "invalid use of incomplete type" } */
  }
}

void check_throws_derived() throw(b);
void(*check_cast_derived)() throw(a, int) = check_throws_derived;
void(*check_invalid_cast_derived)() throw(int) = check_throws_derived;/* { dg-error "invalid conversion" } */