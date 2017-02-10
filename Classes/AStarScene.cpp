#include "AStarScene.h"
#include "MainScene.h"

USING_NS_CC;

AStarScene* AStarScene::createScene()
{
	AStarScene* newAStarScene = AStarScene::create();
	return newAStarScene;
}

bool AStarScene::init()
{
	if (!cocos2d::Scene::init())
	{
		return false;
	}

    //this->scheduleUpdate();

	this->pause = false;
	this->finished = true;
    this->updateScheduled = false;
    this->cleared = true;
    
    // init display boundary box node which draws outer line of simulation display box
    this->displayBoundaryBoxNode = DisplayBoundaryBoxNode::createNode();
    this->displayBoundaryBoxNode->setPosition(cocos2d::Vec2::ZERO);
    this->displayBoundaryBoxNode->displayBoundary = cocos2d::Rect(0, 0, 650, 650);
    this->displayBoundaryBoxNode->drawDisplayBoundaryBox();
    this->displayBoundaryBoxNode->retain();
    this->addChild(this->displayBoundaryBoxNode, Z_ORDER::BOX);

	// bg draw node
	this->bgDrawNode = cocos2d::DrawNode::create();
	this->addChild(this->bgDrawNode, Z_ORDER::GRID_LINE);

	this->bgDrawNode->setLineWidth(2.0f);

	auto startingPos = this->displayBoundaryBoxNode->displayBoundary.origin;

	float cellSize = 26.0f;
	int gridRowSize = static_cast<int>(650.0f / cellSize);
	int gridColSize = static_cast<int>(650.0f / cellSize);

	auto origin = this->displayBoundaryBoxNode->displayBoundary.origin;
	auto destination = origin + this->displayBoundaryBoxNode->displayBoundary.size;

	for (int row = 1; row < gridRowSize; row++)
	{
		float y = startingPos.y + (static_cast<float>(row * cellSize));

		this->bgDrawNode->drawLine(cocos2d::Vec2(origin.x, y), cocos2d::Vec2(destination.x, y), cocos2d::Color4F::ORANGE);
	}
	for (int col = 1; col < gridColSize; col++)
	{
		float x = startingPos.x + (static_cast<float>(col * cellSize));
		this->bgDrawNode->drawLine(cocos2d::Vec2(x, origin.y), cocos2d::Vec2(x, destination.y), cocos2d::Color4F::ORANGE);
	}

	this->draggingStart = false;
	this->draggingEnd = false;

	this->allowDiagonal = true;

	// hovering cell sprite
	this->hoveringCellSprite = cocos2d::Sprite::createWithSpriteFrameName("square_26_hover.png");
	this->hoveringCellSprite->setVisible(false);
	this->addChild(this->hoveringCellSprite, Z_ORDER::HOVER);

	// dragging
	this->draggingStartSprite = cocos2d::Sprite::createWithSpriteFrameName("square_26.png");
	this->draggingStartSprite->setScale(1.2f);
	this->draggingStartSprite->setColor(cocos2d::Color3B(0, 255, 255));
	this->draggingStartSprite->setOpacity(100);
	this->draggingStartSprite->setVisible(false);
	this->addChild(this->draggingStartSprite, Z_ORDER::DRAG);

	this->draggingEndSprite = cocos2d::Sprite::createWithSpriteFrameName("square_26.png");
	this->draggingEndSprite->setScale(1.2f);
	this->draggingEndSprite->setColor(cocos2d::Color3B(0, 255, 255));
	this->draggingEndSprite->setOpacity(100);
	this->draggingEndSprite->setVisible(false);
	this->addChild(this->draggingEndSprite, Z_ORDER::DRAG);
    
    // Init labels node
    this->labelsNode = LabelsNode::createNode();
    this->labelsNode->setSharedLabelPosition(LabelsNode::SHARED_LABEL_POS_TYPE::RECT_PACKING_SCENE);
    this->addChild(this->labelsNode);
    
    auto winSize = cocos2d::Director::getInstance()->getVisibleSize();
    // Starting pos
    float labelX = winSize.height - 10.0f;
    float labelY = winSize.height - 45.0f;
    
    // Set title
    this->labelsNode->initTitleStr("A* Pathfinding", cocos2d::Vec2(labelX, labelY));


	this->elapsedTime = 0;
	this->stepDuration = 0.03f;

	this->shiftPressing = false;

	return true;
}

