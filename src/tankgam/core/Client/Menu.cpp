#include "Client/Menu.h"

#include <fmt/format.h>

#include "sys/Renderer.h"

MenuList::MenuList(MenuCallback callback)
    : callback{ std::move(callback) }, currChoice{ 0 }
{
}

MenuList::~MenuList() = default;

MenuList::MenuList(MenuList&& o) noexcept
{
    callback = std::move(o.callback);
    
    choices = std::move(o.choices);
    
    currChoice = o.currChoice;
    o.currChoice = 0;
}

MenuList& MenuList::operator=(MenuList&& o) noexcept
{
    if (&o == this)
    {
        return *this;
    }
    
    callback = std::move(o.callback);
    
    choices = std::move(o.choices);
    
    currChoice = o.currChoice;
    o.currChoice = 0;
    
    return *this;
}

void MenuList::addChoice(std::string choice)
{
    choices.push_back(std::move(choice));
}

void MenuList::nextChoice()
{
    if (currChoice + 1 < choices.size())
    {
        currChoice++;
    }
}

void MenuList::backChoice()
{
    if (currChoice > 0)
    {
        currChoice--;
    }
}

void MenuList::useCurrentChoice()
{
    if (choices.empty())
    {
        throw std::runtime_error{ "Tried to use menu list when there are no choices" };
    }
    
    if (choices.size() < currChoice)
    {
        throw std::runtime_error{ fmt::format("Our current choice is larger than the number of choices we have:\nCurrent Choice: {}, # Choices: {}", currChoice, choices.size()) };
    }
    
    callback(currChoice);
}

Menu::Menu()
    : currList{ 0 }
{
}

Menu::~Menu() = default;

bool Menu::consumeEvent(const Event& ev)
{
    switch (ev.type)
    {
    case EventType::KeyDown:
        switch (static_cast<KeyPressType>(ev.data1))
        {
        case KeyPressType::DownArrow:
            currentList().nextChoice();
            return true;
        case KeyPressType::UpArrow:
            currentList().backChoice();
            return true;
        case KeyPressType::Return:
            currentList().useCurrentChoice();
            return true;
        default:
            return false;
        }
    default:
        return false;
    }
}

size_t Menu::addList(MenuList list)
{
    lists.push_back(std::move(list));
    
    return lists.size() - 1;
}

void Menu::selectList(size_t listNum)
{
    currList = listNum;
}

MenuList& Menu::currentList()
{
    return lists[currList];
}

void Menu::draw(Renderer& renderer)
{
    for (size_t i = 0; i < lists[currList].choices.size(); i++)
    {
        constexpr float SIZE = 32.0f;
        const float x = SIZE * 2.0f;
        const float y = i * SIZE * 1.5f + SIZE * 2.0f;

        const auto& choice = lists[currList].choices[i];
        renderer.drawText(choice, glm::vec2{ x, y }, SIZE);

        if (i == lists[currList].currChoice)
        {
            renderer.drawText(">", glm::vec2{ x - SIZE * 1.25f, y }, SIZE);
        }
    }
}
