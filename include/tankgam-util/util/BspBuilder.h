#pragma once

#include <memory>
#include <vector>

#include <util/Brush.h>
#include <util/Bsp.h>

class BspBuilder
{
public:
    BspBuilder();
    ~BspBuilder();
    
    BspBuilder(BspBuilder&&) = delete;
    BspBuilder& operator=(BspBuilder&&) = delete;
    
    void addBrushes(std::vector<Brush> brushes);
    
    bsp::File build();
    
private:
    struct Implementation;
    std::unique_ptr<Implementation> pImpl;
};