void AStarScene::initECS()
{
	float cellSize = 26.0f;
	int gridRowSize = static_cast<int>(650.0f / cellSize);
	int gridColSize = static_cast<int>(650.0f / cellSize);

	auto startingPos = this->displayBoundaryBoxNode->displayBoundary.origin;

	auto m = ECS::Manager::getInstance();
	m->resizeEntityPool(ECS::DEFAULT_ENTITY_POOL_NAME, 1024);

	for (int row = 0; row < gridRowSize; row++)
	{
		float y = startingPos.y + static_cast<float>(cellSize * row);
		for (int col = 0; col < gridColSize; col++)
		{
			float x = startingPos.x + static_cast<float>(cellSize * col);
			cocos2d::Vec2 pos = cocos2d::Vec2(x + (cellSize * 0.5f), y + (cellSize * 0.5f));
			
			this->createNewCell(pos);
		}
	}

	ECS::Manager::getInstance()->getAllEntitiesInPool(this->cells);

	this->cells.front()->getComponent<ECS::Cell>()->setState(ECS::Cell::STATE::START);
	this->startingCell = this->cells.front();

	this->cells.back()->getComponent<ECS::Cell>()->setState(ECS::Cell::STATE::END);
	this->endingCell = this->cells.back();
    
    this->updateScore();
}

void AStarScene::createNewCell(const cocos2d::Vec2 & position)
{
	auto m = ECS::Manager::getInstance();
	ECS::Entity* cell = m->createEntity();
	if (cell != nullptr)
	{
		auto cellComp = m->createComponent<ECS::Cell>();

		cellComp->cellSprite = cocos2d::Sprite::createWithSpriteFrameName("square_26.png");
		cellComp->cellSprite->setPosition(position);
		this->addChild(cellComp->cellSprite, Z_ORDER::CELL);

		cellComp->position = position;

		cellComp->cellLabel = cocos2d::Label::createWithTTF("", "fonts/Rubik-Medium.ttf", 24);
		cellComp->cellLabel->setScale(0.5f);
		cellComp->cellLabel->setColor(cocos2d::Color3B::BLACK);
		cellComp->cellLabel->setPosition(cellComp->cellSprite->getContentSize() * 0.5f);
		cellComp->cellSprite->addChild(cellComp->cellLabel);

		cell->addComponent<ECS::Cell>(cellComp);
	}
}

void AStarScene::onEnter()
{
	cocos2d::Scene::onEnter();
	initInputListeners();
	initECS();
}

void AStarScene::update(float delta)
{
	if (finished)
	{
		return;
	}

	this->elapsedTime += delta;
	while (this->elapsedTime >= this->stepDuration)
	{
		this->elapsedTime -= this->stepDuration;

		this->findPath();
	}
}

