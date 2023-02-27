#pragma once

#include <memory>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <initializer_list>
#include <tuple>

namespace regit::containers {

template <typename T, typename Allocator = std::allocator<T>>
class CircularBuffer
{
  static constexpr unsigned char INVALID = 0xCC;

  Allocator mAlloc;
  size_t mCap;
  T* mBuffer;
  T* mStart, *mEnd;

public:

  template <typename T1>
  class Iterator
  {
    T1* mIter;

  public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T1;
    using pointer = typename std::remove_reference<T1>::type*;
    using reference = typename std::remove_reference<T1>::type&;

    Iterator(T1 *iter) noexcept
      : mIter{ iter }
    { }

    reference operator*() noexcept
    {
      return *mIter;
    }

    value_type operator*() const noexcept
    {
      return *mIter;
    }

    pointer operator->() const noexcept
    {
      return mIter;
    }

    bool operator==(const Iterator& rhs) const noexcept
    {
      return mIter == rhs.mIter;
    }

    bool operator!=(const Iterator& rhs) const noexcept
    {
      return !operator==(rhs);
    }

    Iterator& operator++() noexcept
    {
      ++mIter; return *this;
    }

    Iterator operator++(int) noexcept
    {
      Iterator tmp{ *this };
      ++mIter;
      return tmp;
    }

    Iterator& operator--() noexcept
    {
      --mIter;
      return *this;
    }

    Iterator operator--(int) noexcept
    {
      Iterator tmp{ *this };
      --mIter;
      return tmp;
    }

    Iterator operator+(size_t pos) const noexcept
    {
      return mIter + pos;
    }

    difference_type operator-(const Iterator& rhs) const noexcept
    {
      return mIter - rhs.mIter;
    }

    Iterator operator-(size_t pos) const noexcept
    {
      return mIter - pos;
    }
  };

  template <typename T1>
  class ReverseIterator
  {
    T1* mCurr, *mPrev;

  public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T1;
    using pointer = typename std::remove_reference<T1>::type*;
    using reference = typename std::remove_reference<T1>::type&;

    ReverseIterator(T1 *iter) noexcept
      : mCurr{ iter }, mPrev{ std::prev(iter) }
    { }

    reference operator*() noexcept
    {
      return *mPrev;
    }

    value_type operator*() const noexcept
    {
      return *mPrev;
    }

    pointer operator->() const noexcept
    {
      return mPrev;
    }

    bool operator==(const ReverseIterator& rhs) const noexcept
    {
      return std::tie(mCurr, mPrev) == std::tie(rhs.mCurr, rhs.mPrev);
    }

    bool operator!=(const ReverseIterator& rhs) const noexcept
    {
      return !operator==(rhs);
    }

    ReverseIterator& operator++() noexcept
    {
      mCurr = mPrev;
      mPrev = std::prev(mPrev);
      return *this;
    }

    ReverseIterator operator++(int) noexcept
    {
      ReverseIterator tmp{ *this };
      ++*this;
      return tmp;
    }

    ReverseIterator& operator--() noexcept
    {
      mPrev = mCurr;
      mCurr = std::next(mCurr);
      return *this;
    }

    ReverseIterator operator--(int) noexcept
    {
      ReverseIterator tmp{ *this };
      --*this;
      return tmp;
    }

    ReverseIterator operator+(size_t pos) const noexcept
    {
      ReverseIterator tmp{ *this };
      tmp.mCurr = tmp.mCurr - pos;
      tmp.mPrev = std::prev(tmp.mCurr);
      return tmp;
    }

    ReverseIterator operator-(size_t pos) const noexcept
    {
      ReverseIterator tmp{ *this };
      tmp.mCurr = tmp.mCurr + pos;
      tmp.mPrev = std::prev(tmp.mCurr);
      return tmp;
    }

    difference_type operator-(const ReverseIterator& rhs) const noexcept
    {
      return mCurr - rhs.mCurr;
    }

    Iterator<T1> base() const noexcept
    {
      return mCurr;
    }
  };

  explicit CircularBuffer(size_t size = 1)
    : mAlloc{ }, mCap{ size }, mBuffer{ mAlloc.allocate(size) },
      mStart{ mBuffer }, mEnd{ mStart }
  {
    memset(mBuffer, INVALID, sizeof(T) * mCap);
  }

