#include <functional>
#include <string>
#include <memory>

namespace coffee_machine::v1
{
namespace recipe
{
class recipe
{
public:
    virtual int amount_water_ml() = 0;
    virtual int powder_gramm() = 0;
    virtual ~recipe() {}
};
class coffee : public recipe
{
    int amount_water_ml() override
    {
        return 150;
    }
    int powder_gramm() override
    {
        return 100;
    }
};
class tea : public recipe
{
    int amount_water_ml() override
    {
        return 200;
    }
    int powder_gramm() override
    {
        return 150;
    }
};
}
namespace beverage
{
class beverage
{
public:
    beverage(recipe::recipe* recipe)
        : _recipe{recipe}
    {
    }
    ~beverage()
    {
        delete _recipe;
    }
    void prepare()
    {
        boil_water(_recipe->amount_water_ml());
        brew(_recipe->powder_gramm());
        pour_into_cup();
    }
private:
    void boil_water(int) {}
    void brew(int) {}
    void pour_into_cup() {}
    recipe::recipe* _recipe;
};
}
}
namespace coffee_machine::v2
{
namespace recipe
{
struct recipe
{
    std::function<int()> amount_water_ml;
    std::function<int()> powder_gramm;
};
}
namespace beverage
{
class beverage
{
public:
    beverage(recipe::recipe recipe)
        : _recipe{recipe}
    {}
    void prepare()
    {
        boil_water(_recipe.amount_water_ml());
        brew(_recipe.powder_gramm());
        pour_into_cup();
    }
private:
    void boil_water(int) {}
    void brew(int) {}
    void pour_into_cup() {}
private:
    recipe::recipe _recipe;
};
}
}
namespace coffee_machine::v3
{
}

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(v1_recipes)
{
    using namespace coffee_machine::v1;
}

BOOST_AUTO_TEST_CASE(v2_recipes)
{
    using namespace coffee_machine::v2;

    std::string actual_calls{};
    beverage::beverage b{{
        [&]()
        {
            actual_calls += "a";
            return 0;
        },
        [&]()
        {
            actual_calls += "b";
            return 0;
        }
    }};

    b.prepare();

    BOOST_CHECK("ab" == actual_calls);
}
