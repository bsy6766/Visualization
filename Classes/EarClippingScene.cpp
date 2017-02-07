#include "EarClippingScene.h"
#include "MainScene.h"

USING_NS_CC;

EarClippingScene* EarClippingScene::createScene()
{
	EarClippingScene* newEarClippingScene = EarClippingScene::create();
	return newEarClippingScene;
}

bool EarClippingScene::init()
{
	if (!cocos2d::Scene::init())
	{
		return false;
	}

	this->scheduleUpdate();
    
    this->currentSceneState = SCENE_STATE::IDLE;
    
    ECS::Manager::getInstance();
    
    // dot node
    this->dotNode = cocos2d::Node::create();
    this->addChild(this->dotNode, Z_ORDER::DOT);
    
    this->outerLineNode = cocos2d::DrawNode::create();
    this->addChild(this->outerLineNode, Z_ORDER::LINE);
    
    this->innerLineNode = cocos2d::DrawNode::create();
    this->addChild(this->innerLineNode, Z_ORDER::LINE);
    
    this->finalLineNode = cocos2d::DrawNode::create();
    this->addChild(this->finalLineNode, Z_ORDER::LINE);
    
    this->earClippingLineNode = cocos2d::DrawNode::create();
    this->addChild(this->earClippingLineNode, Z_ORDER::LINE);
    
    auto winSize = cocos2d::Director::getInstance()->getVisibleSize();
    
    // init display boundary box node which draws outer line of simulation display box
    this->displayBoundaryBoxNode = DisplayBoundaryBoxNode::createNode();
    this->displayBoundaryBoxNode->setPosition(cocos2d::Vec2::ZERO);
    this->displayBoundaryBoxNode->displayBoundary = cocos2d::Rect(0, 0, 650, 650);
    this->displayBoundaryBoxNode->drawDisplayBoundaryBox();
    this->displayBoundary = this->displayBoundaryBoxNode->displayBoundary;
    this->displayBoundaryBoxNode->retain();
    this->displayBoundaryBoxNode->drawNode->setLocalZOrder(static_cast<int>(Z_ORDER::BOX));
    this->addChild(this->displayBoundaryBoxNode);
    
    // Init labels node
    this->labelsNode = LabelsNode::createNode();
    this->labelsNode->setSharedLabelPosition(LabelsNode::SHARED_LABEL_POS_TYPE::QUADTREE_SCENE);
    this->addChild(this->labelsNode);

    // Starting pos
    float labelX = winSize.height - 10.0f;
    float labelY = winSize.height - 45.0f;
    
    // Set title
    this->labelsNode->initTitleStr("Ear Clipping", cocos2d::Vec2(labelX, labelY));
    
    labelY -= 50.0f;
    
    // Init custom labels
    this->labelsNode->customLabelStartPos = cocos2d::Vec2(labelX, labelY);
    
    // Set size
    const int customLabelSize = 25;
    
    this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Status: Idle", customLabelSize);

    
	return true;
}

void EarClippingScene::onEnter()
{
	cocos2d::Scene::onEnter();
	initInputListeners();
    initECS();
}

void EarClippingScene::initECS()
{
    auto m = ECS::Manager::getInstance();
//    auto system = m->createSystem<ECS::EarClippingSystem>();
    
    m->createEntityPool("OUTER", 32);
    m->createEntityPool("INNER", 32);
    
}

void EarClippingScene::update(float delta)
{

}

const float EarClippingScene::determinant(const cocos2d::Vec2 &a, const cocos2d::Vec2 &b)
{
    return (a.x * b.y) - (a.y * b.x);
}

void EarClippingScene::drawLinesBetweenDots(std::list<cocos2d::Vec2>& verticies, cocos2d::DrawNode& drawNode)
{
    drawNode.clear();
    
    int index = 0;
    auto it = verticies.begin();
    auto next = verticies.begin();
    std::advance(next, 1);
    
    for (; next != verticies.end(); )
    {
        drawNode.drawLine(*it, *next, cocos2d::Color4F::YELLOW);
        
        index++;
        it++;
        next++;
    }
    
    auto first = verticies.begin();
    auto last = verticies.end();
    std::advance(last, -1);
    
    if(this->currentSceneState == SCENE_STATE::IDLE)
    {
        drawNode.drawLine(*first, *last, cocos2d::Color4F::YELLOW);
    }
}