void AStarScene::findPath()
{
	cocos2d::Vec2 start = this->startingCell->getComponent<ECS::Cell>()->position;
	cocos2d::Vec2 dest = this->endingCell->getComponent<ECS::Cell>()->position;
	// Open set
	// Closed set
	//while openset is not empty
	if (this->openSet.empty() == false)
	{
		this->revertPath();

		// Get cell with lowest f score.
		ECS::Cell* current = this->openSet.begin()->second;
		// Pop it from openset
        this->openSet.erase(this->openSet.begin());
        
        if (current->position == dest)
        {
            // You arrived! Finish.
            //cocos2d::log("Finished!");
            this->finished = true;
            this->retracePath(current);
            this->unscheduleUpdate();
            this->updateScheduled = false;
            this->endingCell->getComponent<ECS::Cell>()->previousCell = current;
            return;
        }

		// Mark as closed
        if (current->state == ECS::Cell::STATE::OPENED)
		{
			current->setState(ECS::Cell::STATE::CLOSED);
		}

		// Get neighbors
		std::vector<unsigned int> neighborIndicies = this->getNeightborIndicies(this->cellPosToIndex(current->position));
		//cocos2d::log("Neighbors...");
		for (auto index : neighborIndicies)
		{
			//cocos2d::log("%d", index);

			auto neighborComp = this->cells.at(index)->getComponent<ECS::Cell>();
			// Skip this cell if it's already closed or blocked.
			if (neighborComp->state == ECS::Cell::STATE::CLOSED || neighborComp->state == ECS::Cell::STATE::BLOCK || neighborComp->state == ECS::Cell::STATE::START)
			{
				continue;
			}

			// Calculate g cost from current cell to neighbor cell using current cell's g score
			float newGScore = current->g + fabsf(neighborComp->position.distance(current->position));

			// Add to openSet if not opened yet
			if (neighborComp->state != ECS::Cell::STATE::OPENED)
			{
                neighborComp->setState(ECS::Cell::STATE::OPENED);
				this->openSet.insert(std::pair<int, ECS::Cell*>(neighborComp->f, neighborComp));
			}
			else if (newGScore >= neighborComp->g)
			{
				// This is not better path. continue;
				continue;
			}

			// Recompute costs. This is the better path. So set current cell as previous cell of this neighbor cell.
            // Before we update, remove from openSet and reinsert with updated f score
            auto find_it = this->openSet.begin();
            for (; find_it != this->openSet.end(); )
            {
                if(find_it->first == static_cast<int>(neighborComp->f))
                {
                    if(find_it->second->position == neighborComp->position)
                    {
                        this->openSet.erase(find_it);
                        break;
                    }
                }
                
                find_it++;
            }
			neighborComp->g = newGScore;									// g = Distance from start
			//neighborComp->h = fabsf(dest.distance(neighborComp->position));		// h = Distance from end
			neighborComp->f = neighborComp->g + neighborComp->h;			// f = g + h
			neighborComp->previousCell = current;
            
            this->openSet.insert(std::pair<int, ECS::Cell*>(neighborComp->f, neighborComp));
		}

		this->retracePath(current);

	}
	else
	{
		// Failed to find path
		this->finished = true;
	}
}

cocos2d::Vec2 AStarScene::cursorPointToCellPos(const cocos2d::Vec2& point)
{
	auto cursorPoint = point - this->displayBoundaryBoxNode->displayBoundary.origin;

	int x = static_cast<int>(cursorPoint.x);
	int y = static_cast<int>(cursorPoint.y);

	int cellSize = 26;
	
	int row = x / cellSize;
	int col = y / cellSize;

	cocos2d::Vec2 cellPos = cocos2d::Vec2(row * cellSize + cellSize / 2, col * cellSize + cellSize / 2);
	cellPos += this->displayBoundaryBoxNode->displayBoundary.origin;
	return cellPos;
}

unsigned int AStarScene::cellPosToIndex(const cocos2d::Vec2& cellPos)
{
//	cocos2d::log("cellPos = (%f, %f)", cellPos.x, cellPos.y);
	auto actualCellPos = cellPos;
	actualCellPos -= this->displayBoundaryBoxNode->displayBoundary.origin;
	int x = static_cast<int>(actualCellPos.x) / 26;
	int y = static_cast<int>(actualCellPos.y) / 26;

	unsigned int index = static_cast<unsigned int>(y * 25 + x);

	assert(this->cells.at(index)->getComponent<ECS::Cell>()->position == cellPos);

	return index;
}

void AStarScene::cancelDragging()
{
	this->draggingStart = false;
	this->draggingStartSprite->setVisible(false);
	this->startingCell->getComponent<ECS::Cell>()->setState(ECS::Cell::STATE::START);

	this->draggingEnd = false;
	this->draggingEndSprite->setVisible(false);
	this->endingCell->getComponent<ECS::Cell>()->setState(ECS::Cell::STATE::END);
}

void AStarScene::resetPathFinding()
{
	this->openSet.clear();
	this->path.clear();

	// Add start 
	auto startComp = this->startingCell->getComponent<ECS::Cell>();
	auto endComp = this->endingCell->getComponent<ECS::Cell>();

	startComp->g = 0;															// g = Distance from start
	startComp->h = fabsf(startComp->position.distance(endComp->position));		// h = Distance from end
	startComp->f = startComp->g + startComp->h;									// f = g + h

	endComp->g = fabsf(endComp->position.distance(startComp->position));		// g = Distance from start
	endComp->h = 0;																// h = Distance from end
	endComp->f = endComp->g + endComp->h;										// f = g + h

	this->openSet.insert(std::pair<int, ECS::Cell*>(startComp->f, startComp));
}

