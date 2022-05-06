/*
* Solving Atomix with pattern databases
* Copyright (c) 2016 Alex Gliesch, Marcus Ritt
*
* Permission is hereby granted, free of charge, to any person (the "Person")
* obtaining a copy of this software and associated documentation files (the
* "Software"), to deal in the Software, including the rights to use, copy, modify,
* merge, publish, distribute the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* 1. The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
* 2. Under no circumstances shall the Person be permitted, allowed or authorized
*    to commercially exploit the Software.
* 3. Changes made to the original Software shall be labeled, demarcated or
*    otherwise identified and attributed to the Person.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once
#include <random>
#include <limits>
#include <type_traits>
#include <algorithm>
#include <cassert>
#include <iostream>

template<typename RandomEngine = std::mt19937>
class RandomNumberGenerator {
public:
    RandomNumberGenerator() : engine_(device_()) { }

	RandomNumberGenerator(typename RandomEngine::result_type initialSeed) 
		: engine_(initialSeed) {
	}

    void randomize() {
        engine_.seed(device_());
    }

	void seed(typename RandomEngine::result_type value) {
		engine_.seed(value);
	}

  template<typename T>
  typename std::enable_if<std::is_integral<T>::value, T>::type
      generate(T from, T to) {
      static std::uniform_int_distribution<T> d;
      using parm_t = typename decltype(d)::param_type;
      return d(engine_, parm_t {from, to});
  }

  template<typename T>
  typename std::enable_if<std::is_floating_point<T>::value, T>::type
      generate(T from, T to) {
      static std::uniform_real_distribution<T> d;
      using parm_t = typename decltype(d)::param_type;
      return d(engine_, parm_t {from, to});
  }

  static RandomNumberGenerator& instance() {
      static RandomNumberGenerator rng;
      return rng;
  }

	RandomEngine& engine() { return engine_; }
	std::random_device& device() { return device_; }

private:
    std::random_device device_;
    RandomEngine engine_;
};

template<typename T>
inline T random_number(T from = std::numeric_limits<T>::min(),
                      T to = std::numeric_limits<T>::max()) {
    return RandomNumberGenerator<>::instance().generate(from, to);
}

template<typename StringType>
inline StringType random_string(int size, const StringType& range) {    
    int rangeSize = range.size();
    StringType s;
    if (rangeSize > 0) {
        s.resize(size);
        std::generate_n(std::begin(s), size, [&](){ 
            return range[random_number(0, rangeSize - 1)];
        });
    }     
    return s;
}
