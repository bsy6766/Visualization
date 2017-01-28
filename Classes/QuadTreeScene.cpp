#include "QuadTreeScene.h"
#include "MainScene.h"
#include "Utility.h"

USING_NS_CC;
using namespace ECS;

QuadTreeScene* QuadTreeScene::createScene()
{
	QuadTreeScene* newQTreeScene = QuadTreeScene::create();
	return newQTreeScene;
}

bool QuadTreeScene::init()
{
	if (!cocos2d::Scene::init())
	{
		return false;
	}

	ECS::Entity::idCounter = 0;

	// Limit max entity to 400 in this case
    ECS::Entity::maxEntitySize = 1000;
	
	// init area node where entity boxes get rendered
	this->areaNode = cocos2d::Node::create();
	this->areaNode->setPosition(cocos2d::Vec2::ZERO);
	this->areaNode->retain();
	this->addChild(this->areaNode);

    // init quad tree line node which draws quad tree subdivision lines
	this->quadTreeLineNode = QuadTreeLineNode::createNode();
	this->quadTreeLineNode->setPosition(cocos2d::Vec2::ZERO);
    this->quadTreeLineNode->retain();
    this->quadTreeLineNode->drawNode->setLocalZOrder(static_cast<int>(Z_ORDER::LINE));
    this->addChild(this->quadTreeLineNode);
    
    // init display boundary box node which draws outer line of simulation display box
    this->displayBoundaryBoxNode = DisplayBoundaryBoxNode::createNode();
    this->displayBoundaryBoxNode->setPosition(cocos2d::Vec2::ZERO);
    this->displayBoundaryBoxNode->displayBoundary = cocos2d::Rect(0, 0, 650, 650);
    this->displayBoundaryBoxNode->drawDisplayBoundaryBox();
    this->displayBoundary = this->displayBoundaryBoxNode->displayBoundary;
    this->displayBoundaryBoxNode->retain();
    this->displayBoundaryBoxNode->drawNode->setLocalZOrder(static_cast<int>(Z_ORDER::BOX));
    this->addChild(this->displayBoundaryBoxNode);

	// init flags
	pause = false;
	duplicationCheck = true;
	showGrid = true;
	collisionResolve = false;

	// init index
	lastTrackingEntityID = -1;
	
	// Get window size
	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();
    
    // Init labels node
    this->labelsNode = LabelsNode::createNode();
	this->labelsNode->setSharedLabelPosition(LabelsNode::SHARED_LABEL_POS_TYPE::QUADTREE_SCENE);
    this->addChild(this->labelsNode);

	// Starting pos
	float labelX = winSize.height - 10.0f;
	float labelY = winSize.height - 45.0f;

	// Set title
	this->labelsNode->initTitleStr("Quad Tree", cocos2d::Vec2(labelX, labelY));
	labelY -= 50.0f;

	// Init custom labels
	this->labelsNode->customLabelStartPos = cocos2d::Vec2(labelX, labelY);

	// Set size
	const int customLabelSize = 25;
    
	// Init custom labels
    this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Entities: 0", customLabelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Collision check: 0", customLabelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Collision check w/o duplication : 0", customLabelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Brute-Froce check: 0", customLabelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Quad Tree max level: 0", customLabelSize);
    
	// Calculate next label block y
	const float customLastY = this->labelsNode->customLabels.back()->getBoundingBox().getMinY();
	const float blockGap = 22.0f;

	this->labelsNode->keyboardUsageLabelStartPos = cocos2d::Vec2(labelX, customLastY - blockGap);

	const int headerSize = 25;
	const int labelSize = 20;
    
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Keys (Green = enabled)", headerSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Space:  Toggle update", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "C:  Clear all Entities", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "A:  Add 10 Entities", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "E:  Remove 10 Entities", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "G:  Toggle quad tree subdivision grid", labelSize);
    this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::GRID), cocos2d::Color3B::GREEN);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "D:  Toggle duplication check", labelSize);
    this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::DUPL_CHECK), cocos2d::Color3B::GREEN);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "R:  Toggle collision resolution", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "1:  Increase Quad Tree max level", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "2:  Decrease Quad Tree max level", labelSize);
    
	const float keysLastY = this->labelsNode->keyboardUsageLabels.back()->getBoundingBox().getMinY();
    this->labelsNode->mouseUsageLabelStartPos = cocos2d::Vec2(labelX, keysLastY - blockGap);
    
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Mouse", headerSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Left Click (In box):  Add Entity", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Left Click (On Entity):  Toggle Entity tracking", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Right Click (On Entity):  Remove Entity", labelSize);

	// speed modifier
	this->simulationSpeedModifier = 1.0f;

	this->sliderLabelNode = SliderLabelNode::createNode();
	const float mouseLastY = this->labelsNode->mouseUsageLabels.back()->getBoundingBox().getMinY();
	this->sliderLabelNode->sliderStartPos = cocos2d::Vec2(labelX, mouseLastY - blockGap);
	this->sliderLabelNode->addSlider("Simulation Speed", "Slider", 50, CC_CALLBACK_1(QuadTreeScene::onSliderClick, this));
	this->addChild(this->sliderLabelNode);

	return true;
}