void EarClippingScene::drawTriangles(std::vector<cocos2d::Vec2> &triangles)
{
    this->earClippingLineNode->clear();
    for(unsigned int i = 0; i < triangles.size(); i+=3)
    {
        this->earClippingLineNode->drawTriangle(triangles.at(i), triangles.at(i+1), triangles.at(i+2), cocos2d::Color4F::ORANGE);
        this->earClippingLineNode->drawLine(triangles.at(i), triangles.at(i+1), cocos2d::Color4F::GREEN);
        this->earClippingLineNode->drawLine(triangles.at(i+1), triangles.at(i+2), cocos2d::Color4F::GREEN);
        this->earClippingLineNode->drawLine(triangles.at(i+2), triangles.at(i), cocos2d::Color4F::GREEN);
    }
}

void EarClippingScene::scaleDotSizeAndColor(const float scale, const cocos2d::Vec2& frontPoint, const cocos2d::Color3B& color, const std::string& entityPoolName)
{
    std::vector<ECS::Entity*> dots;
    ECS::Manager::getInstance()->getAllEntitiesInPool(dots, entityPoolName);
    
    for(auto dot : dots)
    {
        auto spriteComp = dot->getComponent<ECS::Sprite>();
        spriteComp->sprite->stopAllActions();
        spriteComp->sprite->runAction(cocos2d::ScaleTo::create(0.5f, scale));
        if(spriteComp->sprite->getPosition() == frontPoint)
        {
            spriteComp->sprite->runAction(cocos2d::TintTo::create(0.5f, cocos2d::Color3B::GREEN));
        }
        else
        {
            spriteComp->sprite->runAction(cocos2d::TintTo::create(0.5f, color));
        }
    }
}

const bool EarClippingScene::isCounterClockWise(const std::list<cocos2d::Vec2>& verticies)
{
    int total = 0;
    int index = 0;
    auto it = verticies.begin();
    auto next = verticies.begin();
    std::advance(next, 1);
    
    for (; next != verticies.end(); )
    {
        total += ((next->x - it->x) * (next->y + it->y));
        index++;
        it++;
        next++;
        
        if(next == verticies.end())
        {
            it = verticies.begin();
            total += ((next->x - it->x) * (next->y + it->y));
        }
    }
    
    if(total >= 0)
    {
        cocos2d::log("It's clockwise");
        return false;
    }
    else
    {
        cocos2d::log("It's counter clock wise");
        return true;
    }
}

void EarClippingScene::reverseVerticiesOrder(std::list<cocos2d::Vec2>& verticies, std::list<cocos2d::Label*>& labels)
{
    auto vert_it = std::begin(verticies);
    std::advance(vert_it, 1);
    std::reverse(vert_it, std::end(verticies));
    auto label_it = std::begin(labels);
    std::advance(label_it, 1);
    std::reverse(label_it, std::end(labels));
    
    this->reassignLabelNumber(labels);
}

void EarClippingScene::reassignLabelNumber(std::list<cocos2d::Label *> &labels)
{
    int count = 0;
    for(auto label : labels)
    {
        label->setString(std::to_string(count));
        count++;
    }
}

const bool EarClippingScene::doesPointIntersectLines(const std::list<cocos2d::Vec2> &verticies, const cocos2d::Vec2 start, const cocos2d::Vec2 end)
{
    if(verticies.size() < 3)
    {
        return false;
    }
    
    auto p1 = verticies.begin();
    auto p2 = verticies.begin();
    std::advance(p2, 1);
    auto end_it = verticies.end();
    std::advance(end_it, -1);
    for (; p2 != end_it; )
    {
        cocos2d::Vec2 a = *p1;
        cocos2d::Vec2 b = *p2;

        bool intersect = doesSegmentIntersects(a, b, start, end);
        if(intersect)
        {
            return true;
        }
        
        p1++;
        p2++;
    }
    
    return false;
}

