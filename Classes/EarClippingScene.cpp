#include "EarClippingScene.h"
#include "MainScene.h"
#include "Utility.h"

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

	// info button
	this->infoButton = cocos2d::ui::Button::create("infoButton.png", "infoButtonSelected.png", "infoButtonDisabled.png", cocos2d::ui::Widget::TextureResType::PLIST);
	this->infoButton->addClickEventListener(CC_CALLBACK_1(EarClippingScene::onInfoButtonClick, this));
	auto pos = this->displayBoundary.origin;
	pos.x -= 15.0f;
	pos.y += (this->displayBoundary.size.height + 15.0f);
	this->infoButton->setPosition(pos);
	this->addChild(this->infoButton);

	// instruction 
	this->instructionLabel = cocos2d::Label::createWithTTF("Welcome to Ear Clipping algorithm visualization.\nHere you can make ONE polygon up to ONE hole.\nPlease read following direction to test it out.\n\n1. Press ENTER to start.\n2. Click in the orange box to make outer polygon.\n3. Press ENTER to prcoeed.\n4. (Optional) Click in the outer polygon to make inner polygon.\n5. Press ENTER to proceed. It will finalize polygon.\n6. (Optional) Press BACK SPACE to go back.\n7. Press ENTER again to run ear clipping algorithm.\n8. Click 'i' icon on top left to close/open this instruction.", "fonts/Rubik-Medium.ttf", 20.0f, cocos2d::Size::ZERO, cocos2d::TextHAlignment::LEFT);
	this->instructionLabel->setAnchorPoint(cocos2d::Vec2(0, 1.0f));
	this->instructionLabel->setPosition(this->infoButton->getPosition() + cocos2d::Vec2(20.0f, -20.0f));
	this->addChild(this->instructionLabel, Z_ORDER::INSTRUCTION);
    
	this->viewingInstruction = true;

    // Init labels node
    this->labelsNode = LabelsNode::createNode();
    this->labelsNode->setSharedLabelPosition(LabelsNode::SHARED_LABEL_POS_TYPE::EAR_CLIPPING_SCENE);
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
	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Outer polygon vertex: 0", customLabelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Inner polygon vertex: 0", customLabelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Total triangle: 0", customLabelSize);

	// init more labels    
	const int headerSize = 25;
	const float blockGap = 22.0f;
	const int labelSize = 20;

	const float weightLastY = this->labelsNode->customLabels.back()->getBoundingBox().getMinY();
	this->labelsNode->keyboardUsageLabelStartPos = cocos2d::Vec2(labelX, weightLastY - blockGap);

	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Keys", headerSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Enter: Start / Proceed", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Back Space: Revert / Back", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "C: Clear all vertex (While drawing polygon)", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "R: Restart", labelSize);

	const float keysLastY = this->labelsNode->keyboardUsageLabels.back()->getBoundingBox().getMinY();
	this->labelsNode->mouseUsageLabelStartPos = cocos2d::Vec2(labelX, keysLastY - blockGap);

	this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Mouse", headerSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Left Click (In box): Add outer vertex", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Left Click (In outer polygon): Add outer vertex", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Right Click (On vertex): Remove vertex", labelSize);

	// speed modifier
	this->simulationSpeedModifier = 1.0f;

	this->sliderLabelNode = SliderLabelNode::createNode();
	const float mouseLastY = this->labelsNode->mouseUsageLabels.back()->getBoundingBox().getMinY();
	this->sliderLabelNode->sliderStartPos = cocos2d::Vec2(labelX, mouseLastY - blockGap);
	this->sliderLabelNode->addSlider("Simulation Speed", "Slider", 50, CC_CALLBACK_1(EarClippingScene::onSliderClick, this));
	this->addChild(this->sliderLabelNode);

	this->elapsedTime = 0;
	this->stepDuration = 0.1f;

	this->finished = false;
    
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
	this->labelsNode->updateFPSLabel(delta);

	delta *= simulationSpeedModifier;

	if (this->currentSceneState == SCENE_STATE::ALGORITHM_STATE)
	{
		this->elapsedTime += delta;
		while (this->elapsedTime > this->stepDuration)
		{
			this->elapsedTime -= this->stepDuration;

			Utility::Time::start();

			this->runEarClipping();

			Utility::Time::stop();

			std::string timeTakenStr = Utility::Time::getElaspedTime();	// Microseconds
			float timeTakenF = std::stof(timeTakenStr);	// to float
			timeTakenF *= 0.001f; // To milliseconds

			if (this->actualVerticiesSize < 3)
			{
				this->labelsNode->updateTimeTakenLabel("0");
			}
			else
			{
				this->labelsNode->updateTimeTakenLabel(std::to_string(timeTakenF).substr(0, 5));
			}

			this->drawTriangles();
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::TOTAL_TRIANGLE), "Total triangle: " + std::to_string(this->triangles.size() / 3));
		}
	}
}

