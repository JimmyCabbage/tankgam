#pragma once

#include <vector>
#include <string>
#include <functional>

#include "Event.h"

class Renderer;

using MenuCallback = std::function<void(size_t)>;

class MenuList
{
public:
    friend class Menu;
    
    explicit MenuList(MenuCallback callback);
    ~MenuList();
    
    MenuList(MenuList&& o) noexcept;
    MenuList& operator=(MenuList&& o) noexcept;
    
    MenuList(const MenuList&) = delete;
    MenuList& operator=(const MenuList&) = delete;
    
    void addChoice(std::string choice);
    
    void nextChoice();
    
    void backChoice();
    
    void useCurrentChoice();

private:
    MenuCallback callback;
    
    std::vector<std::string> choices;
    
    size_t currChoice;
};

class Menu
{
public:
    explicit Menu();
    ~Menu();
    
    Menu(const Menu&) = delete;
    Menu& operator=(const Menu&) = delete;
    
    bool consumeEvent(const Event& ev);
    
    size_t addList(MenuList list);
    
    void selectList(size_t listNum);
    
    MenuList& currentList();

    void draw(Renderer& renderer);

private:
    std::vector<MenuList> lists;
    
    size_t currList;
};
