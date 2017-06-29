#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/include/map.hpp>
#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>

namespace fs = boost::fusion;

namespace Recipes
{
void brewCoffee()
{
    std::cout << "dripping coffee through filter\n";
}

int amountWaterMlForCoffee()
{
    return 150;
}

void brewTea()
{
    std::cout << "steeping tea\n";
}

int amountWaterMlForTea()
{
    return 200;
}
}

struct brew {};
struct amountWaterMl {};

using Recipe = fs::map <
               fs::pair<brew, std::function<void()>>,
               fs::pair<amountWaterMl, std::function<int()> >>;

namespace CaffeineBeverage
{
void make(Recipe const& recipe)
{
    std::cout << "\nboiling " << fs::at_key<amountWaterMl>(recipe)() <<
              "ml water\n";
    fs::at_key<brew>(recipe)();
    std::cout << "pur in cup\n\n";
}
}

struct make {};
using Beverage = fs::map<fs::pair<make, std::function<void()>>>;

using Beverages = std::vector<Beverage>;

int main()
{
    std::cout << "coffee machine as bot\n";

    Beverages prototypes =
    {
        {
            fs::make_pair<make>([]()
            {
                CaffeineBeverage::make(
                {
                    fs::make_pair<brew>(&Recipes::brewCoffee),
                    fs::make_pair<amountWaterMl>(&Recipes::amountWaterMlForCoffee)
                });
            })
        },
        {
            fs::make_pair<make>([]()
            {
                CaffeineBeverage::make(
                {
                    fs::make_pair<brew>(&Recipes::brewTea),
                    fs::make_pair<amountWaterMl>(&Recipes::amountWaterMlForTea)
                });
            })
        }
    };

    std::for_each(std::begin(prototypes), std::end(prototypes),
                  [](Beverage const & beverage)
    {
        fs::at_key<make>(beverage)();
    });

    return 0;
}