void EarClippingScene::drawLinesBetweenDots(std::list<cocos2d::Vec2>& verticies, cocos2d::DrawNode& drawNode, const bool drawEnd)
{
	if (verticies.empty())
	{
		return;
	}

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
    
    if(drawEnd)
    {
        drawNode.drawLine(*first, *last, cocos2d::Color4F::YELLOW);
    }
}

void EarClippingScene::drawTriangles()
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

void EarClippingScene::changeState(SCENE_STATE state)
{
    if(this->currentSceneState == SCENE_STATE::IDLE)
    {
        //
    }
    else if(this->currentSceneState == SCENE_STATE::DRAWING_OUTER_STATE)
    {
		if (this->outerVerticies.empty() == false)
		{
			// End drawing outer polygon
			this->scaleDotSizeAndColor(0.6f, this->outerVerticies.front(), cocos2d::Color3B::BLUE, "OUTER");

			// Outer verticies must be counter clock wise
			bool cc = Utility::Polygon::isCounterClockWise(this->outerVerticies);
			if (!cc)
			{
				this->reverseVerticiesOrder(this->outerVerticies, this->outerVerticiesLabels);
			}
		}
    }
    else if(this->currentSceneState == SCENE_STATE::DRAWING_INNER_STATE)
    {
        // End drawing inner polygon
		if (this->innerVerticies.empty() == false)
		{
			this->scaleDotSizeAndColor(0.6f, this->innerVerticies.front(), cocos2d::Color3B::BLUE, "INNER");

			// Inner verticies must be clock wise
			bool cc = Utility::Polygon::isCounterClockWise(this->innerVerticies);
			if (cc)
			{
				this->reverseVerticiesOrder(this->innerVerticies, this->innerVerticiesLabels);
			}
		}
    }
    else if(this->currentSceneState == SCENE_STATE::FINALIZE_STATE)
    {

    }
    
    this->currentSceneState = state;
}

const bool EarClippingScene::addVertex(const cocos2d::Vec2& point, std::list<cocos2d::Vec2>& verticies, std::list<cocos2d::Label*>& verticiesLabels, const std::string& entityPoolName)
{
	bool intersect = false;
	if (!verticies.empty())
	{
		//intersect = this->doesPointIntersectLines(verticies, verticies.back(), point);
		intersect = Utility::Polygon::doesPointIntersectPolygonSegments(verticies, point);
		if (intersect)
		{
			return false;
		}
	}
    // Draw outer points
    // Check if new point intersects existing lines
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
                this->drawLinesBetweenDots(verticies, *this->outerLineNode, false);
            }
            else if(entityPoolName == "INNER")
            {
                this->drawLinesBetweenDots(verticies, *this->innerLineNode, false);
            }
            else
            {
                this->drawLinesBetweenDots(verticies, *this->finalLineNode, false);
            }
            
			return true;
        }
    }

	return false;
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
    //bool intersect = this->doesPointIntersectLines(points, verticies.back(), verticies.front());
	bool intersect = Utility::Polygon::doesPointIntersectPolygonSegments(verticies, verticies.front());
    if(intersect)
    {
        return false;
    }
    else
    {
        return true;
    }
}

