#include <simple_tester.hpp>
#include <variant/include/variant_impl.hpp>

TEST_BEGIN(Constructor)
{
  regit::variant::Variant var1;
  regit::variant::Variant var2{'A'};
  EXPECT_EQ(var2.get_type_id(), 0)

  regit::variant::Variant var3{var2};
  EXPECT_EQ(var2, var3)

  regit::variant::Variant var4{std::move(var2)};
  EXPECT_EQ(var4.get_value<char>(), 'A')

  regit::variant::Variant var5{static_cast<long>(100)};
  EXPECT_EQ(var5.get_type_id(), 3)

  const regit::variant::Variant var6{var4};
  EXPECT_EQ(var6.get_value<char>(), 'A')

  std::vector<int> container{1,2,3,4,5};
  regit::variant::Variant var7{container};
  EXPECT_EQ(var7.get_value<std::vector<int>>(), container)

  regit::variant::Variant var8{"Hello World"};
  EXPECT_FALSE(std::strcmp(var8.get_value<const char*>(), "Hello World"))

  var8 = std::string{"Goodbye World"};
  EXPECT_EQ(var8.get_value<std::string>(), std::string{"Goodbye World"})
}
TEST_END

TEST_BEGIN(Assignment)
{
  regit::variant::Variant var1{'B'};
  regit::variant::Variant var2{50};
  var1 = 'C';
  var1 = static_cast<short>(81);
  EXPECT_EQ(var1.get_value<short>(), 81)

  var1 = var2;
  EXPECT_EQ(var1.get_type_id(), 2)
  EXPECT_EQ(var1.get_value<int>(), 50)

  regit::variant::Variant var3{static_cast<long long>(1000)};
  var1 = std::move(var3);
  EXPECT_EQ(var1.get_type_id(), 4)
  EXPECT_EQ(var1.get_value<long long>(), 1000)
}
TEST_END

TEST_BEGIN(Operators)
{
  regit::variant::Variant var1;
  regit::variant::Variant var2{'D'};
  regit::variant::Variant var3{static_cast<short>(10)};
  EXPECT_NEQ(var1, var2)

  regit::variant::Variant var4{var1};
  regit::variant::Variant var5{var2};
  EXPECT_EQ(var1, var4)
  EXPECT_EQ(var2, var5)

  short s{var3};
  EXPECT_EQ(s, 10)
  EXPECT_TRUE(var3)
  EXPECT_FALSE(var1)

  regit::variant::Variant var6{123};
  int i = 123;
  EXPECT_EQ(var6, i)
}
TEST_END

void Foo(int, int) { }
TEST_BEGIN(Containers)
{
  std::vector<int> vec{4,5,6,7};
  int i = 0;
  int* pi = &i;
  std::vector<regit::variant::Variant> container
  {
    'A',
    20,
    static_cast<short>(1),
    vec,
    Foo,
    pi
  };

  EXPECT_EQ(container[0].get_value<char>(), 'A')
  EXPECT_EQ(container[3].get_value<std::vector<int>>(), vec)
  EXPECT_EQ(container[5].get_value<int*>(), pi)

  auto functor = container[4].get_value<void(*)(int, int)>();
  functor(1,2);
}
TEST_END

int main(void)
{
  AddTestConstructor();
  AddTestAssignment();
  AddTestOperators();
  AddTestContainers();
  regit::testing::RunAllTests();
}
