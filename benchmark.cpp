#include "performancecounters/event_counter.h"

#include <random>
#include <vector>
#include <iostream>

event_collector collector;

template <class T>
std::vector<std::pair<T, T>> get_random_pairs(size_t howmany) {
  std::vector<std::pair<T, T>> pairs;
  std::mt19937 gen(42);
  std::uniform_real_distribution<T> dis(1, 100);
  for (size_t i = 0; i < howmany; i++) {
    pairs.emplace_back(dis(gen), dis(gen));
  }
  return pairs;
}


void pretty_print(
    std::string type, size_t number_of_floats, std::string name,
    event_aggregate aggregate) {
  (void)number_of_floats;
  printf(" %32s ", name.c_str());
  printf(" %8.2f ns/%s ", aggregate.elapsed_ns() / number_of_floats, type.c_str());

  printf(" %8.2f instructions/%s ", aggregate.best.instructions() / number_of_floats, type.c_str());
  printf("\n");
  printf(" %32s ", "");
  printf(" %8.2f cycles/%s  ", aggregate.best.cycles() / number_of_floats, type.c_str());
  printf("\n");

  printf(" %32s ", "");
  printf(" %8.2f branches/%s  ", aggregate.best.branches() / number_of_floats, type.c_str());
  printf("\n");

  printf(" %32s ", "");
  printf(" %8.2f branch miss/%s  ", aggregate.best.branch_misses() / number_of_floats, type.c_str());
  printf("\n");
  printf(" %32s ", "");
  printf(" %8.2f instructions/cycle ",
         aggregate.best.instructions() / aggregate.best.cycles());
  printf("\n");

}


template <class T>
void copy_first(const std::vector<std::pair<T, T>> &pairs, std::vector<T> &results) {
  for (size_t i = 0; i < pairs.size(); i++) {
    auto [a, b] = pairs[i];
    results[i] = a;
  }
}


template <class T>
void multiple(const std::vector<std::pair<T, T>> &pairs, std::vector<T> &results) {
  for (size_t i = 0; i < pairs.size(); i++) {
    auto [a, b] = pairs[i];
    results[i] = a*b;
  }
}

template <class T>
void add(const std::vector<std::pair<T, T>> &pairs, std::vector<T> &results) {
  for (size_t i = 0; i < pairs.size(); i++) {
    auto [a, b] = pairs[i];
    results[i] = a + b;
  }
}

template <class T>
void subtract(const std::vector<std::pair<T, T>> &pairs, std::vector<T> &results) {
  for (size_t i = 0; i < pairs.size(); i++) {
    auto [a, b] = pairs[i];
    results[i] = a-b;
  }
}

template <class T>
void divide(const std::vector<std::pair<T, T>> &pairs, std::vector<T> &results) {
  for (size_t i = 0; i < pairs.size(); i++) {
    auto [a, b] = pairs[i];
    results[i] = a / b;
  }
}

template <class T>
void square_root(const std::vector<std::pair<T, T>> &pairs, std::vector<T> &results) {
  for (size_t i = 0; i < pairs.size(); i++) {
    auto [a, b] = pairs[i];
    results[i] = std::sqrt(a);
  }
}

template <class T>
T sum(std::vector<T> &results) {
  T answer = 0;
  for (size_t i = 0; i < results.size(); i++) {
    answer += results[i];
  }
  return answer;
}

template <class T>
event_aggregate time_it_ns(std::vector<std::pair<T, T>> &lines, void (*function)(const std::vector<std::pair<T, T>> &, std::vector<T> &), size_t repeat) {
  event_aggregate aggregate{};

  std::vector<T> results(lines.size());
  // warm up the cache:
  for (size_t i = 0; i < 10; i++) {
    function(lines, results);
    T ts = sum(results);
    if (ts == 0) {
      printf("bug\n");
    }
  }
  for (size_t i = 0; i < repeat; i++) {
    collector.start();
    function(lines, results);
    event_count allocate_count = collector.end();
    aggregate << allocate_count;
    T ts = sum(results);
    if (ts == 0) {
      printf("bug\n");
    }
  }
  return aggregate;
}


int main(int argc, char **argv) {
  size_t howmany = 1000000;
  size_t repeat = 100;
  if (argc > 1) {
    howmany = std::atoi(argv[1]);
  }
  auto pairs_double = get_random_pairs<double>(howmany);
  auto pairs_float = get_random_pairs<float>(howmany);
  // for (int i = 0; i < 10; i++) {
  //   printf(" %f %f\n", pairs_double[i].first, pairs_double[i].second);
  // }

  pretty_print("double", pairs_double.size(), "double_copy_first",
               time_it_ns(pairs_double, copy_first<double>, repeat));
  pretty_print("double", pairs_double.size(), "double_add",
               time_it_ns(pairs_double, add<double>, repeat));
  pretty_print("double", pairs_double.size(), "double_subtract",
               time_it_ns(pairs_double, subtract<double>, repeat));
  pretty_print("double", pairs_double.size(), "double_multiply",
                time_it_ns(pairs_double, multiple<double>, repeat));
  pretty_print("double", pairs_double.size(), "double_divide",
                time_it_ns(pairs_double, divide<double>, repeat));
  pretty_print("double", pairs_double.size(), "double_square_root",
                time_it_ns(pairs_double, square_root<double>, repeat));

  printf("\n\n");
  pretty_print("float", pairs_float.size(), "float_copy_first",
                time_it_ns(pairs_float, copy_first<float>, repeat));
  pretty_print("float", pairs_float.size(), "float_add",
                time_it_ns(pairs_float, add<float>, repeat));
  pretty_print("float", pairs_float.size(), "float_subtract",
                time_it_ns(pairs_float, subtract<float>, repeat));
  pretty_print("float", pairs_float.size(), "float_multiply",
                time_it_ns(pairs_float, multiple<float>, repeat));
  pretty_print("float", pairs_float.size(), "float_divide",
                time_it_ns(pairs_float, divide<float>, repeat));
  pretty_print("float", pairs_float.size(), "float_square_root",
                time_it_ns(pairs_float, square_root<float>, repeat));    
  if(!collector.has_events()) {
    std::cerr << "I cannot access the performance counters. Make sure you run the program in privileged mode (e.g., sudo) under Linux our macOS/ARM." << std::endl;
    return 1;
  }
  return 0;
}