const bool EarClippingScene::removeVertex(const std::string& entityPoolName, std::list<cocos2d::Vec2>& verticies, std::list<cocos2d::Label*>& verticiesLabels, const cocos2d::Vec2& point)
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
					if (verticies.empty() == false)
					{
						this->scaleDotSizeAndColor(1.0f, verticies.front(), cocos2d::Color3B::RED, entityPoolName);
					}
                    
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
                this->drawLinesBetweenDots(verticies, *this->outerLineNode, false);
            }
            else
            {
                this->drawLinesBetweenDots(verticies, *this->innerLineNode, false);
            }
            
			return true;
        }
    }

	return false;
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
    else if(entityPoolName == "INNER")
    {
        this->innerLineNode->clear();
    }
	else if (entityPoolName == "FINAL")
	{
		this->finalLineNode->clear();
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
            //bool intersects = this->doesPointIntersectLines(this->outerVerticies, c, d);
			bool intersect = Utility::Polygon::doesPointIntersectPolygonSegments(this->outerVerticies, d);
            if(intersect)
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
                }
            }
        }
    }
    
    // Create new entity pool
    ECS::Manager::getInstance()->deleteEntityPool("FINAL");
    ECS::Manager::getInstance()->createEntityPool("FINAL", 64);
    
    if(cuttingPointFound == false)
    {
        // No possible cut.        
        for(auto p : this->outerVerticies)
        {
            this->addVertex(p, this->finalVerticies, this->finalVerticiesLabels, "FINAL");
        }
        
        this->drawLinesBetweenDots(this->finalVerticies, *this->finalLineNode, true);
        
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
        
        this->drawLinesBetweenDots(this->finalVerticies, *this->finalLineNode, true);
        
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

void EarClippingScene::runEarClipping()
{
    cocos2d::Vec2 p;
    cocos2d::Vec2 prevP;
    cocos2d::Vec2 nextP;
    
	if (this->head == nullptr)
	{
		this->head = verticies.front();
	}

	p = head->point;
	prevP = head->prev->point;
	nextP = head->next->point;

	if (this->isEar(prevP, p, nextP))
	{
		// Ear found
		this->triangles.push_back(prevP);
		this->triangles.push_back(p);
		this->triangles.push_back(nextP);

		// Remove ear
		head->prev->next = head->next;
		head->next->prev = head->prev;

		this->actualVerticiesSize--;
	}

	this->head = this->head->next;

	if (this->actualVerticiesSize < 3)
	{
		this->changeState(SCENE_STATE::FINISHED);
		this->toggleVertexVisibility(this->finalVerticiesLabels, "FINAL", false);
		this->drawTriangles();
		this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Finished");
	}
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
            
            if(Utility::Polygon::isPointInOrOnTriangle(nextP, p, prevP, point))
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

void EarClippingScene::makeVerticies()
{
	for (auto vertex : this->verticies)
	{
		delete vertex;
	}

	this->verticies.clear();

	auto f_it = this->finalVerticies.begin();
	for (; f_it != this->finalVerticies.end();)
	{
		EarClippingScene::Vertex* newVertex = new EarClippingScene::Vertex();
		newVertex->point = *f_it;
		newVertex->prev = nullptr;
		newVertex->next = nullptr;
		this->verticies.push_back(newVertex);

		f_it++;
	}

	unsigned int verticiesSize = this->verticies.size();
	for (unsigned int i = 0; i < verticiesSize; i++)
	{
		if (i >= 1 && i < verticiesSize - 1)
		{
			auto p_it = this->verticies.begin();
			std::advance(p_it, i);

			auto prevP_it = this->verticies.begin();
			std::advance(prevP_it, i - 1);

			auto nextP_it = this->verticies.begin();
			std::advance(nextP_it, i + 1);

			(*p_it)->prev = *prevP_it;
			(*p_it)->next = *nextP_it;
		}
	}

	this->verticies.front()->prev = this->verticies.back();
	auto v_it = this->verticies.begin();
	std::advance(v_it, 1);
	this->verticies.front()->next = *v_it;

	v_it = this->verticies.begin();
	std::advance(v_it, verticiesSize - 1);
	this->verticies.back()->prev = *v_it;
	this->verticies.back()->next = this->verticies.front();

	this->head = nullptr;

	this->actualVerticiesSize = this->verticies.size();
}

void EarClippingScene::onInfoButtonClick(cocos2d::Ref * sender)
{
	if (this->viewingInstruction)
	{
		this->instructionLabel->runAction(cocos2d::Sequence::create(cocos2d::ScaleTo::create(0.25f, 0.0f), cocos2d::FadeTo::create(0, 0), nullptr));
	}
	else
	{
		this->instructionLabel->setOpacity(255);
		this->instructionLabel->runAction(cocos2d::ScaleTo::create(0.25f, 1.0f));
	}

	this->viewingInstruction = !this->viewingInstruction;
}

void EarClippingScene::onSliderClick(cocos2d::Ref* sender)
{
	// Click ended. Get value
	float percentage = static_cast<float>(this->sliderLabelNode->sliderLabels.back().slider->getPercent());
	//50% = 1.0(default. So multiply by 2.
	percentage *= 2.0f;
	// 0% == 0, 100% = 1.0f, 200% = 2.0f
	if (percentage == 0)
	{
		// 0 will make simulation stop
		percentage = 1;
	}
	//. Devide by 100%
	percentage *= 0.01f;

	// apply
	this->simulationSpeedModifier = percentage;
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
    auto mouseEvent = static_cast<EventMouse*>(event);
    float x = mouseEvent->getCursorX();
    float y = mouseEvent->getCursorY();
    
    auto point = cocos2d::Vec2(x, y);
    
    this->labelsNode->updateMouseHover(point);
}

void EarClippingScene::onMouseDown(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
//	0 = left, 1 = right, 2 = middle
	int mouseButton = mouseEvent->getMouseButton();
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();
    
    auto point = cocos2d::Vec2(x, y);
    
    bool ret = this->labelsNode->updateMouseDown(point);
    if (ret)
    {
        return;
    }
    
    if(mouseButton == 0)
    {
        if(this->displayBoundary.containsPoint(point))
        {
            if(this->currentSceneState == SCENE_STATE::DRAWING_OUTER_STATE)
            {
                // Add outer vertex
                bool success = this->addVertex(point, this->outerVerticies, this->outerVerticiesLabels, "OUTER");
				if (success)
				{
					this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::OUTER_VERTEX), "Outer polygon vertex: " + std::to_string(this->outerVerticies.size()) + " / 32");
					this->labelsNode->playAnimation(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::ADD_OUTER_VERTEX));
				}
            }
            else if(this->currentSceneState == SCENE_STATE::DRAWING_INNER_STATE)
            {
                // Draw inner points
                // Check if point is in polygon
                bool inPolygon = Utility::Polygon::isPointInPolygon(this->outerVerticies, point);
                if(inPolygon)
                {
					bool success = this->addVertex(point, this->innerVerticies, this->innerVerticiesLabels, "INNER");
					if (success)
					{
						this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::INNER_VERTEX), "Inner polygon vertex: " + std::to_string(this->innerVerticies.size()) + " / 32");
						this->labelsNode->playAnimation(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::ADD_INNER_VERTEX));
					}
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
                bool success = this->removeVertex("OUTER", this->outerVerticies, this->outerVerticiesLabels, point);
				if (success)
				{
					this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::OUTER_VERTEX), "Outer polygon vertex: " + std::to_string(this->outerVerticies.size()) + " / 32");
					this->labelsNode->playAnimation(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::REMOVE_VERTEX));
				}
            }
            else if(this->currentSceneState == SCENE_STATE::DRAWING_INNER_STATE)
            {
                bool success = this->removeVertex("INNER", this->innerVerticies, this->innerVerticiesLabels, point);
				if (success)
				{
					this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::INNER_VERTEX), "Inner polygon vertex: " + std::to_string(this->innerVerticies.size()) + " / 32");
					this->labelsNode->playAnimation(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::REMOVE_VERTEX));
				}
            }
        }
    }
}

void EarClippingScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
    if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE)
    {
		// Terminate
		cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
    }
    

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ENTER)
	{
		if (this->currentSceneState == SCENE_STATE::IDLE)
		{
			this->changeState(SCENE_STATE::DRAWING_OUTER_STATE);
			//this->scaleDotSizeAndColor(1.0f, this->outerVerticies.front(), cocos2d::Color3B::RED, "OUTER");
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Drawing outer polygon");
			this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::ENTER));
		}
		else if (this->currentSceneState == SCENE_STATE::DRAWING_OUTER_STATE)
		{
			bool canFinish = this->finishAddingVertex(this->outerVerticies);
			if (canFinish)
			{
				this->changeState(SCENE_STATE::IDLE);
				this->drawLinesBetweenDots(this->outerVerticies, *this->outerLineNode, true);
				this->changeState(SCENE_STATE::DRAWING_INNER_STATE);
				this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Drawing inner polygon");
				this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::ENTER));
			}
		}
		else if (this->currentSceneState == SCENE_STATE::DRAWING_INNER_STATE)
		{
			bool canFinish = this->finishAddingVertex(this->innerVerticies);

			if (canFinish == false)
			{
				if (this->innerVerticies.empty())
				{
					canFinish = true;
				}
			}

			if (canFinish)
			{
				if (this->innerVerticies.empty() == false)
				{
					this->changeState(SCENE_STATE::IDLE);
					this->drawLinesBetweenDots(this->innerVerticies, *this->innerLineNode, true);
				}
				// Finalize
				this->changeState(SCENE_STATE::FINALIZE_STATE);
				this->finalizeVerticies();
				this->scaleDotSizeAndColor(0.6f, this->finalVerticies.front(), cocos2d::Color3B::GREEN, "FINAL");
				this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Finalize polygon");
				this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::ENTER));
			}
		}
		else if (this->currentSceneState == SCENE_STATE::FINALIZE_STATE)
		{
			if (!this->finalVerticies.empty())
			{
				// Run ear clipping algoritm
				this->changeState(SCENE_STATE::ALGORITHM_STATE);
				this->makeVerticies();
				this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Running Ear Clipping");
				this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::ENTER));

				if (this->finalVerticies.size() == 3)
				{
					this->triangles.clear();
					for (auto point : this->finalVerticies)
					{
						this->triangles.push_back(point);
					}

					this->drawTriangles();

					this->changeState(SCENE_STATE::FINISHED);
					this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Finished");
				}
			}
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_BACKSPACE)
	{
		if (this->currentSceneState == SCENE_STATE::DRAWING_OUTER_STATE)
		{
			// If outer verticies has less than 3 verticies, it doesn't have any polygon. Can't add inner polygon
			this->clearVerticies(this->outerVerticies, this->outerVerticiesLabels, "OUTER");
			this->changeState(SCENE_STATE::IDLE);
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Idle");
			this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::CANCEL));
		}
		else if (this->currentSceneState == SCENE_STATE::DRAWING_INNER_STATE)
		{
			this->clearVerticies(this->innerVerticies, this->innerVerticiesLabels, "INNER");
			this->changeState(SCENE_STATE::DRAWING_OUTER_STATE);
			this->scaleDotSizeAndColor(1.0f, this->outerVerticies.front(), cocos2d::Color3B::RED, "OUTER");
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Drawing outer polygon");
			this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::CANCEL));
		}
		else if (this->currentSceneState == SCENE_STATE::FINALIZE_STATE)
		{
			this->clearVerticies(this->finalVerticies, this->finalVerticiesLabels, "FINAL");

			this->changeState(SCENE_STATE::DRAWING_INNER_STATE);

			this->toggleVertexVisibility(this->outerVerticiesLabels, "OUTER", true);
			this->toggleVertexVisibility(this->innerVerticiesLabels, "INNER", true);

			this->drawLinesBetweenDots(this->outerVerticies, *this->outerLineNode, true);
			this->drawLinesBetweenDots(this->innerVerticies, *this->innerLineNode, false);

			this->scaleDotSizeAndColor(1.0f, this->innerVerticies.front(), cocos2d::Color3B::RED, "INNER");
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Drawing inner polygon");
			this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::CANCEL));
		}
	}

    if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_C)
    {
        if(this->currentSceneState == SCENE_STATE::DRAWING_OUTER_STATE)
        {
            this->clearVerticies(this->outerVerticies, this->outerVerticiesLabels, "OUTER");
			this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::CLEAR));
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::OUTER_VERTEX), "Outer polygon vertex:  0 / 32");
        }
        else if(this->currentSceneState == SCENE_STATE::DRAWING_INNER_STATE)
        {
            this->clearVerticies(this->innerVerticies, this->innerVerticiesLabels, "INNER");
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::INNER_VERTEX), "Inner polygon vertex:  0 / 32");
			this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::CLEAR));
        }
    }
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_R)
	{
		this->currentSceneState = SCENE_STATE::IDLE;

		this->clearVerticies(this->outerVerticies, this->outerVerticiesLabels, "OUTER");
		this->clearVerticies(this->innerVerticies, this->innerVerticiesLabels, "INNER");
		this->clearVerticies(this->finalVerticies, this->finalVerticiesLabels, "FINAL");

		for (auto vertex : this->verticies)
		{
			delete vertex;
		}
		this->verticies.clear();

		this->actualVerticiesSize = 0;
		this->head = nullptr;
		this->triangles.clear();
		this->elapsedTime = 0;
		this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Idle");

		this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::OUTER_VERTEX), "Outer polygon vertex:  0 / 32");
		this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::INNER_VERTEX), "Inner polygon vertex:  0 / 32");
		this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::TOTAL_TRIANGLE), "Total triangles: 0");

		auto m = ECS::Manager::getInstance();

		m->deleteEntityPool("OUTER");
		m->deleteEntityPool("INNER");
		m->deleteEntityPool("FINAL");
		m->createEntityPool("OUTER", 32);
		m->createEntityPool("INNER", 32);

		this->earClippingLineNode->clear();

		this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::RESTART));
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
