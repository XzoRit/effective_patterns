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
    beverage(recipe::recipe& recipe)
        : _recipe{recipe}
    {
    }
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
    recipe::recipe& _recipe;
};
}
}
namespace coffee_machine::v2
{
}
namespace coffee_machine::v3
{
}

#include <boost/test/unit_test.hpp>
#include <fakeit.hpp>

using namespace fakeit;

BOOST_AUTO_TEST_CASE(recipes)
{
    using namespace coffee_machine::v1;

    Mock<recipe::recipe> recipe{};
    Fake(Method(recipe, amount_water_ml));
    Fake(Method(recipe, powder_gramm));

    beverage::beverage b{recipe.get()};

    b.prepare();

    Verify(Method(recipe, amount_water_ml)).Exactly(Once);
    Verify(Method(recipe, powder_gramm)).Exactly(Once);
}
