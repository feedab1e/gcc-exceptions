// { dg-options "-std=gnu++0x" }
// 2008-06-15  Paolo Carlini  <paolo.carlini@oracle.com>

// Copyright (C) 2008 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

// 21.4 Numeric Conversions [string.conversions]

#include <string>
#include <limits>
#include <stdexcept>
#include <testsuite_hooks.h>

void
test01()
{
#ifdef _GLIBCXX_USE_C99

  bool test __attribute__((unused)) = false;
  using namespace std;

  try
    {
      string one;
      stof(one);      
    }
  catch(std::invalid_argument)
    {
      test = true;
    }
  catch(...)
    {
    }
  VERIFY( test );

  test = false;
  try
    {
      string one("a");
      stof(one);      
    }
  catch(std::invalid_argument)
    {
      test = true;
    }
  catch(...)
    {
    }
  VERIFY( test );

  float f1 = 0.0f;
  size_t idx1 = 0;
  try
    {
      string one("2.0a");
      f1 = stof(one, &idx1);      
    }
  catch(...)
    {
      test = false;
    }
  VERIFY( test );
  VERIFY( f1 == 2.0f );
  VERIFY( idx1 == 3 );

  test = false;
  try
    {
      string one("1e");
      one.append(2 * numeric_limits<float>::max_exponent10, '9');
      f1 = stof(one);
    }
  catch(std::out_of_range)
    {
      test = true;
    }
  catch(...)
    {
    }
  VERIFY( test );
  VERIFY( f1 == 2.0f );

  try
    {
      long double ld0 = numeric_limits<float>::max() / 100.0;
      string one(to_string(ld0));
      stof(one);
    }
  catch(...)
    {
      test = false;
    }
  VERIFY( test );

  if (sizeof(float) < sizeof(long double))
    {
      test = false;
      f1 = -1.0f;
      try
	{
	  long double ld1 = numeric_limits<float>::max();
	  ld1 *= 100.0;
	  string one(to_string(ld1));
	  f1 = stof(one);
	}
      catch(std::out_of_range)
	{
	  test = true;
	}
      catch(...)
	{
	}
      VERIFY( test );
      VERIFY( f1 == -1.0f );
    }

#endif
}

int main()
{
  test01();
  return 0;
}
