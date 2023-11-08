#include "util/BspBuilder.h"

#include <algorithm>
#include <span>

#include "util/Bsp.h"

struct BspBuilder::Implementation
{
    std::vector<Brush> brushes;
};

BspBuilder::BspBuilder()
{
    pImpl = std::make_unique<Implementation>();
}

BspBuilder::~BspBuilder() = default;

void BspBuilder::addBrushes(std::vector<Brush> brushes)
{
    pImpl->brushes.reserve(pImpl->brushes.size() + brushes.size());
    std::copy(brushes.begin(), brushes.end(), std::back_inserter(pImpl->brushes));
}

struct TextureInfo
{
    glm::vec3 uAxis;
    float uOffset;
    
    glm::vec3 vAxis;
    float vOffset;
    
    float scale;
    
    std::string textureName;
};

struct ConvexPolygon
{
    Plane plane;
    
    std::vector<glm::vec3> vertices;
    
    bool usedAsSplit;
    
    TextureInfo textureInfo;
    
    std::unique_ptr<ConvexPolygon> next;
};

static std::unique_ptr<ConvexPolygon> convertBrushesToPolygons(std::span<const Brush> brushes)
{
    std::unique_ptr<ConvexPolygon> firstPolygon;
    
    for (const auto& brush : brushes)
    {
        const auto brushVertices = brush.getVertices();
        const float textureScale = brush.getTextureScale();
        const std::string textureName = brush.getTextureName().data();
        const auto planes = brush.getPlanes();
        for (const auto& plane : planes)
        {
            std::vector<glm::vec3> vertices;
            for (const auto& brushVertex : brushVertices)
            {
                if (Plane::classifyPoint(plane, brushVertex) == Plane::Classification::Coincident)
                {
                    vertices.push_back(brushVertex);
                }
            }
            
            std::unique_ptr<ConvexPolygon> newPolygon = std::make_unique<ConvexPolygon>();
            
            const auto textureAxises = bsp::getTextureAxisFromNormal(plane.normal);
            
            newPolygon->plane = plane;
            newPolygon->vertices = std::move(vertices);
            newPolygon->usedAsSplit = false;
            newPolygon->textureInfo =
            {
                .uAxis = textureAxises.first,
                .uOffset = 0.0f,
                .vAxis = textureAxises.second,
                .vOffset = 0.0f,
                .scale = textureScale,
                .textureName = textureName
            };
            
            //add this to the linked list
            newPolygon->next = std::move(firstPolygon);
            firstPolygon = std::move(newPolygon);
        }
    }
    
    return firstPolygon;
}

static ConvexPolygon* findBestSplitPolygon(std::unique_ptr<ConvexPolygon>& firstPolygon)
{
    {
        size_t i = 0;
        ConvexPolygon* bestPolygon = nullptr;
        for (ConvexPolygon* currentPolygon = firstPolygon.get(); currentPolygon; currentPolygon = currentPolygon->next.get())
        {
            if (!currentPolygon->usedAsSplit)
            {
                i++;
                bestPolygon = currentPolygon;
            }
        }
        
        if (i == 0)
        {
            return nullptr;
        }
        else if (i == 1)
        {
            return bestPolygon;
        }
    }
    
    constexpr float blendFactor = 0.2f;
    
    ConvexPolygon* bestPolygon = nullptr;
    size_t bestSplits = std::numeric_limits<size_t>::max(); //GET THE LEAST NUMBER OF SPLITS
    float bestScore = std::numeric_limits<float>::max(); //hope for a balanceder tree
    
    for (ConvexPolygon* currentPolygon = firstPolygon.get(); currentPolygon; currentPolygon = currentPolygon->next.get())
    {
        if (currentPolygon->usedAsSplit)
        {
            continue;
        }
        
        const Plane& plane = currentPolygon->plane;
        size_t numSplit = 0;
        int numInFront = 0;
        int numBehind = 0;
        
        for (ConvexPolygon* otherPolygon = firstPolygon.get(); otherPolygon; otherPolygon = otherPolygon->next.get())
        {
            if (currentPolygon == otherPolygon || otherPolygon->usedAsSplit)
            {
                continue;
            }
            
            switch (Plane::classifyPoints(plane, otherPolygon->vertices))
            {
            case Plane::Classification::Front:
                numInFront++;
                break;
            case Plane::Classification::Back:
                numBehind++;
                break;
            case Plane::Classification::Spanning:
                numSplit++;
                break;
            case Plane::Classification::Coincident:
                //do nothing
                break;
            }
        }
        
        const float score = blendFactor * numSplit + (1.0f - blendFactor) * std::fabs(numInFront - numBehind);
        
        if (numSplit < bestSplits || (numSplit == bestSplits && score < bestScore))
        {
            bestPolygon = currentPolygon;
            bestSplits = numSplit;
            bestScore = score;
        }
    }
    
    if (bestPolygon)
    {
        return bestPolygon;
    }
    
    return nullptr;
}

