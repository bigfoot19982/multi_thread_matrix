#include "test_runner.h"
#include "profile.h"
#include <vector>
#include <thread>
#include <future>
using namespace std;

template <typename Iterator>
class IteratorRange {
public:
  IteratorRange(Iterator begin, Iterator end)
    : first(begin)
    , last(end)
    , size_(distance(first, last))
  {
  }

  Iterator begin() const {
    return first;
  }

  Iterator end() const {
    return last;
  }

  size_t size() const {
    return size_;
  }

private:
  Iterator first, last;
  size_t size_;
};

template <typename Iterator>
class Paginator {
private:
  vector<IteratorRange<Iterator>> pages;

public:
  Paginator(Iterator begin, Iterator end, size_t page_size) {
    for (size_t left = distance(begin, end); left > 0; ) {
      size_t current_page_size = min(page_size, left);
      Iterator current_page_end = next(begin, current_page_size);
      pages.push_back({begin, current_page_end});

      left -= current_page_size;
      begin = current_page_end;
    }
  }

  auto begin() const {
    return pages.begin();
  }

  auto end() const {
    return pages.end();
  }

  size_t size() const {
    return pages.size();
  }
};

template <typename C>
auto Paginate(C& c, size_t page_size) {
  return Paginator(begin(c), end(c), page_size);
}

template <typename C>
int64_t calc(C& page, size_t first_row, size_t sz) {
    int64_t sum = 0;
    for (auto& i : page) {
        for (size_t col = 0; col < sz; ++col) {
            sum += i[col];
        }
    }
    return sum;
}

int64_t CalculateMatrixSum(const vector<vector<int>>& matrix) {
    vector<future<int64_t>> tmp;
    size_t sz = matrix.size();
    //unsigned int thr = thread::hardware_concurrency();
    size_t pg_sz = max(1, int(sz/8));
    size_t first_row = 0;
    for (auto& page : Paginate(matrix, pg_sz)) {
        tmp.push_back(
            async([page, first_row, sz] {
                return calc(page, first_row, sz);
            })
        );
        first_row += pg_sz;
    }
    int64_t sum = 0;
    for (auto& i : tmp) sum += i.get();
    return sum;
}

void TestCalculateMatrixSum() {
  const vector<vector<int>> matrix = {
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12},
    {13, 14, 15, 16}
  };
  ASSERT_EQUAL(CalculateMatrixSum(matrix), 136);
}

void TestCalculateMatrixSumGenerated() {
    const vector<vector<int>> matrix(900, vector<int> (900, 1));
    ASSERT_EQUAL(CalculateMatrixSum(matrix), 810000);
}

int main() {
    /*{
        LOG_DURATION("1 or 7");
        TestRunner tr;
        RUN_TEST(tr, TestCalculateMatrixSumGenerated);
    }*/
    TestRunner tr;
    RUN_TEST(tr, TestCalculateMatrixSum);
}
