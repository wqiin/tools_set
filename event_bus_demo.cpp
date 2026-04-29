#include "dexode/EventBus.hpp"


/**
 * @brief Sample code to play with!
 */

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using EventBus = dexode::EventBus;
using Listener = dexode::EventBus::Listener;

namespace event // Example namespace for events
{
struct Gold // Event that will be proceed when our gold changes
{
    int value = 0;
};
} // namespace event

enum class Monster
{
    Frog,
    Tux,
    Shark
};

class Character
{
public:
    Character(const std::shared_ptr<EventBus>& eventBus)
        : _bus{eventBus}
    {}

    void kill(Monster monsterType)
    {
        if(Monster::Frog == monsterType)
        {
            _gold += 1;
        }
        else if(Monster::Tux == monsterType)
        {
            _gold += 100;
        }
        else if(Monster::Shark == monsterType)
        {
            _gold += 25;
        }
        _bus->postpone(event::Gold{_gold});
    }

private:
    int _gold = 0;
    std::shared_ptr<EventBus> _bus;
};

class UIWallet
{
public:
    UIWallet(const std::shared_ptr<EventBus>& eventBus)
        : _listener{eventBus}
    {}

    void onDraw() // Example "draw" of UI
    {
        std::cout << "Gold:" << _gold << std::endl;
    }

    void onEnter() // We could also do such things in ctor and dtor but a lot of UI has something
    // like this
    {
        _listener.listen<event::Gold>(
            [this](const auto& event) {
                _gold = std::to_string(event.value);
                std::cout << "listener thread id: " << std::this_thread::get_id() << std::endl;
            });
    }

    void onExit()
    {
        _listener.unlistenAll();
    }

private:
    std::string _gold = "0";
    Listener _listener;
};

// Shop button is only enabled when we have some gold (odd decision but for sample good :P)
class ShopButton
{
public:
    ShopButton(const std::shared_ptr<EventBus>& eventBus)
        : _listener{eventBus}
    {
        // We can use lambda or bind your choice
        _listener.listen<event::Gold>(
            std::bind(&ShopButton::onGoldUpdated, this, std::placeholders::_1));
        // Also we use RAII idiom to handle unlisten
    }

    bool isEnabled() const
    {
        return _isEnabled;
    }

private:
    Listener _listener;
    bool _isEnabled = false;

    void onGoldUpdated(const event::Gold& event)
    {
        _isEnabled = event.value > 0;
        std::cout << "Shop button is:" << _isEnabled << std::endl; // some kind of logs

        std::cout << "listener thread id: " << std::this_thread::get_id() << std::endl;
    }
};

void event_bus_demo()
{
    auto eventBus = std::make_shared<EventBus>();

    std::cout << "main thread id: " << std::this_thread::get_id() << std::endl;

    Character characterController{eventBus};

    UIWallet wallet{eventBus}; // UIWallet doesn't know anything about character
    // or even who store gold
    ShopButton shopButton{eventBus}; // ShopButton doesn't know anything about character
    {
        wallet.onEnter();//starting to listen for gold changes
    }

    wallet.onDraw();//pinting gold amount
    {
        characterController.kill(Monster::Tux);// killing monster and getting gold
        eventBus->process();
    }


    wallet.onDraw();//pinting gold amount after killing monster

    // It is easy to test UI eg.
    eventBus->postpone(event::Gold{1});// we can also postpone events and process them later
    eventBus->process();// and check if shop button is enabled
    assert(shopButton.isEnabled() == true);

    eventBus->postpone(event::Gold{0});
    eventBus->process();
    assert(shopButton.isEnabled() == false);

    wallet.onExit();// stopping listen for gold changes
}