void QuadTreeScene::initEntitiesAndQTree()
{
	// Create 40 entities at start
	const int initialEntityCount = 40;
	for (int i = 0; i < initialEntityCount; i++)
	{
		auto newEntity = createNewEntity();
		if (newEntity != nullptr)
		{
			this->entities.push_back(newEntity);
		}
	} 

	// Store lineNode pointer to quadtree for visualization
	QuadTree::lineDrawNode = this->quadTreeLineNode->drawNode;
	// Init quadtree with initial boundary
	this->quadTree = new QuadTree(this->displayBoundary, 0);
    
    this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::QUAD_TREE_MAX_LEVEL), "Quad Tree max level: " + std::to_string(this->quadTree->getCurrentLevelSetting()));
}

Entity* QuadTreeScene::createNewEntity()
{
	Entity* newEntity = new Entity();
	if (newEntity->id >= Entity::maxEntitySize)
	{
		auto size = this->entities.size();
		if (size == Entity::maxEntitySize)
		{
			// Limit entities by 1000
			delete newEntity;
			return nullptr;
		}
		else
		{
			// We can spawn more entities but id exceeded 1000. reassign it.
			reassignEntityIds();
			newEntity->id = size;
			Entity::idCounter = size + 1;
		}
	}

	// attach component and return
	newEntity->components[DIRECTION_VECTOR] = new ECS::DirectionVector();
	auto spriteComp = new ECS::Sprite(*this->areaNode, "quadTreeEntityBox.png");
	spriteComp->sprite->setScaleX(Utility::Random::randomReal<float>(0.25f, 1.0f));
	spriteComp->sprite->setScaleY(Utility::Random::randomReal<float>(0.25f, 1.0f));
	spriteComp->sprite->setLocalZOrder(static_cast<int>(Z_ORDER::ENTITY));
	newEntity->components[SPRITE] = spriteComp;
	newEntity->components[QTREE_DATA] = new QTreeData();
	return newEntity;
}

void QuadTreeScene::onEnter()
{
	cocos2d::Scene::onEnter();

	initEntitiesAndQTree();
	
	initInputListeners();

	this->scheduleUpdate();
}

void QuadTreeScene::update(float delta)
{
	// Updates fps count and time 
    this->labelsNode->updateFPSLabel(delta);

	// Skip entire update process if entities is empty
	if (this->entities.empty())
	{
		this->labelsNode->updateTimeTakenLabel("0");
		return;
	}

	Utility::Time::start();

	//Speed modifier
	delta *= this->simulationSpeedModifier;

	resetQTreeAndUpdatePosition(delta);

	checkCollision();

	Utility::Time::stop();

	std::string timeTakenStr = Utility::Time::getElaspedTime();	// Microseconds
	float timeTakenF = std::stof(timeTakenStr);	// to float
	timeTakenF *= 0.001f; // To milliseconds

	this->labelsNode->updateTimeTakenLabel(std::to_string(timeTakenF).substr(0, 5));

    int entityCount = static_cast<int>(this->entities.size());
    this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::ENTITIES), "Entities: " + std::to_string(entityCount) + " / 1000");
    this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::BRUTE_FORCE), "Brute-force check: " + std::to_string(entityCount * entityCount));
}

