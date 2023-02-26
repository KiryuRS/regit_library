#include <simple_tester.hpp>
#include <async/include/thread_pool.hpp>

#include <atomic>
#include <chrono>

using namespace std::chrono_literals;

TEST_BEGIN(OneThread)
{
  int counter = 0;

  auto incrementer = [&counter] () mutable { ++counter; };
  size_t num_threads = 1;
  regit::async::GenericThreadPool thread_pool{num_threads};
  const int expected_increments = 5;

  thread_pool.Start();
  for (int i = 0; i != expected_increments; ++i)
    thread_pool.Post(incrementer);
  std::this_thread::sleep_for(10ms);
  thread_pool.Stop();

  EXPECT_EQ(counter, expected_increments);
}
TEST_END

TEST_BEGIN(MultipleThreads)
{
  std::atomic_int counter = 0;

  auto incrementer = [&counter] () mutable { ++counter; };
  size_t num_threads = 3;
  regit::async::GenericThreadPool thread_pool{num_threads};
  const int expected_increments = 5;

  thread_pool.Start();
  for (int i = 0; i != expected_increments; ++i)
    thread_pool.Post(incrementer);
  std::this_thread::sleep_for(10ms);
  thread_pool.Stop();

  EXPECT_EQ(counter, expected_increments);
}
TEST_END

int main(void)
{
  AddTestOneThread();
  AddTestMultipleThreads();
  regit::testing::RunAllTests();
}