struct Node
{
    Plane splitPlane;
    
    enum class Type
    {
        Node = 0,
        Leaf = 1
    } type;
    
    enum class Contents
    {
        Solid,
        Empty
    } contents;
    
    std::unique_ptr<Node> childFront;
    std::unique_ptr<Node> childBack;
    
    std::unique_ptr<ConvexPolygon> firstPolygon;
};

static std::pair<ConvexPolygon, ConvexPolygon> splitFace(const Plane& plane, const ConvexPolygon& polygon)
{
    std::vector<glm::vec3> frontVerticies;
    std::vector<glm::vec3> backVerticies;
    
    glm::vec3 a = polygon.vertices[polygon.vertices.size() - 1];
    
    Plane::Classification aSide = Plane::classifyPoint(plane, a);
    
    for (const glm::vec3& b : polygon.vertices)
    {
        const Plane::Classification bSide = Plane::classifyPoint(plane, b);
        if (bSide == Plane::Classification::Front)
        {
            if (aSide == Plane::Classification::Back)
            {
                const glm::vec3 intersection = Plane::intersectRay(plane, a, glm::normalize(b - a)).value();
                frontVerticies.push_back(intersection);
                backVerticies.push_back(intersection);
            }
            frontVerticies.push_back(b);
        }
        else if (bSide == Plane::Classification::Back)
        {
            if (aSide == Plane::Classification::Front)
            {
                const glm::vec3 intersection = Plane::intersectRay(plane, a, glm::normalize(b - a)).value();
                frontVerticies.push_back(intersection);
                backVerticies.push_back(intersection);
            }
            else if (aSide == Plane::Classification::Coincident)
            {
                backVerticies.push_back(a);
            }
            backVerticies.push_back(b);
        }
        else
        {
            frontVerticies.push_back(b);
            if (aSide == Plane::Classification::Back)
            {
                backVerticies.push_back(b);
            }
        }
        
        a = b;
        aSide = bSide;
    }
    
    const auto convertVerticiesToPolygon = [&polygon](std::vector<glm::vec3> verticies) -> ConvexPolygon
    {
        ConvexPolygon newPolygon{};
        
        newPolygon.plane = polygon.plane;
        newPolygon.textureInfo = polygon.textureInfo;
        newPolygon.usedAsSplit = polygon.usedAsSplit;
        
        newPolygon.vertices = std::move(verticies);
        
        newPolygon.next = nullptr;
        
        return newPolygon;
    };
    
    ConvexPolygon frontPolygon = convertVerticiesToPolygon(std::move(frontVerticies));
    ConvexPolygon backPolygon = convertVerticiesToPolygon(std::move(backVerticies));
    
    return { std::move(frontPolygon), std::move(backPolygon) };
}

