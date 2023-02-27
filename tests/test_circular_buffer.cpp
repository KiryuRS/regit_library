#include <containers/circular_buffer/include/circular_buffer.hpp>
#include <simple_tester.hpp>

namespace
{
  template <typename T>
  void PrintCircularBuffer(const regit::containers::CircularBuffer<T>& circularBuffer)
  {
    std::cout << "\n";
    for (const auto& elem : circularBuffer)
      std::cout << elem << ' ';
    std::cout << "\n";
  }
}

TEST_BEGIN(Construction)
{
  std::array<int, 5> arr{ 1,2,3,4,5 };
  regit::containers::CircularBuffer<int> cb1(arr.size());
  regit::containers::CircularBuffer<int> cb2{ arr.begin(), arr.end() };
  regit::containers::CircularBuffer<int> cb3{ cb2 };
  regit::containers::CircularBuffer<int> cb4{ std::move(cb3) };
  regit::containers::CircularBuffer<int> cb5{ 1,2,3,4,5 };
  regit::containers::CircularBuffer<unsigned char> cb(1);

  EXPECT_EQ(cb5.capacity(), cb4.size());
  EXPECT_EQ(cb4.back(), cb5.back());
  EXPECT_EQ(cb.front(), 0xCC);
}
TEST_END

TEST_BEGIN(Assignment)
{
  std::array<int, 5> arr{ 1,2,3,4,5 };
  regit::containers::CircularBuffer<int> cb1{ arr.begin(), arr.end() };
  regit::containers::CircularBuffer<int> cb2, cb3, cb4;
  cb2 = cb1;
  cb3 = std::move(cb2);
  cb4 = cb3;
  cb3 = cb4;
  cb4 = std::move(cb3);
  cb4 = cb4;
  EXPECT_EQ(cb4[4], arr.back());
}
TEST_END

TEST_BEGIN(PushEmplacePop)
{
  std::array<int, 5> arr{ -1, 1, 2, 3, 4 };
  regit::containers::CircularBuffer<int> cb(5);
  cb.push(-1);
  cb.emplace(1);
  cb.push(2);
  cb.emplace(3);
  cb.emplace(4);
  cb.emplace(5);
  cb.emplace(6);
  cb.emplace(7);
  cb.pop();
  cb.pop();
  cb.pop();
  cb.emplace(-1);
  cb.push(1);
  cb.emplace(2);
  int* cData = cb.data();

  for (unsigned i = 0; i != 5; ++i)
    EXPECT_EQ(cData[i], arr[i]);
}
TEST_END

TEST_BEGIN(Iterators)
{
  std::string str1, str2, str3;
  regit::containers::CircularBuffer<char> cb(7);
  cb.emplace('A');
  cb.emplace('B');
  cb.emplace('C');
  cb.emplace('D');
  cb.emplace('E');
  cb.emplace('F');
  cb.emplace('G');

  for (auto begin = cb.cbegin(), end = cb.cend(); begin != end; ++begin)
    str1 += *begin;
  for (auto begin = cb.crbegin(), end = cb.crend(); begin != end; ++begin)
    str2 += *begin;
  std::reverse(str2.begin(), str2.end());

  for (const auto& elem : cb)
    str3 += elem;
  auto rIter = cb.rbegin();
  ++rIter;
  auto iter = cb.begin();
  iter = iter + 6;
  --iter;

  EXPECT_EQ(str1, str2);
  EXPECT_EQ(str1, str3);
  EXPECT_EQ(*rIter, *iter);
}
TEST_END

TEST_BEGIN(Subscript)
{
  std::array<int, 10> arr{ 1,2,3,4,5,6,7,8,9,10 };
  regit::containers::CircularBuffer<int> cb1{ arr.begin(), arr.end() };
  for (unsigned i = 1; i != 10; ++i)
    cb1[i] += cb1[i - 1];

  bool success = false;
  // TODO: Convert to EXPECT_THROW
  try
  {
    success = cb1[999] == 55;           // should invoke a std::out_of_range error
  } catch (const std::out_of_range& e)
  {
    success = cb1[9] == 55;
  }

  EXPECT_TRUE(success);
}
TEST_END

TEST_BEGIN(StlAlgorithm)
{
  std::array<int, 10> arr{ 1,2,3,4,5,6,7,8,9,10 };

  regit::containers::CircularBuffer<int> cb{ arr.begin(), arr.end() };
  int sum = 0;
  std::for_each(cb.begin(), cb.end(), [&sum](int value){ sum += value; });
  sum %= sum;
  std::fill(cb.begin(), cb.end(), 1);

  regit::containers::CircularBuffer<int> anotherCB(cb.size());
  std::copy(cb.begin(), cb.end(), anotherCB.begin());
  auto iter = std::find(anotherCB.begin(), anotherCB.end(), sum);

  EXPECT_EQ(iter, anotherCB.end());
}
TEST_END

TEST_BEGIN(Resize)
{
   std::array<int, 5> arr{ 1,2,3,4,5 };
  regit::containers::CircularBuffer<int> cb{ arr.begin(), arr.end() };
  cb.resize(10);
  cb.resize(20);
  cb.resize(7);
  cb.resize(6);
  cb.resize(1);
  cb.resize(3);
  cb.resize(5);
  EXPECT_EQ(cb.front(), cb.back());
}
TEST_END

int main(void)
{
  AddTestResize();
  AddTestStlAlgorithm();
  AddTestSubscript();
  AddTestIterators();
  AddTestPushEmplacePop();
  AddTestAssignment();
  AddTestConstruction();
  regit::testing::RunAllTests();
}