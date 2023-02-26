#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

// Compiled with gcc 9.4.0
// Flags: -std=c++17 -pthread -O3

namespace regit::async {

namespace detail
{
  using work_t = std::function<void()>;
  class NaiveThreadWrapper final
  {
  public:
    template <
      typename ... ArgsT,
      // prevent anther instance of NaiveThreadWrapper from calling this constructor
      std::enable_if_t<std::is_constructible_v<std::thread, ArgsT...>, int> = 0>
    NaiveThreadWrapper(ArgsT&& ... work) noexcept
      : m_thread{std::forward<ArgsT>(work)...}
    {
    }

    NaiveThreadWrapper(NaiveThreadWrapper&&) noexcept = default;

    ~NaiveThreadWrapper()
    {
      if (m_thread.joinable())
        m_thread.join();
    }

  private:
    std::thread m_thread;
  };

  class DefaultWorkPolicy
  {
  public:
    void BeginWork(const work_t& work) noexcept
    {
      try
      {
        if (work)
          work();
      }
      catch (const std::exception&)
      {
        // work failed for some reason, but let's just move on
      }
    }
  };

} // detail namespace

  template <typename ThreadT = detail::NaiveThreadWrapper, typename WorkPolicyT = detail::DefaultWorkPolicy>
  class GenericThreadPool final : private WorkPolicyT
  {
  public:
    // Generally thread pools shouldnt be moved or copyable to prevent unecessary undefined behaviors
    GenericThreadPool(const GenericThreadPool&) = delete;
    GenericThreadPool(GenericThreadPool&&) = delete;
    GenericThreadPool& operator=(const GenericThreadPool&) = delete;
    GenericThreadPool& operator=(GenericThreadPool&&) = delete;

    using worker_t = std::function<void()>;
    using thread_factory_t = std::function<ThreadT(worker_t)>;
    using thread_t = ThreadT;

    GenericThreadPool(size_t size) noexcept;
    template <typename ThreadFactoryT>
    GenericThreadPool(size_t size, ThreadFactoryT&& threadFactory) noexcept;
    ~GenericThreadPool();

    void Start();
    void Stop();
    void Post(detail::work_t work);

  private:
    void WorkerFunc();

    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::atomic_bool m_stopping;
    std::queue<detail::work_t> m_jobs;
    std::vector<ThreadT> m_threads;
    thread_factory_t m_threadFactory;

    const size_t m_poolSize;
    std::once_flag m_init_flag, m_deinit_flag, m_ready_flag;
  };

  template <typename ThreadT, typename WorkPolicyT>
  GenericThreadPool<ThreadT, WorkPolicyT>::GenericThreadPool(size_t size) noexcept
    : GenericThreadPool{
        size,
        [](worker_t joinPool) { return ThreadT{std::move(joinPool)}; }}
  {
  }

  template <typename ThreadT, typename WorkPolicyT>
  template <typename ThreadFactoryT>
  GenericThreadPool<ThreadT, WorkPolicyT>::GenericThreadPool(size_t size, ThreadFactoryT&& threadFactory) noexcept
    : m_stopping{false}
    , m_threadFactory{std::forward<ThreadFactoryT>(threadFactory)}
    , m_poolSize{size}
  {
    static_assert(std::is_same_v<ThreadT, std::result_of_t<ThreadFactoryT(std::function<void()>)>>);
  }

  template <typename ThreadT, typename WorkPolicyT>
  GenericThreadPool<ThreadT, WorkPolicyT>::~GenericThreadPool()
  {
    Stop();
  }

  template <typename ThreadT, typename WorkPolicyT>
  void GenericThreadPool<ThreadT, WorkPolicyT>::Start()
  {
    std::call_once(
      m_init_flag,
      [this] ()
      {
        m_threads.resize(m_poolSize);
        for (size_t i = 0; i != m_poolSize; ++i)
          m_threads.emplace_back(m_threadFactory([this] { WorkerFunc(); }));
      });
  }

  template <typename ThreadT, typename WorkPolicyT>
  void GenericThreadPool<ThreadT, WorkPolicyT>::Stop()
  {
    std::call_once(
      m_deinit_flag,
      [this] ()
      {
        m_stopping = true;
        m_condition.notify_all();

        m_threads.clear();
      });
  }

  template <typename ThreadT, typename WorkPolicyT>
  void GenericThreadPool<ThreadT, WorkPolicyT>::Post(detail::work_t work)
  {
    {
      std::lock_guard<std::mutex> lock{m_mutex};
      m_jobs.emplace(std::move(work));
    }
    m_condition.notify_one();
  }

  template <typename ThreadT, typename WorkPolicyT>
  void GenericThreadPool<ThreadT, WorkPolicyT>::WorkerFunc()
  {
    while (!m_stopping)
    {
      detail::work_t work;
      {
        std::unique_lock<std::mutex> lock{m_mutex};
        m_condition.wait(lock, [this] { return m_stopping || !m_jobs.empty(); });

        if (m_stopping)
          break;

        work = m_jobs.front();
        m_jobs.pop();
      }

      WorkPolicyT::BeginWork(work);
    }
  }

  // Class Template Argument Deduction (CTAD)
  // https://en.cppreference.com/w/cpp/language/class_template_argument_deduction
  template <typename ThreadT, typename WorkPolicyT>
  GenericThreadPool(size_t) -> GenericThreadPool<ThreadT, WorkPolicyT>;

} // namespace regit::async
