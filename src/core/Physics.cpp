#include "Physics.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <algorithm>

float phys::collision(Box movingBox, Box staticBox, glm::vec2& outNormal)
{
    glm::vec2 invEntry;
    glm::vec2 invExit;
    
    const glm::vec2 mbTopLeft = movingBox.center - glm::vec2{movingBox.width / 2.0f, movingBox.height / 2.0f};
    const glm::vec2 sbTopLeft = staticBox.center - glm::vec2{staticBox.width / 2.0f, staticBox.height / 2.0f};
    
    //find distance between objects on near and far sides for x & y
    if (movingBox.velocity.x > 0.0f)
    {
        invEntry.x = sbTopLeft.x - (mbTopLeft.x + movingBox.width);
        invExit.x = (sbTopLeft.x + staticBox.width) - mbTopLeft.x;
    }
    else
    {
        invEntry.x = (sbTopLeft.x + staticBox.width) - mbTopLeft.x;
        invExit.x = sbTopLeft.x - (mbTopLeft.x + movingBox.width);
    }
    
    if (movingBox.velocity.y > 0.0f)
    {
        invEntry.y = sbTopLeft.y - (mbTopLeft.y + movingBox.height);
        invExit.y = (sbTopLeft.y + staticBox.height) - mbTopLeft.y;
    }
    else
    {
        invEntry.y = (sbTopLeft.y + staticBox.height) - mbTopLeft.y;
        invExit.y = sbTopLeft.y - (mbTopLeft.y + movingBox.height);
    }
    
    //find time of collision and time of leaving for each axis (if statement is to prevent divide by zero)
    glm::vec2 entry;
    glm::vec2 exit;
    
    if (std::abs(movingBox.velocity.x) < 0.01f)
    {
        entry.x = -std::numeric_limits<float>::infinity();
        exit.x = std::numeric_limits<float>::infinity();
    }
    else
    {
        entry.x = invEntry.x / movingBox.velocity.x;
        exit.x = invExit.x / movingBox.velocity.x;
    }
    
    if (std::abs(movingBox.velocity.y) < 0.01f)
    {
        entry.y = -std::numeric_limits<float>::infinity();
        exit.y = std::numeric_limits<float>::infinity();
    }
    else
    {
        entry.y = invEntry.y / movingBox.velocity.y;
        exit.y = invExit.y / movingBox.velocity.y;
    }
    
    //find the earliest/latest times of collisionfloat
    const float entryTime = std::max(entry.x, entry.y);
    const float exitTime = std::min(exit.x, exit.y);
    
    //if there was no collision
    if (entryTime > exitTime || (entry.x < 0.0f && entry.y < 0.0f) ||
        entry.x > 1.0f || entry.y > 1.0f)
    {
        outNormal = {};
        return 1.0f;
    }
    //there was a collision
    else
    {
        //calculate normal of collided surface
        if (entry.x > entry.y)
        {
            if (invEntry.x < 0.0f)
            {
                outNormal.x = 1.0f;
                outNormal.y = 0.0f;
            }
            else
            {
                outNormal.x = -1.0f;
                outNormal.y = 0.0f;
            }
        }
        else
        {
            if (invEntry.y < 0.0f)
            {
                outNormal.x = 0.0f;
                outNormal.y = 1.0f;
            }
            else
            {
                outNormal.x = 0.0f;
                outNormal.y = -1.0f;
            }
        }
    }
    
    return entryTime;
}

float phys::collision(Box box, Segment seg, glm::vec2& outNormal)
{
    const glm::vec2 segDir = seg.end - seg.begin;
    glm::vec2 segMin{};
    glm::vec2 segMax{};
    
    if (segDir.x > 0.0f) //right
    {
        segMin.x = seg.begin.x;
        segMax.x = seg.end.x;
    }
    else //left
    {
        segMin.x = seg.end.x;
        segMax.x = seg.begin.x;
    }
    
    if (segDir.y > 0.0f) //up
    {
        segMin.y = seg.begin.y;
        segMax.y = seg.end.y;
    }
    else //down
    {
        segMin.y = seg.end.y;
        segMax.y = seg.begin.y;
    }
    
    //outNormal = glm::normalize(glm::vec2{segDir.y, -segDir.x});
    outNormal = seg.normal;
    
    float hitTime = 0.0f;
    float outTime = 1.0f;
    
    float radius = box.width * std::abs(seg.normal.x) + box.height * std::abs(seg.normal.y); //radius to segment
    const float boxProj = glm::dot(seg.begin - box.center, seg.normal); //projected segment distance to box
    const float velProj = glm::dot(box.velocity, seg.normal); //projected velocity to normal
    
    if (velProj < 0.0f)
    {
        radius *= -1.0f;
    }
    
    hitTime = std::max((boxProj - radius) / velProj, hitTime);
    outTime = std::min((boxProj + radius) / velProj, outTime);
    
    const glm::vec2 boxMin = box.center - (glm::vec2{box.width, box.height} / 2.0f);
    const glm::vec2 boxMax = box.center + (glm::vec2{box.width, box.height} / 2.0f);
    
    //x axis overlap
    if (box.velocity.x < 0.01f) //sweeping left
    {
        if (boxMax.x < segMin.x)
        {
            return 1.0f;
        }
        hitTime = std::max((segMax.x - boxMin.x) / box.velocity.x, hitTime);
        outTime = std::min((segMin.x - boxMax.x) / box.velocity.x, outTime);
    }
    else if (box.velocity.x > 0.01f) //sweeping right
    {
        if (boxMin.x > segMax.x)
        {
            return false;
        }
        hitTime = std::max((segMin.x - boxMax.x) / box.velocity.x, hitTime);
        outTime = std::min((segMax.x - boxMin.x) / box.velocity.x, outTime);
    }
    else if (segMin.x > boxMax.x || segMax.x < boxMin.x)
    {
        return 1.0f;
    }
    
    if (hitTime > outTime)
    {
        return 1.0f;
    }
    
    //y axis overlap
    if (box.velocity.y < 0.01f) //sweeping left
    {
        if (boxMax.y < segMin.y)
        {
            return 1.0f;
        }
        hitTime = std::max((segMax.y - boxMin.y) / box.velocity.y, hitTime);
        outTime = std::min((segMin.y - boxMax.y) / box.velocity.y, outTime);
    }
    else if (box.velocity.y > 0.01f) //sweeping right
    {
        if (boxMin.y > segMax.y)
        {
            return false;
        }
        hitTime = std::max((segMin.y - boxMax.y) / box.velocity.y, hitTime);
        outTime = std::min((segMax.y - boxMin.y) / box.velocity.y, outTime);
    }
    else if (segMin.y > boxMax.y || segMax.y < boxMin.y)
    {
        return 1.0f;
    }
    
    if (hitTime > outTime)
    {
        return 1.0f;
    }
    
    return hitTime;
}
