#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

namespace regit::testing {

namespace {

enum ColorCode
{
  FG_BLACK    = 30,
  FG_RED      = 31,
  FG_GREEN    = 32,
  FG_YELLOW   = 33,
  FG_BLUE     = 34,
  FG_MAGENTA  = 35,
  FG_CYAN     = 36,
  FG_WHITE    = 37,
  FG_DEFAULT  = 39,

  BG_BLACK    = 40,
  BG_RED      = 41,
  BG_GREEN    = 42,
  BG_YELLOW   = 43,
  BG_BLUE     = 44,
  BG_MAGENTA  = 45,
  BG_CYAN     = 46,
  BG_WHITE    = 47,
  BG_DEFAULT  = 49,
};

enum TextCode
{
  RESET = 0,
  BOLD = 1,
  UNDERLINE = 4,
  BOLD_BRIGHT_OFF = 21,
  UNDERLINE_OFF = 24
};

class ColorCodeModifier
{
public:
  ColorCodeModifier(TextCode t_code, ColorCode c_code)
    : m_t_code{t_code}
    , m_c_code{c_code}
  {
  }

  friend std::ostream& operator<<(std::ostream& os, const ColorCodeModifier& modifier)
  {
    return os << "\033[" << modifier.m_t_code << ";" << modifier.m_c_code << "m";
  }

private:
  const TextCode m_t_code;
  const ColorCode m_c_code;
};

std::string PrettyPrint(size_t index)
{
  switch (index)
  {
  case 1:
    return "1st";
  case 2:
    return "2nd";
  case 3:
    return "3rd";
  default:
    break;
  }
  return std::to_string(index) + "th";
}

} // anonymous namespace

class Test final
{
private:
  using func_t = std::function<void()>;

  struct TestInfo
  {
    func_t Functor;
    std::vector<bool> Conditions;

    TestInfo() noexcept = default;

    TestInfo(func_t functor, const std::vector<bool>& conditions)
      : Functor{functor}
      , Conditions{conditions}
    {
    }
  };

public:
  template <typename FunctorT>
  bool operator()(FunctorT functor) const noexcept
  {
    try
    {
      functor();
    }
    catch(const std::exception& err)
    {
      std::cout << "Exception occured in test body (" << err.what() << ')' << std::endl;
      return false;
    }
    catch (...)
    {
      std::cout << "Unknown exception occured in test body" << std::endl;
      return false;
    }

    return true;
  }

  template <typename StringT, typename FunctorT>
  void emplace(StringT&& string, FunctorT&& functor)
  {
    TestsHolder.emplace(std::make_pair(
      std::forward<StringT>(string),
      TestInfo{std::forward<FunctorT>(functor), {}}));
  }

  template <typename StringT>
  void add_condition(const StringT& testName, bool expression)
  {
    auto& testInfo = TestsHolder[testName];
    testInfo.Conditions.emplace_back(expression);
  }

  void RunAllTests() const noexcept
  {
    const ColorCodeModifier PASSED_COLOR{TextCode::BOLD, ColorCode::FG_GREEN};
    const ColorCodeModifier FAILED_COLOR{TextCode::BOLD, ColorCode::FG_RED};
    const ColorCodeModifier DEFAULT_COLOR{TextCode::RESET, ColorCode::FG_DEFAULT};

    for (const auto& [name, testInfo] : TestsHolder)
    {
      std::cout << '[' << name << "] ..." << std::endl;
      auto result = operator()(testInfo.Functor);

      auto begin = testInfo.Conditions.begin();
      auto end = testInfo.Conditions.end();
      for (auto iter = begin; iter != end; ++iter)
      {
        if (!*iter)
        {
          result = false;
          auto index = iter - begin + 1;
          std::cout << '[' << name << ']'
            << " failed at " << PrettyPrint(index)
            << " condition" << std::endl;
        }
      }

      std::cout << '[' << name << ']';
      if (result)
        std::cout << PASSED_COLOR << " PASSED";
      else
        std::cout << FAILED_COLOR << " FAILED";
      std::cout << DEFAULT_COLOR << std::endl;
    }
  }

private:
  // TODO: Fix test execution order and remove the need for "AddTest..."
  std::unordered_map<std::string, TestInfo> TestsHolder;
};

inline Test TheTest;

void RunAllTests() noexcept
{
  TheTest.RunAllTests();
}

}

#define TEST_BEGIN(TestName) void AddTest##TestName() { regit::testing::TheTest.emplace(#TestName, [TEST_REGIT_NAME=#TestName] () -> void
#define TEST_END ); }

#define EXPECT_TRUE(expression) regit::testing::TheTest.add_condition(TEST_REGIT_NAME, expression);
#define EXPECT_FALSE(expression) EXPECT_TRUE(!expression)
#define EXPECT_EQ(expression1, expression2) EXPECT_TRUE(expression1 == expression2)
#define EXPECT_NEQ(expression1, expression2) EXPECT_TRUE(expression1 != expression2)