static std::unique_ptr<Node> buildNode(std::unique_ptr<ConvexPolygon> firstPolygon)
{
    const auto makeLeaf = [](std::unique_ptr<ConvexPolygon> firstPolygon)
    {
        std::unique_ptr<Node> newNode = std::make_unique<Node>();
        
        newNode->type = Node::Type::Leaf;
        
        if (!firstPolygon) newNode->contents = Node::Contents::Empty;
        else newNode->contents = Node::Contents::Solid;
        
        newNode->firstPolygon = std::move(firstPolygon);
        
        return newNode;
    };
    
    if (!firstPolygon)
    {
        return makeLeaf(std::move(firstPolygon));
    }
    
    ConvexPolygon* splittingPolygon = findBestSplitPolygon(firstPolygon);
    
    if (!splittingPolygon)
    {
        return makeLeaf(std::move(firstPolygon));
    }
    
    splittingPolygon->usedAsSplit = true;
    //mark polygons with similar planes as already used as a split
    for (ConvexPolygon* polygon = firstPolygon.get(); polygon; polygon = polygon->next.get())
    {
        const bool sameNormalA = glm::dot(polygon->plane.normal, splittingPolygon->plane.normal) <= 0.01f;
        const bool sameDistanceA = std::abs(polygon->plane.distance - splittingPolygon->plane.distance) <= 0.01f;
        const bool sameNormalB = glm::dot(-polygon->plane.normal, splittingPolygon->plane.normal) <= 0.01f;
        const bool sameDistanceB = std::abs(-polygon->plane.distance - splittingPolygon->plane.distance) <= 0.01f;
        
        const bool samePlane = (sameNormalA && sameDistanceA) || (sameNormalB && sameDistanceB);
        
        if (!polygon->usedAsSplit && samePlane)
        {
            polygon->usedAsSplit = true;
        }
    }
    
    std::unique_ptr<ConvexPolygon> frontPolygonList;
    std::unique_ptr<ConvexPolygon> backPolygonList;
    
    for (ConvexPolygon* currentPolygon = firstPolygon.get(); currentPolygon; currentPolygon = currentPolygon->next.get())
    {
        Plane::Classification classification = Plane::classifyPoints(splittingPolygon->plane, currentPolygon->vertices);
        
        const auto addToList = [](std::unique_ptr<ConvexPolygon>& polygonList, ConvexPolygon& addPolygon)
        {
            std::unique_ptr<ConvexPolygon> newPolygon = std::make_unique<ConvexPolygon>();
            
            newPolygon->plane = addPolygon.plane;
            newPolygon->vertices = addPolygon.vertices;
            newPolygon->usedAsSplit = addPolygon.usedAsSplit;
            newPolygon->textureInfo = addPolygon.textureInfo;
            
            newPolygon->next = std::move(polygonList);
            
            polygonList = std::move(newPolygon);
        };
        
        switch (classification)
        {
        case Plane::Classification::Coincident:
            addToList(backPolygonList, *currentPolygon);
            addToList(frontPolygonList, *currentPolygon);
            break;
        case Plane::Classification::Back:
            addToList(backPolygonList, *currentPolygon);
            break;
        case Plane::Classification::Front:
            addToList(frontPolygonList, *currentPolygon);
            break;
        case Plane::Classification::Spanning:
            auto [frontPolygon, backPolygon] = splitFace(splittingPolygon->plane, *currentPolygon);
            addToList(frontPolygonList, frontPolygon);
            addToList(backPolygonList, backPolygon);
            break;
        }
    }
    
    std::unique_ptr<Node> newNode = std::make_unique<Node>();
    
    newNode->type = Node::Type::Node;
    
    newNode->splitPlane = splittingPolygon->plane;
    
    newNode->firstPolygon = std::move(firstPolygon);
    
    if (!frontPolygonList) newNode->childFront = makeLeaf(std::move(frontPolygonList));
    else                   newNode->childFront = buildNode(std::move(frontPolygonList));
    if (!backPolygonList)  newNode->childBack  = makeLeaf(std::move(backPolygonList));
    else                   newNode->childBack  = buildNode(std::move(backPolygonList));
    
    return newNode;
}