  CircularBuffer(const CircularBuffer& rhs)
    : mAlloc{ rhs.mAlloc }, mCap{ rhs.mCap }, mBuffer{ mAlloc.allocate(rhs.mCap) },
      mStart{ mBuffer + (rhs.mStart - rhs.mBuffer) }, mEnd{ mBuffer + (rhs.mEnd - rhs.mBuffer) }
  {
    memcpy(mBuffer, rhs.mBuffer, sizeof(T) * rhs.mCap);
  }

  CircularBuffer(CircularBuffer&& rhs) noexcept
    : mAlloc{ rhs.mAlloc }, mCap{ rhs.mCap }, mBuffer{ rhs.mBuffer },
      mStart{ rhs.mStart }, mEnd{ rhs.mEnd }
  {
    rhs.mStart = nullptr;
    rhs.mEnd = nullptr;
    rhs.mCap = 0;
    rhs.mBuffer = nullptr;
  }

  template <typename InputIt>
  CircularBuffer(InputIt _begin, InputIt _end)
    : mAlloc{ }, mCap{ static_cast<size_t>(_end - _begin) },
      mBuffer{ mAlloc.allocate(mCap) }, mStart{ mBuffer }, mEnd{ mBuffer + mCap - 1 }
  {
    std::uninitialized_copy(_begin, _end, mBuffer);
  }

  CircularBuffer(std::initializer_list<T> il)
    : CircularBuffer{ il.begin(), il.end() }
  { }

  CircularBuffer& operator=(const CircularBuffer& rhs)
  {
    Allocator tmpAlloc = rhs.mAlloc;
    T* tmp = mAlloc.allocate(rhs.mCap);
    memcpy(tmp, rhs.mBuffer, sizeof(T) * rhs.mCap);
    size_t start_pos = mBuffer - mStart;
    size_t end_pos = mBuffer - mEnd;
    mAlloc.deallocate(mBuffer, mCap);

    mAlloc = rhs.mAlloc;
    mBuffer = tmp;
    mStart = mBuffer + start_pos;
    mEnd = mBuffer + end_pos;
    mCap = rhs.mCap;

    return *this;
  }

  CircularBuffer& operator=(CircularBuffer&& rhs) noexcept
  {
    std::swap(mBuffer, rhs.mBuffer);
    std::swap(mCap, rhs.mCap);
    std::swap(mStart, rhs.mStart);
    std::swap(mEnd, rhs.mEnd);
    std::swap(mAlloc, rhs.mAlloc);

    return *this;
  }

  ~CircularBuffer()
  {
    clear();
    mAlloc.deallocate(mBuffer, mCap);
    mBuffer = nullptr;
  }

  void clear()
  {
    T* end = mBuffer + mCap;
    T* iter = mStart;
    while (iter != mEnd)
    {
      iter->~T();
      iter = std::next(iter) == end ? mBuffer : std::next(iter);
    }
    mEnd = mBuffer;
    mStart = mBuffer;
    memset(mBuffer, INVALID, sizeof(T) * mCap);
  }

  size_t size() const noexcept
  {
    if (mStart - mEnd == 0)
      return *reinterpret_cast<unsigned char*>(mStart) != INVALID;

    if (mStart - mEnd < 0)
      return  mEnd - mStart + 1;

    if (std::prev(mStart) == mEnd)
      return mCap;

    return mCap - (mStart - mEnd);
  }

  void resize(size_t sz)
  {
    T* tmp = mAlloc.allocate(sz);
    if (mCap <= sz)
    {
      memcpy(tmp, mBuffer, sizeof(T) * mCap);
      if (sz - mCap)
          memset(tmp + mCap, INVALID, sizeof(T) * (sz - mCap));
      size_t start_pos = mStart - mBuffer;
      size_t end_pos = mEnd - mBuffer;
      mStart = tmp + start_pos;
      mEnd = tmp + end_pos;
    }
    else
    {
      memcpy(tmp, mBuffer, sizeof(T) * sz);
      size_t start_pos = mStart - mBuffer;
      size_t curr_size = size();
      mStart = tmp + start_pos % sz;
      mEnd = mStart + curr_size % sz;
    }
    mAlloc.deallocate(mBuffer, mCap);
    mBuffer = tmp;
    mCap = sz;
  }

