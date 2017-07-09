#include <boost/signals2.hpp>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace coffee_machine::v1
{
namespace recipe
{
class recipe
{
public:
    virtual int amount_water_ml() = 0;
    virtual int powder_gramm() = 0;
    virtual ~recipe() noexcept = default;
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
namespace order
{
class order
{
public:
    virtual void execute() = 0;
    virtual ~order() noexcept = default;
};
class beverage : public order
{
public:
    beverage(v1::beverage::beverage* beverage)
        : _beverage{beverage}
    {}
    virtual ~beverage()
    {
        delete _beverage;
    }
    void execute() override
    {
        _beverage->prepare();
    }
private:
    v1::beverage::beverage* _beverage;
};
}
namespace factory
{
class beverage
{
public:
    virtual ~beverage() noexcept = default;
    virtual v1::beverage::beverage* create() = 0;
};
class coffee : public beverage
{
public:
    v1::beverage::beverage* create() override
    {
        return new v1::beverage::beverage(new recipe::coffee());
    }
};
class tea : public beverage
{
public:
    v1::beverage::beverage* create() override
    {
        return new v1::beverage::beverage(new recipe::tea());
    }
};
class orders
{
public:
    orders()
    {
        _factories["coffee"] = new factory::coffee();
        _factories["tea"   ] = new factory::tea();
    }
    ~orders()
    {
        for(auto it = _factories.begin(); it != _factories.end(); ++it)
        {
            delete it->second;
            it->second = nullptr;
        }
        _factories.clear();
    }
    order::order* create(const std::string& beverage)
    {
        return new order::beverage(_factories[beverage]->create());
    }
    void add(factory::beverage* factory, const std::string name)
    {
        _factories.insert(std::make_pair(name, factory));
    }
private:
    std::map<std::string, factory::beverage*> _factories;
};
}
class order_state_observer
{
public:
    virtual void started(int num_of_orders) = 0;
    virtual void progress(int in_percent) = 0;
    virtual void finished() = 0;
    virtual ~order_state_observer() noexcept = default;
};
class coffee_machine
{
public:
    void request(order::order* order)
    {
        _orders.push_back(order);
    }
    void start()
    {
        const auto num_orders{std::size(_orders)};
        this->notify_start(num_orders);

        int counter{};
        for(auto it{std::begin(_orders)}; it != std::end(_orders); ++it, ++counter)
        {
            (*it)->execute();
            delete(*it);
            (*it) = nullptr;
            this->notify_progress(((float)++counter / num_orders) * 100);
        }

        _orders.clear();
        this->notify_finished();
    }
    void add_order_state_observer(order_state_observer* observer)
    {
        _order_state_observers.push_back(observer);
    }
private:
    void notify_start(int num_of_orders)
    {
        for(auto it{std::begin(_order_state_observers)};
                it != std::end(_order_state_observers);
                ++it)
        {
            (*it)->started(num_of_orders);
        }
    }
    void notify_progress(int percent)
    {
        for(auto it{std::begin(_order_state_observers)};
                it != std::end(_order_state_observers);
                ++it)
        {
            (*it)->progress(percent);
        }
    }
    void notify_finished()
    {
        for(auto it{std::begin(_order_state_observers)};
                it != std::end(_order_state_observers);
                ++it)
        {
            (*it)->finished();
        }
    }
    using orders = std::vector<order::order*>;
    orders _orders;
    using order_state_observers = std::vector<order_state_observer*>;
    order_state_observers _order_state_observers;
};
namespace view
{
class console : public order_state_observer
{
public:
    void started(int num_of_orders) override
    {
        std::cout << "started preparing " << num_of_orders << " orders\n";
    }
    void progress(int in_percent) override
    {
        std::cout << "running " << in_percent << " %\n";
    }
    void finished() override
    {
        std::cout << "finished\n";
    }
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
const recipe coffee
{
    []()
    {
        return 150;
    },
    []()
    {
        return 100;
    }
};
const recipe tea
{
    []()
    {
        return 200;
    },
    []()
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
    beverage(const recipe::recipe& recipe)
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
namespace order
{
using order = std::function<void()>;
}
namespace factory
{
class orders
{
public:
    orders()
    {
        _factories["coffee"] = []()
        {
            beverage::beverage{recipe::coffee} .prepare();
        };
        _factories["tea"   ] = []()
        {
            beverage::beverage{recipe::tea   } .prepare();
        };
    }
    order::order create(const std::string& beverage)
    {
        return _factories[beverage];
    }
    void add(order::order order_func, const std::string name)
    {
        _factories.insert(std::make_pair(name, order_func));
    }
private:
    std::map<std::string, order::order> _factories;
};
}
using namespace boost::signals2;
class coffee_machine
{
public:
    template<class func_sig>
    using signal = typename
                   signal_type<func_sig, keywords::mutex_type<dummy_mutex>>::type;

    signal<void(int)> sig_started;
    signal<void(int)> sig_progress;
    signal<void()> sig_finished;

    void request(order::order order)
    {
        _orders.push_back(order);
    }
    void start()
    {
        const auto num_orders{std::size(_orders)};
        sig_started(num_orders);

        auto counter{0};
        for(auto && o : _orders)
        {
            o();
            sig_progress(((float)++counter / num_orders) * 100);
        }
        _orders.clear();
        sig_finished();
    }
private:
    using orders = std::vector<order::order>;
    orders _orders;
};
namespace view
{
namespace console
{
void started(int num_of_orders)
{
    std::cout << "started preparing " << num_of_orders << " orders\n";
}
void progress(int in_percent)
{
    std::cout << "running " << in_percent << " %\n";
}
void finished()
{
    std::cout << "finished\n";
}
}
}
}
namespace coffee_machine::v3
{
}

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(v1_recipes)
{
    using namespace coffee_machine::v1;

    recipe::recipe* coffee_recipe{new recipe::coffee{}};
    recipe::recipe*    tea_recipe{new recipe::tea   {}};

    beverage::beverage* coffee{new beverage::beverage{coffee_recipe}};
    beverage::beverage*    tea{new beverage::beverage{   tea_recipe}};

    using beverages = std::vector<beverage::beverage*>;
    beverages bs{coffee, tea};

    for(auto && b : bs) b->prepare();
    bs.clear();

    struct mock_recipe : public recipe::recipe
    {
        int amount_water_ml() override
        {
            water_called = true;
            return 0;
        }
        int powder_gramm() override
        {
            powder_called = true;
            return 0;
        }
        bool water_called{false};
        bool powder_called{false};
    };
    auto* mocked_recipe{new mock_recipe{}};
    beverage::beverage mocked_beverage{mocked_recipe};

    mocked_beverage.prepare();

    BOOST_CHECK(mocked_recipe->water_called);
    BOOST_CHECK(mocked_recipe->powder_called);
}
BOOST_AUTO_TEST_CASE(v1_orders)
{
    using namespace coffee_machine::v1;

    coffee_machine::v1::coffee_machine c{};

    auto t{new recipe::tea{}};
    auto tea{new beverage::beverage{t}};

    c.request(new order::beverage{tea});
    c.start();

    struct mock_order : public order::order
    {
        void execute() override
        {
            _called = true;
        }
        bool _called{false};
    };

    auto o{new mock_order{}};
    c.request(o);
    c.start();

    // o is actually already deleted
    BOOST_CHECK(o->_called);
}
BOOST_AUTO_TEST_CASE(v1_order_state_observers)
{
    using namespace coffee_machine::v1;

    {
        coffee_machine::v1::coffee_machine c{};

        auto t{new recipe::tea{}};
        auto tea{new beverage::beverage{t}};

        view::console view{};
        c.add_order_state_observer(&view);

        c.request(new order::beverage{tea});
        c.start();
    }
    {
        struct mock_observer : public order_state_observer
        {
            void started(int num_of_orders) override
            {
                _call_order += "s" + std::to_string(num_of_orders);
            }
            void progress(int in_percent) override
            {
                _call_order += "p" + std::to_string(in_percent);
            }
            void finished() override
            {
                _call_order += "f";
            }
            std::string _call_order{};
        };

        mock_observer o{};
        coffee_machine::v1::coffee_machine c{};
        auto t{new recipe::tea{}};
        auto tea{new beverage::beverage{t}};

        c.add_order_state_observer(&o);
        c.request(new order::beverage{tea});
        c.start();

        BOOST_CHECK_EQUAL(o._call_order, "s1p100f");
    }
}
BOOST_AUTO_TEST_CASE(v1_order_factory)
{
    using namespace coffee_machine::v1;

    {
        coffee_machine::v1::coffee_machine c{};

        factory::orders order_factory{};
        c.request(order_factory.create("coffee"));
        c.request(order_factory.create("tea"));
        c.start();
    }
    {
        struct mock_beverage_factory : public factory::beverage
        {
            coffee_machine::v1::beverage::beverage* create() override
            {
                _called = true;
                return nullptr;
            }
            bool _called{false};
        };

        factory::orders order_factory{};
        mock_beverage_factory* mock = new mock_beverage_factory{};
        order_factory.add(mock, "mock_beverage");

        order_factory.create("mock_beverage");

        BOOST_CHECK(mock->_called);
    }
}
BOOST_AUTO_TEST_CASE(v2_recipes)
{
    using namespace coffee_machine::v2;

    beverage::beverage coffee{recipe::coffee};
    beverage::beverage tea{recipe::tea};

    using beverages = std::vector<beverage::beverage>;
    beverages bs{coffee, tea};

    for(auto && b : bs) b.prepare();

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
BOOST_AUTO_TEST_CASE(v2_orders)
{
    using namespace coffee_machine::v2;

    beverage::beverage coffee{recipe::coffee};
    beverage::beverage tea{recipe::tea};

    coffee_machine::v2::coffee_machine c{};

    c.request([ = ]() mutable { coffee.prepare(); });
    c.request([ = ]() mutable {    tea.prepare(); });

    c.start();

    std::string actual_calls{};

    c.request([&]()
    {
        actual_calls += "o";
    });
    c.request([&]()
    {
        actual_calls += "o";
    });

    c.start();

    BOOST_CHECK("oo" == actual_calls);
}
BOOST_AUTO_TEST_CASE(v2_order_state_observers)
{
    using namespace coffee_machine::v2;
    {
        beverage::beverage coffee{recipe::coffee};
        beverage::beverage tea{recipe::tea};

        coffee_machine::v2::coffee_machine c{};

        c.sig_started .connect(view::console::started);
        c.sig_progress.connect(view::console::progress);
        c.sig_finished.connect(view::console::finished);


        c.request([ = ]() mutable { coffee.prepare(); });
        c.request([ = ]() mutable {    tea.prepare(); });

        c.start();
    }
    {
        coffee_machine::v2::coffee_machine c{};

        std::string actual_calls{};
        c.sig_started .connect([&](auto a)
        {
            actual_calls += "s" + std::to_string(a);
        });
        c.sig_progress.connect([&](auto a)
        {
            actual_calls += "p" + std::to_string(a);
        });
        c.sig_finished.connect([&]()
        {
            actual_calls += "f";
        });


        c.request([]() {});
        c.request([]() {});

        c.start();

        BOOST_CHECK_EQUAL("s2p50p100f", actual_calls);
    }
}
BOOST_AUTO_TEST_CASE(v2_order_factory)
{
    using namespace coffee_machine::v2;
    {
        coffee_machine::v2::coffee_machine c{};
        coffee_machine::v2::factory::orders order_factory{};

        c.request(order_factory.create("coffee"));
        c.request(order_factory.create("tea"));

        c.start();
    }
    {
        coffee_machine::v2::factory::orders order_factory{};
        bool called{false};
        order_factory.add([&]()
        {
            called = true;
        }, "mock_order");

        auto order = order_factory.create("mock_order");
        // order has to be invoked explicitly
        // since function given to order factory
        // is invoked lazily
        order();

        BOOST_CHECK(called);
    }
}