const float EarClippingScene::ccw(const cocos2d::Vec2& a, const cocos2d::Vec2 &b)
{
    return a.cross(b);
}

const float EarClippingScene::ccw(const cocos2d::Vec2 &p, const cocos2d::Vec2 &a, const cocos2d::Vec2 &b)
{
    return ccw(a - p, b - p);
}

const bool EarClippingScene::doesSegmentIntersects(cocos2d::Vec2 a, cocos2d::Vec2 b, cocos2d::Vec2 c, cocos2d::Vec2 d)
{
    float ab = ccw(a, b, c) * ccw(a, b, d);
    float cd = ccw(c, d, a) * ccw(c, d, b);
    
    // In case where two lines are aligned in one segment or end point touches
    /*
    if(ab == 0 && cd == 0)
    {
        if (b < a)
        {
            auto t = a;
            a = b;
            b = t;
        }
        
        if(d < c)
        {
            auto t = c;
            c = d;
            d = c;
        }
        
        return !(b < c || d < a);
    }
    else
    {
        return ab <= 0 && cd <= 0;
    }
     */
    
    // if ab, cd is 0, it means end point touches each other.
    return ab < 0 && cd < 0;
}

void EarClippingScene::changeState(SCENE_STATE state)
{
    if(this->currentSceneState == SCENE_STATE::IDLE)
    {
        //
    }
    else if(this->currentSceneState == SCENE_STATE::DRAWING_OUTER_STATE)
    {
        // End drawing outer polygon
        this->scaleDotSizeAndColor(0.6f, this->outerVerticies.front(), cocos2d::Color3B::BLUE, "OUTER");
        
        // Outer verticies must be counter clock wise
        bool cc = this->isCounterClockWise(this->outerVerticies);
        if(!cc)
        {
            this->reverseVerticiesOrder(this->outerVerticies, this->outerVerticiesLabels);
        }
        
//        this->drawLinesBetweenDots(this->outerVerticies);
    }
    else if(this->currentSceneState == SCENE_STATE::DRAWING_INNER_STATE)
    {
        // End drawing inner polygon
        this->scaleDotSizeAndColor(0.6f, this->innerVerticies.front(), cocos2d::Color3B::BLUE, "INNER");
        
        // Inner verticies must be clock wise
        bool cc = this->isCounterClockWise(this->innerVerticies);
        if(cc)
        {
            this->reverseVerticiesOrder(this->innerVerticies, this->innerVerticiesLabels);
        }
        
//        this->drawLinesBetweenDots(this->innerVerticies);
    }
    else if(this->currentSceneState == SCENE_STATE::ALGORITHM_STATE)
    {
        
    }
    
    this->currentSceneState = state;
}

void EarClippingScene::addVertex(const cocos2d::Vec2& point, std::list<cocos2d::Vec2>& verticies, std::list<cocos2d::Label*>& verticiesLabels, const std::string& entityPoolName)
{
    // Draw outer points
    // Check if new point intersects existing lines
    bool intersect = this->doesPointIntersectLines(verticies, verticies.back(), point);
    if(intersect == false)
    {
        // Create dot
        bool success = ECS::Manager::getInstance()->getSystem<ECS::EarClippingSystem>()->createNewDot(*this->dotNode, entityPoolName, point);
        if(success)
        {
            // Add new point
            verticies.push_back(point);
            // Attach number label
            auto newLabel = cocos2d::Label::createWithTTF(std::to_string(verticies.size() - 1), "fonts/Rubik-Medium.ttf", 15);
            auto pos = verticies.back();
            pos.x += 10.0f;
            newLabel->setPosition(pos);
            this->dotNode->addChild(newLabel);
            verticiesLabels.push_back(newLabel);
            
            this->scaleDotSizeAndColor(1.0f, verticies.front(), cocos2d::Color3B::RED, entityPoolName);
            
            if(entityPoolName == "OUTER")
            {
                this->drawLinesBetweenDots(verticies, *this->outerLineNode);
            }
            else if(entityPoolName == "INNER")
            {
                this->drawLinesBetweenDots(verticies, *this->innerLineNode);
            }
            else
            {
                this->drawLinesBetweenDots(verticies, *this->finalLineNode);
            }
            
            cocos2d::log("Added vertex (%f, %f), #%s", point.x, point.y, newLabel->getString().c_str());
        }
    }
}