static int64_t convertNode(bsp::File& file, std::unique_ptr<Node>& node)
{
    const auto addVertex = [&file](const glm::vec3& vertex)
    {
        auto index = static_cast<bsp::ArrayLength>(file.vertices.size());
        if (std::find(file.vertices.begin(), file.vertices.end(), vertex) == file.vertices.end())
        {
            file.vertices.push_back(vertex);
        }
        else
        {
            index = static_cast<bsp::ArrayLength>(std::distance(file.vertices.begin(), std::find(file.vertices.begin(), file.vertices.end(), vertex)));
        }
        
        return index;
    };
    
    const auto addPlane = [&file](const Plane& plane)
    {
        const auto index = static_cast<bsp::ArrayLength>(file.planes.size());
        file.planes.push_back(plane);
        
        return index;
    };
    
    const auto addTextureInfo = [&file](const bsp::TextureInfo& textureInfo)
    {
        auto index = static_cast<bsp::ArrayLength>(file.textureInfos.size());
        if (std::find(file.textureInfos.begin(), file.textureInfos.end(), textureInfo) == file.textureInfos.end())
        {
            file.textureInfos.push_back(textureInfo);
        }
        else
        {
            const auto texInfoLoc = std::find(file.textureInfos.begin(), file.textureInfos.end(), textureInfo);
            index = static_cast<bsp::ArrayLength>(std::distance(file.textureInfos.begin(), texInfoLoc));
        }
        
        return index;
    };
    
    const auto convertTextureInfo = [&file](const TextureInfo& texInfo)
    {
        if (std::find(file.textureNames.begin(), file.textureNames.end(), texInfo.textureName) == file.textureNames.end())
        {
            file.textureNames.push_back(texInfo.textureName);
        }
        
        const auto texNameLoc = std::find(file.textureNames.begin(), file.textureNames.end(), texInfo.textureName);
        
        const bsp::TextureInfo newTexInfo
        {
            .uAxis = texInfo.uAxis,
            .uOffset = texInfo.uOffset,
            .vAxis = texInfo.vAxis,
            .vOffset = texInfo.vOffset,
            .scale = texInfo.scale,
            .textureIndex = static_cast<bsp::SmallArrayLength>(std::distance(file.textureNames.begin(), texNameLoc))
        };
        
        return newTexInfo;
    };
    
    const auto convertPolygons = [&file, &node, addVertex, addPlane, addTextureInfo, convertTextureInfo](bsp::ArrayLength& numFaces)
    {
        for (const ConvexPolygon* currentPolygon = node->firstPolygon.get(); currentPolygon; currentPolygon = currentPolygon->next.get())
        {
            bsp::Face newFace{};
            newFace.firstEdge = bsp::ArrayLength(file.edges.size());
            newFace.numEdges = 0;
            
            for (size_t i = 0; i < currentPolygon->vertices.size(); i++)
            {
                size_t j = (i + 1) % currentPolygon->vertices.size();
                
                bsp::Edge newEdge{};
                newEdge.startVertex = addVertex(currentPolygon->vertices[i]);
                newEdge.endVertex = addVertex(currentPolygon->vertices[j]);
                
                file.edges.push_back(newEdge);
                
                newFace.numEdges++;
            }
            
            newFace.plane = addPlane(currentPolygon->plane);
            
            newFace.textureInfoIndex = addTextureInfo(convertTextureInfo(currentPolygon->textureInfo));
            
            file.faces.push_back(newFace);
            
            numFaces++;
        }
    };
    
    if (node->type == Node::Type::Node)
    {
        bsp::Node newNode{};
        newNode.splitPlane = addPlane(node->splitPlane);
        
        newNode.firstFace = static_cast<bsp::ArrayLength>(file.faces.size());
        convertPolygons(newNode.numFaces);
        
        newNode.frontChild = convertNode(file, node->childFront);
        newNode.backChild = convertNode(file, node->childBack);
        
        file.nodes.push_back(newNode);
        
        const size_t newLoc = file.nodes.size() - 1;
        
        return int64_t(newLoc);
    }
    else
    {
        bsp::Leaf newLeaf{};
        
        newLeaf.content = node->contents == Node::Contents::Empty ? 0 : 1;
        
        newLeaf.firstFace = static_cast<bsp::ArrayLength>(file.faces.size());
        convertPolygons(newLeaf.numFaces);
        
        file.leaves.push_back(newLeaf);
        
        const size_t newLoc = file.leaves.size() - 1;
        
        return -(int64_t(newLoc) + 1);
    }
}

bsp::File BspBuilder::build()
{
    std::unique_ptr<ConvexPolygon> firstPolygon = convertBrushesToPolygons(pImpl->brushes);
    
    std::unique_ptr<Node> rootNode = buildNode(std::move(firstPolygon));
    
    bsp::File file{};
    convertNode(file, rootNode);
    
    return file;
}