  bool empty() const noexcept
  {
    return mStart == mEnd && *reinterpret_cast<unsigned char*>(mStart) == INVALID;
  }

  size_t capacity() const noexcept
  {
    return mCap;
  }

  T operator[](unsigned index) const
  {
    if (index >= mCap)
      throw std::out_of_range{ "array out of bounds!" };
    T* iter = mStart + index;
    T* end = mBuffer + mCap;
    long int offset = iter - end;
    if (offset < 0)
      return *iter;
    return *(mBuffer + static_cast<unsigned>(offset));
  }

  T& operator[](unsigned index)
  {
    if (index >= mCap)
      throw std::out_of_range{ "array out of bounds!" };
    T* iter = mStart + index;
    T* end = mBuffer + mCap;
    long int offset = iter - end;
    if (offset < 0)
      return *iter;
    return *(mBuffer + static_cast<unsigned>(offset));
  }

  void push(const T& value)
  {
    if (empty())
      *mStart = value;
    else
    {
      T* end = mBuffer + mCap;
      mEnd = std::next(mEnd) == end ? mBuffer : std::next(mEnd);
      *mEnd = value;
      if (mEnd == mStart)
        mStart = std::next(mStart) == end ? mBuffer : std::next(mStart);
    }
  }

  template <typename ... Args>
  void emplace(Args&& ... args)
  {
    if (empty())
      new (mStart) T{ std::forward<Args>(args)... };
    else
    {
      T* end = mBuffer + mCap;
      mEnd = std::next(mEnd) == end ? mBuffer : std::next(mEnd);
      new (mEnd) T{ std::forward<Args>(args)... };
      if (mEnd == mStart)
        mStart = std::next(mStart) == end ? mBuffer : std::next(mStart);
    }
  }

  void pop()
  {
    mEnd->~T();
    memset(mEnd, INVALID, sizeof(T));
    if (mEnd == mStart)
      return;
    mEnd = mEnd == mBuffer ? mBuffer + mCap - 1 : std::prev(mEnd);
  }

  T at(unsigned index) const
  {
    return operator[](index);
  }

  T& at(unsigned index)
  {
    return operator[](index);
  }

  T back() const noexcept
  {
    return *mEnd;
  }

  T& back() noexcept
  {
    return *mEnd;
  }

  T front() const noexcept
  {
    return *mStart;
  }

  T& front() noexcept
  {
    return *mStart;
  }

  T* data() const noexcept
  {
    return mBuffer;
  }

  Allocator get_allocator() const noexcept
  {
    return mAlloc;
  }

  bool operator==(const CircularBuffer& rhs) const noexcept
  {
    return std::tie(mBuffer, mStart, mEnd, mCap) == std::tie(rhs.mBuffer, rhs.mStart, rhs.mEnd, rhs.mCap);
  }

  bool operator!=(const CircularBuffer& rhs) const noexcept
  {
    return !operator==(rhs);
  }

  void swap(CircularBuffer& rhs)
  {
    CircularBuffer tmp{ *this };
    *this = std::move(rhs);
    rhs = std::move(tmp);
  }

  Iterator<T> begin() const noexcept
  {
    return mBuffer;
  }

  Iterator<T> end() const noexcept
  {
    return mBuffer + mCap;
  }

  Iterator<const T> cbegin() const noexcept
  {
    return mBuffer;
  }

  Iterator<const T> cend() const noexcept
  {
    return mBuffer + mCap;
  }

  ReverseIterator<T> rbegin() const noexcept
  {
    return mBuffer + mCap;
  }

  ReverseIterator<T> rend() const noexcept
  {
    return mBuffer;
  }

  ReverseIterator<const T> crbegin() const noexcept
  {
    return mBuffer + mCap;
  }

  ReverseIterator<const T> crend() const noexcept
  {
    return mBuffer;
  }

  using value_type = T;
  using allocator_type = Allocator;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;
  using iterator = Iterator<T>;
  using const_iterator = Iterator<const T>;
  using reverse_iterator = ReverseIterator<T>;
  using const_reverse_iterator = ReverseIterator<const T>;
};

}