const bool EarClippingScene::finishAddingVertex(std::list<cocos2d::Vec2> &verticies)
{
    if(verticies.size() < 3)
    {
        return false;
    }
    // Check if last point and starting point segment intersects any line
    // copy points
    std::list<cocos2d::Vec2> points = verticies;
    // remove the first and last point (touching points)
    points.pop_front();
    points.pop_back();
    // Check intersect
    bool intersect = this->doesPointIntersectLines(points, verticies.back(), verticies.front());
    if(intersect)
    {
        return false;
    }
    else
    {
        return true;
    }
}

const bool EarClippingScene::isPointInPolygon(std::list<cocos2d::Vec2>& verticies, const cocos2d::Vec2& point)
{
    auto c = point;
    auto d = point;
    d.x += 2000.0f;  // Create a long line, think as raycasting to right. 2000 pixels are enough for this example.
    
    auto p1 = verticies.begin();
    auto p2 = verticies.begin();
    std::advance(p2, 1);
    
    int count = 0;
    for (; p2 != verticies.end(); )
    {
        bool intersect = this->doesSegmentIntersects(*p1, *p2, c, d);
        if(intersect)
        {
            count++;
        }
        
        p1++;
        p2++;
        
        if(p2 == verticies.end())
        {
            p1 = verticies.begin();
            bool intersect = this->doesSegmentIntersects(*p2, *p1, c, d);
            if(intersect)
            {
                count++;
            }
        }
    }
    
    if(count % 2 == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}


const bool EarClippingScene::isPointInOrOnTriangle(const cocos2d::Vec2& a, const cocos2d::Vec2& b, const cocos2d::Vec2& c, const cocos2d::Vec2& p)
{
    return this->determinant(p - a, b - a) >= 0 &&
            this->determinant(p - b, c - b) >= 0 &&
            this->determinant(p - c, a - c) >= 0;
}


void EarClippingScene::removeVertex(const std::string& entityPoolName, std::list<cocos2d::Vec2>& verticies, std::list<cocos2d::Label*>& verticiesLabels, const cocos2d::Vec2& point)
{
    std::vector<ECS::Entity*> entities;
    ECS::Manager::getInstance()->getAllEntitiesInPool(entities, entityPoolName);
    
    for(auto dot : entities)
    {
        auto spriteComp = dot->getComponent<ECS::Sprite>();
        if(spriteComp->sprite->getBoundingBox().containsPoint(point))
        {
            auto pos = spriteComp->sprite->getPosition();
            
            auto it = verticies.begin();
            auto label_it = verticiesLabels.begin();
            for (; it != verticies.end(); )
            {
                if((*it) == pos)
                {
                    verticies.erase(it);
                    (*label_it)->removeFromParentAndCleanup(true);
                    verticiesLabels.erase(label_it);
                    this->scaleDotSizeAndColor(1.0f, verticies.front(), cocos2d::Color3B::RED, entityPoolName);
                    
                    // Reorder label
                    this->reassignLabelNumber(verticiesLabels);
                    
                    break;
                }
                it++;
                label_it++;
            }
            
            dot->kill();
            if(entityPoolName == "OUTER")
            {
                this->drawLinesBetweenDots(verticies, *this->outerLineNode);
            }
            else
            {
                this->drawLinesBetweenDots(verticies, *this->innerLineNode);
            }
            break;
        }
    }
}

void EarClippingScene::clearVerticies(std::list<cocos2d::Vec2> &verticies, std::list<cocos2d::Label *>& verticiesLabels, const std::string& entityPoolName)
{
    verticies.clear();
    for(auto label : verticiesLabels)
    {
        label->removeFromParentAndCleanup(true);
    }
    verticiesLabels.clear();
    
    auto m = ECS::Manager::getInstance();
    m->deleteEntityPool(entityPoolName);
    int size = 0;
    if(entityPoolName == "OUTER")
        size = 32;
    else
        size = 32;
    m->createEntityPool(entityPoolName, size);
    
    if(entityPoolName == "OUTER")
    {
        this->outerLineNode->clear();
    }
    else
    {
        this->innerLineNode->clear();
    }
}

void EarClippingScene::finalizeVerticies()
{
    // Find nearest point. Iterate through outer verticies and find visible point and must be shortest.
    auto cuttingOuterPoint = cocos2d::Vec2::ZERO;
    auto cuttingInnerPoint = cocos2d::Vec2::ZERO;
    bool cuttingPointFound = false;
    float shortestDist = 2000.0f;   // Arbitrary number. 2000 is enough long distance
    // Try all inner points
    for(auto innerPoint : this->innerVerticies)
    {
        auto c = innerPoint;
        for(auto point : this->outerVerticies)
        {
            auto d = point;
            
            // Check if segment cd doesn't intersect any lines.
            bool intersects = this->doesPointIntersectLines(this->outerVerticies, c, d);
            if(intersects)
            {
                continue;
            }
            else
            {
                float dist = c.distance(d);
                if(shortestDist > dist)
                {
                    cuttingOuterPoint = point;
                    cuttingInnerPoint = innerPoint;
                    shortestDist = dist;
                    cuttingPointFound = true;
//                    cocos2d::log("Found new cutting points");
//                    cocos2d::log("cutting outer point (%f, %f)", cuttingOuterPoint.x, cuttingOuterPoint.y);
//                    cocos2d::log("cutting inner point (%f, %f)", cuttingInnerPoint.x, cuttingInnerPoint.y);
//                    cocos2d::log("Distance = %f", shortestDist);
                }
            }
        }
    }
    
//    cocos2d::log("cutting outer point (%f, %f)", cuttingOuterPoint.x, cuttingOuterPoint.y);
//    cocos2d::log("cutting inner point (%f, %f)", cuttingInnerPoint.x, cuttingInnerPoint.y);
    
    // Create new entity pool
    ECS::Manager::getInstance()->deleteEntityPool("FINAL");
    ECS::Manager::getInstance()->createEntityPool("FINAL", 64);
    
    if(cuttingPointFound == false)
    {
        // No possible cut.
//        cocos2d::log("NO POSSIBLE CUT");
        
        for(auto p : this->outerVerticies)
        {
            this->addVertex(p, this->finalVerticies, this->finalVerticiesLabels, "FINAL");
        }
        
        this->drawLinesBetweenDots(this->finalVerticies, *this->finalLineNode);
        
        this->toggleVertexVisibility(this->outerVerticiesLabels, "OUTER", false);
        this->toggleVertexVisibility(this->innerVerticiesLabels, "INNER", false);
    }
    else
    {
        // Found cutting point. Combine outer and inner.
        for(auto outerPoint : this->outerVerticies)
        {
            if(outerPoint == cuttingOuterPoint)
            {
                // Insert outer point.
                // First add outer cutting point
                this->addVertex(cuttingOuterPoint, this->finalVerticies, this->finalVerticiesLabels, "FINAL");
                auto pos = this->finalVerticiesLabels.back()->getPosition();
                pos.x += 10.0f;
                this->finalVerticiesLabels.back()->setPosition(pos);
                
                int innerPointSize = this->innerVerticies.size();
                int counter = 0;
                auto it = this->innerVerticies.begin();
                
                for(; it != this->innerVerticies.end(); it++)
                {
                    if((*it) == cuttingInnerPoint)
                    {
                        break;
                    }
                }
                
                while(counter < innerPointSize)
                {
                    this->addVertex((*it), this->finalVerticies, this->finalVerticiesLabels, "FINAL");
                    it++;
                    if(it == this->innerVerticies.end())
                    {
                        it = this->innerVerticies.begin();
                    }
                    
                    counter++;
                }
                
                // add additional point to make cutting point connects to outer polygon
                // 1. Add first inner point again
                this->addVertex(cuttingInnerPoint, this->finalVerticies, this->finalVerticiesLabels, "FINAL");
                pos = this->finalVerticiesLabels.back()->getPosition();
                pos.x += 10.0f;
                this->finalVerticiesLabels.back()->setPosition(pos);
                // 2. Add cutting outer point again
                this->addVertex(cuttingOuterPoint, this->finalVerticies, this->finalVerticiesLabels, "FINAL");
                pos = this->finalVerticiesLabels.back()->getPosition();
                pos.x += 10.0f;
                this->finalVerticiesLabels.back()->setPosition(pos);
            }
            else
            {
                this->addVertex(outerPoint, this->finalVerticies, this->finalVerticiesLabels, "FINAL");
            }
        }
        
        this->drawLinesBetweenDots(this->finalVerticies, *this->finalLineNode);
        
        this->toggleVertexVisibility(this->outerVerticiesLabels, "OUTER", false);
        this->toggleVertexVisibility(this->innerVerticiesLabels, "INNER", false);
    }
}

void EarClippingScene::toggleVertexVisibility(std::list<cocos2d::Label*>& verticiesLabels, const std::string& entityPoolName, const bool mode)
{
    std::vector<ECS::Entity*> entities;
    ECS::Manager::getInstance()->getAllEntitiesInPool(entities, entityPoolName);
    
    for(auto dot : entities)
    {
        auto spriteComp = dot->getComponent<ECS::Sprite>();
        spriteComp->sprite->setVisible(mode);
    }
    
    for(auto label : verticiesLabels)
    {
        label->setVisible(mode);
    }
}

std::vector<cocos2d::Vec2> EarClippingScene::runEarClipping()
{
    const int size = static_cast<int>(this->finalVerticies.size());
    if(size < 3)
    {
        return std::vector<cocos2d::Vec2>();
    }
    
    if(size == 3)
    {
        std::vector<cocos2d::Vec2> singleTriangle;
        for(auto point : this->finalVerticies)
        {
            singleTriangle.push_back(point);
        }
        
        return singleTriangle;
    }
    
    std::list<EarClippingScene::Vertex*> verticies;
    
    auto f_it = this->finalVerticies.begin();
    for (; f_it != this->finalVerticies.end();)
    {
        EarClippingScene::Vertex* newVertex = new EarClippingScene::Vertex();
        newVertex->point = *f_it;
        newVertex->prev = nullptr;
        newVertex->next = nullptr;
        verticies.push_back(newVertex);
        
        f_it++;
    }
    
    unsigned int verticiesSize = verticies.size();
    for(unsigned int i = 0; i < verticiesSize; i++)
    {
        if(i >= 1 && i < verticiesSize - 1)
        {
            auto p_it = verticies.begin();
            std::advance(p_it, i);
            
            auto prevP_it = verticies.begin();
            std::advance(prevP_it, i - 1);
            
            auto nextP_it = verticies.begin();
            std::advance(nextP_it, i + 1);
            
            (*p_it)->prev = *prevP_it;
            (*p_it)->next = *nextP_it;
        }
    }
    
    verticies.front()->prev = verticies.back();
    auto v_it = verticies.begin();
    std::advance(v_it, 1);
    verticies.front()->next = *v_it;
    
    v_it = verticies.begin();
    std::advance(v_it, verticiesSize - 1);
    verticies.back()->prev = *v_it;
    verticies.back()->next = verticies.front();
    
    cocos2d::Vec2 p;
    cocos2d::Vec2 prevP;
    cocos2d::Vec2 nextP;
    
    std::vector<cocos2d::Vec2> triangles;
    
    int actualSize = verticiesSize;
    
    EarClippingScene::Vertex* head = verticies.front();
    
    while(actualSize >= 3)
    {
        p = head->point;
        prevP = head->prev->point;
        nextP = head->next->point;
        
//        cocos2d::log("Finidng ear");
//        cocos2d::log("p = (%f, %f)", p.x, p.y);
//        cocos2d::log("prevP = (%f, %f)", prevP.x, prevP.y);
//        cocos2d::log("nextP = (%f, %f)", nextP.x, nextP.y);
        
        if(this->isEar(prevP, p, nextP))
        {
            // Ear found
//            cocos2d::log("Found ear!");
            triangles.push_back(prevP);
            triangles.push_back(p);
            triangles.push_back(nextP);
            
            // Remove ear
//            cocos2d::log("Reassigning pointers");
            head->prev->next = head->next;
            head->next->prev = head->prev;
//            cocos2d::log("removing p = (%f, %f)", p.x, p.y);
//            cocos2d::log("prev's next is = (%f, %f)", head->prev->next->point.x, head->prev->next->point.y);
//            cocos2d::log("next's prev is = (%f, %f)", head->next->prev->point.x, head->next->prev->point.y);
            
            actualSize--;
        }
        
        head = head->next;
    }
    
    for(auto v : verticies)
    {
        delete v;
    }
    verticies.clear();
    
    return triangles;
}

const bool EarClippingScene::isEar(const cocos2d::Vec2 prevP, const cocos2d::Vec2 p, const cocos2d::Vec2 nextP)
{
    
    if(this->isConvex(prevP, p, nextP))
    {
        std::list<cocos2d::Vec2> triangle = {prevP, p, nextP};
        // Check if triangle doesn't have any points in.
        for(auto point : this->finalVerticies)
        {
            if(point == prevP || point == p || point == nextP)
            {
                continue;
            }
//            if(this->isPointInPolygon(triangle, point))
//            {
//                return false;
//            }
            
            if(this->isPointInOrOnTriangle(nextP, p, prevP, point))
            {
                return false;
            }
        }
        
        return true;
    }
    else
    {
        return false;
    }
}


const bool EarClippingScene::isConvex(const cocos2d::Vec2 prevP, const cocos2d::Vec2 p, const cocos2d::Vec2 nextP)
{
    float result = ((prevP.x * (nextP.y - p.y)) + (p.x * (prevP.y - nextP.y)) + (nextP.x * (p.y - prevP.y)));
    return result < 0;
}

void EarClippingScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(EarClippingScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(EarClippingScene::onMouseDown, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(EarClippingScene::onKeyPressed, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void EarClippingScene::onMouseMove(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void EarClippingScene::onMouseDown(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
//	0 = left, 1 = right, 2 = middle
	int mouseButton = mouseEvent->getMouseButton();
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();
    cocos2d::Vec2 point(x, y);
    
    if(mouseButton == 0)
    {
        if(this->displayBoundary.containsPoint(point))
        {
            if(this->currentSceneState == SCENE_STATE::DRAWING_OUTER_STATE)
            {
                // Add outer vertex
                this->addVertex(point, this->outerVerticies, this->outerVerticiesLabels, "OUTER");
            }
            else if(this->currentSceneState == SCENE_STATE::DRAWING_INNER_STATE)
            {
                // Draw inner points
                // Check if point is in polygon
                bool inPolygon = this->isPointInPolygon(this->outerVerticies, point);
                if(inPolygon)
                {
                    this->addVertex(point, this->innerVerticies, this->innerVerticiesLabels, "INNER");
                }
            }
        }
    }
    else if(mouseButton == 1)
    {
        if(this->displayBoundary.containsPoint(point))
        {
            if(this->currentSceneState == SCENE_STATE::DRAWING_OUTER_STATE)
            {
                this->removeVertex("OUTER", this->outerVerticies, this->outerVerticiesLabels, point);
            }
            else if(this->currentSceneState == SCENE_STATE::DRAWING_INNER_STATE)
            {
                this->removeVertex("INNER", this->innerVerticies, this->innerVerticiesLabels, point);
            }
        }
    }
}

void EarClippingScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
    if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE)
    {
        if(this->currentSceneState == SCENE_STATE::IDLE)
        {
            // Terminate
            cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
        }
        else
        {
            if(this->currentSceneState == SCENE_STATE::DRAWING_OUTER_STATE)
            {
                bool canFinish = this->finishAddingVertex(this->outerVerticies);
                if(canFinish)
                {
                    this->changeState(SCENE_STATE::IDLE);
                    this->drawLinesBetweenDots(this->outerVerticies, *this->outerLineNode);
                }
                else
                {
                    if(this->outerVerticies.size() < 3)
                    {
                        this->clearVerticies(this->outerVerticies, this->outerVerticiesLabels, "OUTER");
                    }
                    this->changeState(SCENE_STATE::IDLE);
                }
            }
            else if(this->currentSceneState == SCENE_STATE::DRAWING_INNER_STATE)
            {
                
                bool canFinish = this->finishAddingVertex(this->innerVerticies);
                if(canFinish)
                {
                    this->changeState(SCENE_STATE::IDLE);
                    this->drawLinesBetweenDots(this->innerVerticies, *this->innerLineNode);
                }
            }
            else
            {
                if(this->innerVerticies.size() < 3)
                {
                    this->clearVerticies(this->innerVerticies, this->innerVerticiesLabels, "INNER");
                }
                this->changeState(SCENE_STATE::IDLE);
            }
        }
    }
    
    
    if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_C)
    {
        if(this->currentSceneState == SCENE_STATE::DRAWING_OUTER_STATE)
        {
            this->clearVerticies(this->outerVerticies, this->outerVerticiesLabels, "OUTER");
        }
        else if(this->currentSceneState == SCENE_STATE::DRAWING_INNER_STATE)
        {
            this->clearVerticies(this->innerVerticies, this->innerVerticiesLabels, "INNER");
        }
    }
    
    if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_1)
    {
        if(this->currentSceneState != SCENE_STATE::DRAWING_OUTER_STATE)
        {
            this->changeState(SCENE_STATE::DRAWING_OUTER_STATE);
            this->scaleDotSizeAndColor(1.0f, this->outerVerticies.front(), cocos2d::Color3B::RED, "OUTER");
        }
    }
    else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_2)
    {
        if(this->currentSceneState != SCENE_STATE::DRAWING_INNER_STATE)
        {
            // If outer verticies has less than 3 verticies, it doesn't have any polygon. Can't add inner polygon
            if(this->outerVerticies.size() < 3)
            {
                return;
            }
            this->changeState(SCENE_STATE::DRAWING_INNER_STATE);
            this->scaleDotSizeAndColor(1.0f, this->innerVerticies.front(), cocos2d::Color3B::RED, "INNER");
        }
    }
    else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_3)
    {
        if(this->currentSceneState != SCENE_STATE::FINALIZE_STATE)
        {
            // finialize
            // If outer verticies has less than 3 verticies, it doesn't have any polygon. Can't finalize
            if(this->outerVerticies.size() < 3)
            {
                return;
            }
            this->changeState(SCENE_STATE::FINALIZE_STATE);
            this->finalizeVerticies();
            this->scaleDotSizeAndColor(0.6f, this->innerVerticies.front(), cocos2d::Color3B::GREEN, "FINAL");
        }
    }
    else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_4)
    {
        if(this->currentSceneState != SCENE_STATE::ALGORITHM_STATE)
        {
            if(!this->finalVerticies.empty())
            {
                // Run ear clipping algoritm
                this->changeState(SCENE_STATE::ALGORITHM_STATE);
                std::vector<cocos2d::Vec2> triangles = this->runEarClipping();
                this->toggleVertexVisibility(this->finalVerticiesLabels, "FINAL", false);
                this->drawTriangles(triangles);
                
            }
        }
    }
}

void EarClippingScene::releaseInputListeners()
{
	if(this->mouseInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->mouseInputListener);
	if(this->keyInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->keyInputListener);
}

void EarClippingScene::onExit()
{
	cocos2d::Scene::onExit();
	releaseInputListeners();
    ECS::Manager::deleteInstance();
}