void QuadTreeScene::resetQTreeAndUpdatePosition(float delta)
{
	if (!pause)
	{
		// Only reset quad tree when it's not paused
		this->quadTree->clear();
	}

	auto it = this->entities.begin();
	for (; it != this->entities.end();)
	{
		// Remove if entities is dead
		if ((*it)->alive == false)
		{
			delete (*it);
			it = this->entities.erase(it);
			continue;
		}

		if (pause)
		{
			// if simulation is paused, don't update entitie's position
			it++;
			continue;
		}

		// Get entity's component
		auto dirVecComp = (*it)->getComponent<DirectionVector*>(DIRECTION_VECTOR);
		auto qTreeObjComp = (*it)->getComponent<QTreeData*>(QTREE_DATA); 
		auto spriteComp = (*it)->getComponent<ECS::Sprite*>(SPRITE);

		// Update new position based on direction, speed and time
		cocos2d::Vec2 movedDistance = dirVecComp->dirVec * qTreeObjComp->speed * delta;
		auto newPos = spriteComp->sprite->getPosition() + movedDistance;
		spriteComp->sprite->setPosition(newPos);

		// Check if entity is still in boundary
		bool inBoundary = Utility::containsRect(this->displayBoundary, spriteComp->sprite->getBoundingBox());
		if (!inBoundary)
		{
			// out of boundary
			bool flipX = false;
			bool flipY = false;
			checkBoundary(*spriteComp, flipX, flipY);

			if (flipX || flipY)
			{
				flipDirVec(flipX, flipY, dirVecComp->dirVec);
			}
		}

		if (duplicationCheck)
		{
			// Reset look up table to 0 if duplication check is enabled
			std::fill(qTreeObjComp->visitied.begin(), qTreeObjComp->visitied.end(), 0);
		}

		// Re-insert to quadtree
		this->quadTree->insert((*it));

		// Reset color to white
		spriteComp->sprite->setColor(cocos2d::Color3B::WHITE);

		// next
		it++;
	}
    
    QuadTree::lineDrawNode->clear();
	if (showGrid)
	{
		// Show grids
		this->quadTree->showLines();
	}
}

void QuadTreeScene::checkCollision()
{
	// If simulation is paused, no need to check collision
	if (pause) return;

	// Reset counters
	int collisionChecksCount = 0;
	int collisionCheckWithOutRepeatCount = 0;

	for (auto entity : this->entities)
	{
		// Get entity's component
		auto entitySpriteComp = entity->getComponent<ECS::Sprite*>(SPRITE);
		auto entityQTreeObjectComp = entity->getComponent<QTreeData*>(QTREE_DATA);
		auto entityDirVecComp = entity->getComponent<DirectionVector*>(DIRECTION_VECTOR);

		// Get entity's bounding box
		auto bb = entitySpriteComp->sprite->getBoundingBox();
		
		// Query near entities
		std::list<Entity*> neighbors;
		this->quadTree->queryAllEntities(bb, neighbors);

		// Skip if there is no other entities nearby
		if (neighbors.empty())
		{
			continue;
		}

		// Iterate near entities
		for (auto nearEntity : neighbors)
		{
			// Get components
			auto nearEntityQTreeObjectComp = nearEntity->getComponent<QTreeData*>(QTREE_DATA);
			auto nearEntitySpriteComp = nearEntity->getComponent<ECS::Sprite*>(SPRITE);
			auto nearEntityDirVecComp = nearEntity->getComponent<DirectionVector*>(DIRECTION_VECTOR);

			// Increment collision check count. 
			collisionChecksCount++;

			if (duplicationCheck)
			{
				// Duplication check enabled. Skip if comparing same entities
				if (entity->id != nearEntity->id)
				{
					// Mark near entitiy as 'visitied'
					entityQTreeObjectComp->visitied.at(nearEntity->id) = 1;

					// See if near entitiy already checked collision with this entity
					bool alreadyChecked = nearEntityQTreeObjectComp->visitied.at(entity->id) ? true : false;

					if(!alreadyChecked)
					{
						// colliding entitiy havent visited entity yet.
						nearEntityQTreeObjectComp->visitied.at(entity->id) = 1;

						// Check collision
						if (entitySpriteComp->sprite->getBoundingBox().intersectsRect(nearEntitySpriteComp->sprite->getBoundingBox()))
						{
							// Color colliding entity with red
							entitySpriteComp->sprite->setColor(cocos2d::Color3B::RED);
							nearEntitySpriteComp->sprite->setColor(cocos2d::Color3B::RED);

							if (collisionResolve)
							{
								// Resolve collision
								resolveCollisions(*entitySpriteComp, *nearEntitySpriteComp, *entityDirVecComp, *nearEntityDirVecComp);
							}
						}
						// Increment counter.
						collisionCheckWithOutRepeatCount++;
					}
					//else, near entity already checked collision with this entity
				}
				//else, comparing same entities.
			}
			else
			{
				// Doesn't check duplication, skip if comparing same entity
				if (entity->id != nearEntity->id)
				{
					// Get both bounding box
					auto eBB = entitySpriteComp->sprite->getBoundingBox();
					auto nBB = nearEntitySpriteComp->sprite->getBoundingBox();

					if (eBB.intersectsRect(nBB))
					{
						// Color colliding entity with red
						entitySpriteComp->sprite->setColor(cocos2d::Color3B::RED);
						nearEntitySpriteComp->sprite->setColor(cocos2d::Color3B::RED);

						if (collisionResolve)
						{
							// Resolve collision
							resolveCollisions(*entitySpriteComp, *nearEntitySpriteComp, *entityDirVecComp, *nearEntityDirVecComp);
						}
					}
				}
			}

			if (entityQTreeObjectComp->tracking)
			{
				// Mark near entity color with green if entity is tracking
				nearEntitySpriteComp->sprite->setColor(cocos2d::Color3B::GREEN);
			}

		}

		if (entityQTreeObjectComp->tracking)
		{
			// if entity is tracking, mark color with blue
			entitySpriteComp->sprite->setColor(cocos2d::Color3B::BLUE);
		}
	}
    
    this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::COLLISION), "Collision check: " + std::to_string(collisionChecksCount));
    this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::COLLISION_WO_DUP_CHECK), "Collision check w/o duplication: " + std::to_string(collisionCheckWithOutRepeatCount));
}

