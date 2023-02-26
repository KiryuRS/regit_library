#include <simple_tester.hpp>
#include <async/include/timer.hpp>

#include <chrono>

using namespace std::chrono_literals;

TEST_BEGIN(IncrementAfterTime)
{
  int counter = 0;
  regit::async::SimplerTimer timer;
  auto incrementer = [&counter] () mutable { ++counter; };

  timer.Post(std::chrono::seconds{1}, incrementer);
  std::this_thread::sleep_for(2s);
  EXPECT_EQ(counter, 1);
}
TEST_END

int main(void)
{
  // TODO: Fix me
  // AddTestIncrementAfterTime();
  regit::testing::RunAllTests();
}