std::vector<unsigned int> AStarScene::getNeightborIndicies(unsigned int currentIndex)
{
//	cocos2d::log("Getting neighbors for index: %d", currentIndex);
	int rowSize = 25;
	int colSize = 25;

	bool topEdge = false;
	bool botEdge = false;
	bool leftEdge = false;
	bool rightEdge = false;

	std::vector<unsigned int> neighbors;

	// N, E, W, S
	int row = currentIndex / rowSize;
	if (row == 0)
	{
		// It's on top row.
		topEdge = true;
	}
	else if (row == (rowSize - 1))
	{
		// It's on bottom row
		botEdge = true;
	}

	int col = currentIndex % colSize;
	if (col == 0)
	{
		// Left edge
		leftEdge = true;
	}
	else if (col == (colSize - 1))
	{
		// right edge
		rightEdge = true;
	}

	unsigned int maxSize = rowSize * colSize;

	if (!topEdge)
	{
		// Add N
		unsigned int index = currentIndex - colSize;
		if (index < maxSize)
		{
			neighbors.push_back(index);
		}
	}

	if (!botEdge)
	{
		// add S
		unsigned int index = currentIndex + colSize;
		if (index < maxSize)
		{
			neighbors.push_back(index);
		}
	}

	if (!leftEdge)
	{
		// add W
		unsigned int index = currentIndex - 1;
		if (index < maxSize)
		{
			neighbors.push_back(index);
		}
	}

	if (!rightEdge)
	{
		// add E
		unsigned int index = currentIndex + 1;
		if (index < maxSize)
		{
			neighbors.push_back(index);
		}
	}

	// Diagonal, NE, NW, SE, SW
	if (this->allowDiagonal)
	{
		if (!topEdge && !leftEdge)
		{
			// ADd NW
			unsigned int index = currentIndex - colSize - 1;
			if (index < maxSize)
			{
				neighbors.push_back(index);
			}
		}

		if (!topEdge && !rightEdge)
		{
			// Add NE
			unsigned int index = currentIndex - colSize + 1;
			if (index < maxSize)
			{
				neighbors.push_back(index);
			}
		}

		if (!botEdge && !leftEdge)
		{
			// Add SW
			unsigned int index = currentIndex + colSize - 1;
			if (index < maxSize)
			{
				neighbors.push_back(index);
			}
		}

		if (!botEdge && !rightEdge)
		{
			// Add SE
			unsigned int index = currentIndex + colSize + 1;
			if (index < maxSize)
			{
				neighbors.push_back(index);
			}
		}
	}

	return neighbors;
}

void AStarScene::retracePath(ECS::Cell* currentCell)
{
	ECS::Cell* startComp = this->startingCell->getComponent<ECS::Cell>();

	path.clear();

	if (currentCell->previousCell == nullptr)
	{
		return;
	}

	while (currentCell != startComp)
	{
		this->path.push_back(currentCell);
		if (currentCell->previousCell == nullptr)
		{
			break;
		}
		currentCell = currentCell->previousCell;
	}

	path.reverse();

	for(auto cell : this->path)
	{
		if (cell->state == ECS::Cell::STATE::CLOSED)
		{
			cell->setState(ECS::Cell::STATE::PATH);
		}
	}
}

void AStarScene::revertPath()
{
	for (auto cell : this->path)
	{
		if (cell->state == ECS::Cell::STATE::PATH)
		{
			cell->setState(ECS::Cell::STATE::CLOSED);
		}
	}
}

void AStarScene::updateScore()
{
    for (auto cell : this->cells)
    {
        auto cellComp = cell->getComponent<ECS::Cell>();
        
        auto startComp = this->startingCell->getComponent<ECS::Cell>();
        auto endComp = this->endingCell->getComponent<ECS::Cell>();
        
        cellComp->g = fabsf(startComp->position.distance(cellComp->position));		// g = Distance from start
        cellComp->h = fabsf(endComp->position.distance(cellComp->position));		// h = Distance from end
        cellComp->f = cellComp->g + cellComp->h;			// f = g + h
    }
}

void AStarScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(AStarScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(AStarScene::onMouseDown, this);
	this->mouseInputListener->onMouseUp = CC_CALLBACK_1(AStarScene::onMouseUp, this);
	this->mouseInputListener->onMouseScroll = CC_CALLBACK_1(AStarScene::onMouseScroll, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(AStarScene::onKeyPressed, this);
	this->keyInputListener->onKeyReleased = CC_CALLBACK_2(AStarScene::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void AStarScene::onMouseMove(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	cocos2d::Vec2 point = cocos2d::Vec2(x, y);

	//cocos2d::log("Moving cursor (%f, %f)", x, y);
	this->hoveringCellSprite->setVisible(true);
	this->hoveringCellSprite->setPosition(point);

	if (this->displayBoundaryBoxNode->displayBoundary.containsPoint(point))
	{
		this->hoveringCellSprite->setVisible(true);
		this->hoveringCellSprite->setPosition(this->cursorPointToCellPos(point));

		if (this->draggingStart)
		{
			this->draggingStartSprite->setPosition(this->cursorPointToCellPos(point));
		}

		if (this->draggingEnd)
		{
			this->draggingEndSprite->setPosition(this->cursorPointToCellPos(point));
		}
	}
	else
	{
		if (this->hoveringCellSprite->isVisible())
		{
			this->hoveringCellSprite->setVisible(false);
		}
	}
}

void AStarScene::onMouseDown(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	int mouseButton = mouseEvent->getMouseButton();
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	cocos2d::Vec2 point = cocos2d::Vec2(x, y);

	if (this->displayBoundaryBoxNode->displayBoundary.containsPoint(point))
	{
		if (mouseButton == 0)
		{
			for (auto cell : this->cells)
			{
				auto comp = cell->getComponent<ECS::Cell>();
				if (comp->cellSprite->getBoundingBox().containsPoint(point))
				{
					switch (comp->state)
					{
					case ECS::Cell::STATE::EMPTY:
						comp->setState(ECS::Cell::STATE::BLOCK);
						break;
					case ECS::Cell::STATE::BLOCK:
						comp->setState(ECS::Cell::STATE::EMPTY);
						break;
					case ECS::Cell::STATE::START:
						this->draggingStart = true;
						this->draggingStartSprite->setVisible(true);
						this->draggingStartSprite->setPosition(this->cursorPointToCellPos(point));
						comp->setState(ECS::Cell::STATE::EMPTY);
						break;
					case ECS::Cell::STATE::END:
						this->draggingEnd = true;
						this->draggingEndSprite->setVisible(true);
						this->draggingEndSprite->setPosition(this->cursorPointToCellPos(point));
						comp->setState(ECS::Cell::STATE::EMPTY);
						break;
					case ECS::Cell::STATE::PATH:
					case ECS::Cell::STATE::OPENED:
					case ECS::Cell::STATE::CLOSED:
					default:
						break;
					}
					break;
				}
			}
		}
	}
}

void AStarScene::onMouseUp(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	int mouseButton = mouseEvent->getMouseButton();
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	auto point = cocos2d::Vec2(x, y);

	if (mouseButton == 0)
	{
		if (this->displayBoundaryBoxNode->displayBoundary.containsPoint(point))
		{
			for (auto cell : this->cells)
			{
				auto comp = cell->getComponent<ECS::Cell>();
				if (comp->cellSprite->getBoundingBox().containsPoint(point))
				{
					if (comp->state == ECS::Cell::STATE::EMPTY)
					{
						if (this->draggingStart)
						{
							if (comp->position == this->startingCell->getComponent<ECS::Cell>()->position)
							{
								cancelDragging();
							}
							else
							{
								comp->setState(ECS::Cell::STATE::START);
								this->startingCell->getComponent<ECS::Cell>()->setState(ECS::Cell::STATE::EMPTY);
								this->startingCell = cell;
								this->draggingStart = false;
								this->draggingStartSprite->setVisible(false);
                                this->updateScore();
							}
							break;
						}

						if (this->draggingEnd)
						{
							if (comp->position == this->endingCell->getComponent<ECS::Cell>()->position)
							{
								cancelDragging();
							}
							else
							{
								comp->setState(ECS::Cell::STATE::END);
								this->endingCell->getComponent<ECS::Cell>()->setState(ECS::Cell::STATE::EMPTY);
								this->endingCell = cell;
								this->draggingEnd = false;
								this->draggingEndSprite->setVisible(false);
                                this->updateScore();
							}
							break;
						}
					}
					else
					{
						cancelDragging();
					}
				}
			}
		}
		else
		{
			cancelDragging();
		}
	}

}

void AStarScene::onMouseScroll(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//float x = mouseEvent->getScrollX();
	//float y = mouseEvent->getScrollY();
}

void AStarScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE)
	{
		cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_LEFT_SHIFT)
	{
		this->shiftPressing = true;
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_SPACE)
	{
		this->pause = !this->pause;
		if (pause)
		{
			this->unscheduleUpdate();
		}
		else
		{
			this->scheduleUpdate();
		}
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ENTER)
	{
		if (finished)
		{
            if(this->cleared)
            {
                // Algorithm is finished.
                this->resetPathFinding();
                finished = false;
                if (this->shiftPressing)
                {
                    // Step mode
                    this->unscheduleUpdate();
                    this->findPath();
                }
                else
                {
                    this->scheduleUpdate();
                    this->updateScheduled = true;
                }
                
                this->cleared = false;
            }
		}
		else
		{
			// Algorithm is running
			if (this->shiftPressing)
			{
                if(this->updateScheduled)
                {
                    this->unscheduleUpdate();
                    this->updateScheduled = false;
                }
				this->findPath();
			}
			else
            {
                if(!this->updateScheduled)
                {
                    this->scheduleUpdate();
                    this->updateScheduled = true;
                }
			}
		}
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_F)
	{
		ECS::Cell::labelState = ECS::Cell::LABEL_STATE::F;
		for (auto cell : this->cells)
		{
			auto comp = cell->getComponent<ECS::Cell>();
			if (comp->state == ECS::Cell::STATE::OPENED || comp->state == ECS::Cell::STATE::CLOSED)
			{
            }
            comp->cellLabel->setString(std::to_string(static_cast<int>(comp->f)));
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_G)
	{
		ECS::Cell::labelState = ECS::Cell::LABEL_STATE::G;
		for (auto cell : this->cells)
		{
			auto comp = cell->getComponent<ECS::Cell>();
			if (comp->state == ECS::Cell::STATE::OPENED || comp->state == ECS::Cell::STATE::CLOSED)
			{
            }
            comp->cellLabel->setString(std::to_string(static_cast<int>(comp->g)));
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_H)
	{
		ECS::Cell::labelState = ECS::Cell::LABEL_STATE::H;
		for (auto cell : this->cells)
		{
			auto comp = cell->getComponent<ECS::Cell>();
			if (comp->state == ECS::Cell::STATE::OPENED || comp->state == ECS::Cell::STATE::CLOSED)
			{
            }
            comp->cellLabel->setString(std::to_string(static_cast<int>(comp->h)));
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_R)
	{
		ECS::Cell::labelState = ECS::Cell::LABEL_STATE::NONE;

		for (auto cell : this->cells)
		{
			auto comp = cell->getComponent<ECS::Cell>();
			comp->setState(ECS::Cell::STATE::EMPTY);
		}
        
        this->startingCell->getComponent<ECS::Cell>()->setState(ECS::Cell::STATE::START);
        this->endingCell->getComponent<ECS::Cell>()->setState(ECS::Cell::STATE::END);
        
        this->pause = false;
        this->finished = true;
        this->elapsedTime = 0;
        
        this->openSet.clear();
        this->path.clear();

		cancelDragging();
        
        if(this->updateScheduled)
        {
            this->updateScheduled = false;
            this->unscheduleUpdate();
        }
        
        this->updateScore();
        
        this->cleared = true;
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_C)
	{
		for (auto cell : this->cells)
		{
			auto comp = cell->getComponent<ECS::Cell>();
			if (comp->state == ECS::Cell::STATE::BLOCK)
			{
				comp->setState(ECS::Cell::STATE::EMPTY);
			}
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_D)
	{
		this->allowDiagonal = !this->allowDiagonal;
	}
}

void AStarScene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_LEFT_SHIFT)
	{
		this->shiftPressing = false;
	}
}

void AStarScene::releaseInputListeners()
{
	if(this->mouseInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->mouseInputListener);
	if(this->keyInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->keyInputListener);
}

void AStarScene::onExit()
{
	cocos2d::Scene::onExit();
	releaseInputListeners(); 

	ECS::Manager::deleteInstance();
}