void QuadTreeScene::checkBoundary(ECS::Sprite & spriteComp, bool& flipX, bool& flipY)
{
	// Get boundary
	const auto bb = spriteComp.sprite->getBoundingBox();

	if (bb.getMinX() < displayBoundary.getMinX())
	{
		// out left. push entity back to boundary
		float diff = displayBoundary.getMinX() - bb.getMinX();
		auto curPos = spriteComp.sprite->getPosition();
		curPos.x += diff + 0.1f;
		spriteComp.sprite->setPosition(curPos);
		flipX = true;
	}
	else if (bb.getMaxX() > displayBoundary.getMaxX())
	{
		// out right. push entity back to boundary
		float diff = bb.getMaxX() - displayBoundary.getMaxX();
		auto curPos = spriteComp.sprite->getPosition();
		curPos.x -= diff + 0.1f;
		spriteComp.sprite->setPosition(curPos);
		flipX = true;
	}

	if (bb.getMinY() < displayBoundary.getMinY())
	{
		// out bottom.
		float diff = displayBoundary.getMinY() - bb.getMinY();
		auto curPos = spriteComp.sprite->getPosition();
		curPos.y += diff + 0.1f;
		spriteComp.sprite->setPosition(curPos);
		flipY = true;
	}
	else if (bb.getMaxY() > displayBoundary.getMaxY())
	{
		// out right. push entity back to boundary
		float diff = bb.getMaxY() - displayBoundary.getMaxY();
		auto curPos = spriteComp.sprite->getPosition();
		curPos.y -= diff + 0.1f;
		spriteComp.sprite->setPosition(curPos);
		flipY = true;
	}

}

void QuadTreeScene::flipDirVec(const bool flipX, const bool flipY, cocos2d::Vec2& dirVec)
{
	if (flipX)
	{
		// Flip x direction
		dirVec.x *= -1.0f;
		if (!flipY)
		{
			// For y, make it random for fun
			dirVec.y *= Utility::Random::random_minus_1_1();
		}
	}

	if (flipY)
	{
		dirVec.y *= -1.0f;
		if (!flipX)
		{
			// For x, make if random for fun
			dirVec.x *= Utility::Random::random_minus_1_1();
		}
	}
}

void QuadTreeScene::resolveCollisions(ECS::Sprite & entitySpriteComp, ECS::Sprite & nearEntitySpriteComp, ECS::DirectionVector& entityDirVecComp, ECS::DirectionVector& nearEntityDirVecComp)
{
	auto eBB = entitySpriteComp.sprite->getBoundingBox();
	auto nBB = nearEntitySpriteComp.sprite->getBoundingBox();

	auto bb = Utility::getIntersectingRect(eBB, nBB);

	auto ePos = entitySpriteComp.sprite->getPosition();
	auto nPos = nearEntitySpriteComp.sprite->getPosition();

	bool flipX = false;
	bool flipY = false;

	if (bb.size.width < bb.size.height)
	{
		// hit from left and right
		float halfWidth = bb.size.width * 0.5f + 0.1f;

		if (eBB.getMidX() < nBB.getMidX())
		{
			// entity is on left and near entity is on right
			ePos.x -= halfWidth;
			nPos.x += halfWidth;
		}
		else
		{
			ePos.x += halfWidth;
			nPos.x -= halfWidth;
		}

		flipX = true;
		flipY = Utility::Random::randomInt100() > 50 ? true : false;
	}
	else if (bb.size.width > bb.size.height)
	{
		// hit from top and bottom
		float halfHeight = bb.size.height * 0.5f + 0.1f;
		if (eBB.getMidY() < nBB.getMidY())
		{
			// entity is lower than near entity
			ePos.x -= halfHeight;
			nPos.x += halfHeight;
		}
		else
		{
			ePos.x += halfHeight;
			nPos.x -= halfHeight;
		}

		flipX = Utility::Random::randomInt100() > 50 ? true : false;
		flipY = true;
	}
	// Else, Diagonally hit. Happens really rarely. just ignore.
	entitySpriteComp.sprite->setPosition(ePos);
	nearEntitySpriteComp.sprite->setPosition(nPos);

	flipDirVec(flipX, flipY, entityDirVecComp.dirVec);
	flipDirVec(flipX, flipY, nearEntityDirVecComp.dirVec);
}

void QuadTreeScene::reassignEntityIds()
{
	int counter = 0;
	for (auto entity : this->entities)
	{
		entity->id = counter;
		counter++;
	}
}

void QuadTreeScene::toggleColor(const bool enabled, LabelsNode::TYPE type, const int index, const bool playAniamtion)
{
    if(enabled)
    {
        this->labelsNode->setColor(type, index, cocos2d::Color3B::GREEN, playAniamtion);
    }
    else
    {
        this->labelsNode->setColor(type, index, cocos2d::Color3B::WHITE, playAniamtion);
    }
}

void QuadTreeScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(QuadTreeScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(QuadTreeScene::onMouseDown, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(QuadTreeScene::onKeyPressed, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void QuadTreeScene::onMouseMove(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();
    
    this->labelsNode->updateMouseHover(cocos2d::Vec2(x, y));
}

void QuadTreeScene::onMouseDown(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	int mouseButton = mouseEvent->getMouseButton();
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	auto point = cocos2d::Vec2(x, y);

    bool ret = this->labelsNode->updateMouseDown(point);
    if(ret) return;

	if (mouseButton == 0)
	{
		// Left click
		if (this->displayBoundary.containsPoint(point))
		{
			// In display boundary
			auto bb = cocos2d::Rect();
			bb.origin = cocos2d::Vec2(point.x - 5.0f, point.y - 5.0f);
			bb.size = cocos2d::Vec2(10.0f, 10.0f);

			std::list<Entity*> nearEntities;

			// Query near entities
			this->quadTree->queryAllEntities(bb, nearEntities);

			if (!nearEntities.empty())
			{
				for (auto entity : nearEntities)
				{
					auto entitySpriteComp = entity->getComponent<ECS::Sprite*>(SPRITE);
					auto entityQTreeObjectComp = entity->getComponent<QTreeData*>(QTREE_DATA);

					if (entitySpriteComp->sprite->getBoundingBox().containsPoint(point))
					{
						// Clicked on entitiy sprite
                        this->labelsNode->playAnimation(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::TRACK));

						if (this->lastTrackingEntityID == entity->id)
						{
							// Already tracking same entity. Disable tracking.
							entityQTreeObjectComp->tracking = false;
							this->lastTrackingEntityID = -1;
							return;
						}

						// New entity to track
						entityQTreeObjectComp->tracking = true;

						if (this->lastTrackingEntityID >= 0)
						{
							// Already has other entity tracking.
							for (auto lastEntity : this->entities)
							{
								if (lastEntity->id == this->lastTrackingEntityID)
								{
									// Disable tracking on last tracking entitiy
									auto comp = lastEntity->getComponent<ECS::QTreeData*>(QTREE_DATA);
									comp->tracking = false;
									break;
								}
							}
						}

						// Set this entity to new tracking entity
						this->lastTrackingEntityID = entity->id;

						return;
					}
				}
			}

			// Spawn new entity
			auto newEntity = createNewEntity();
			if (newEntity != nullptr)
			{
				this->entities.push_back(newEntity);
				auto newEntitySpriteComp = this->entities.back()->getComponent<ECS::Sprite*>(SPRITE);
				newEntitySpriteComp->sprite->setPosition(point);
                
                this->labelsNode->playAnimation(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::ADD_ONE));
			}


		}
	}
	else if (mouseButton == 1)
	{
		for (auto entity : this->entities)
		{
			auto entitySpriteComp = entity->getComponent<ECS::Sprite*>(SPRITE);
			if (entitySpriteComp->sprite->getBoundingBox().containsPoint(point))
			{
				entity->alive = false;
                
                this->labelsNode->playAnimation(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::REMOVE_ONE));
			}
		}
	}
}

void QuadTreeScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE)
	{
		// Terminate 
		cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_1)
	{
		// Increase quadtree level
        this->quadTree->increaseLevel();
        this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::QUAD_TREE_MAX_LEVEL), "Quad Tree max level: " + std::to_string(this->quadTree->getCurrentLevelSetting()));
        
        this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::INC_QTREE_LEVEL));
	}
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_2)
	{
		// Decrease quadtree level
        this->quadTree->decreaseLevel();
        this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::QUAD_TREE_MAX_LEVEL), "Quad Tree max level: " + std::to_string(this->quadTree->getCurrentLevelSetting()));
        
        this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::DEC_QTREE_LEVEL));
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_SPACE)
	{
		// Toggle pause simulation. Still counts fps and entity
		pause = !pause;
		if (pause)
		{
			this->labelsNode->updateTimeTakenLabel("0");
		}
        this->toggleColor(pause, LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::PAUSE));
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_R)
	{
		// Toggle collision resolution
        collisionResolve = !collisionResolve;
        this->toggleColor(collisionResolve, LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::COL_RESOLVE));
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_C)
	{
		// Wipe all entities
		for (auto entity : this->entities)
		{
			entity->alive = false;
		}
        
        this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::CLEAR));

		this->labelsNode->updateTimeTakenLabel("0");
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_A)
	{
		// Add ten entities
		for (int i = 0; i < 10; i++)
		{
			auto newEntity = createNewEntity();
			if (newEntity != nullptr)
			{
				this->entities.push_back(newEntity);
				auto spriteComp = this->entities.back()->getComponent<ECS::Sprite*>(SPRITE);
				spriteComp->setRandomPosInBoundary(this->displayBoundary);
			}
		}
        
        this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::ADD_TEN));
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_E)
	{
		// Remove last 10 entities
		int count = 0;
		for (auto entity : this->entities)
		{
			if (count == 10)
			{
				break;
			}
			entity->alive = false;
			count++;
        }
        
        this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::REMOVE_TEN));
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_G)
	{
		// Toggle quadtree grid
		showGrid = !showGrid;
        this->toggleColor(showGrid, LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::GRID));
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_D)
	{
		// Toggle duplication check
		duplicationCheck = !duplicationCheck;
        if(duplicationCheck)
        {
            this->labelsNode->setColor(LabelsNode::TYPE::CUSTOM, static_cast<int>(CUSTOM_LABEL_INDEX::COLLISION_WO_DUP_CHECK), cocos2d::Color3B::WHITE, false);
        }
        else
        {
            this->labelsNode->setColor(LabelsNode::TYPE::CUSTOM, static_cast<int>(CUSTOM_LABEL_INDEX::COLLISION_WO_DUP_CHECK), cocos2d::Color3B::GRAY, false);
        }
        
        this->toggleColor(duplicationCheck, LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::DUPL_CHECK));
	}
}

void QuadTreeScene::releaseInputListeners()
{
	if(this->mouseInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->mouseInputListener);
	if(this->keyInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->keyInputListener);
}

void QuadTreeScene::onSliderClick(cocos2d::Ref* sender)
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

void QuadTreeScene::onExit()
{
	cocos2d::Scene::onExit();
	releaseInputListeners();
	this->areaNode->release();
	this->quadTreeLineNode->release();
    this->displayBoundaryBoxNode->release();

	// Delete all entities
	for (auto entity : this->entities)
	{
		if (entity != nullptr)
		{
			delete entity;
		}
	}

	// Delete quadtree
	if (this->quadTree != nullptr)
	{
		delete quadTree;
	}
}
