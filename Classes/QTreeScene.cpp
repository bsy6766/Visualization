#include "QTreeScene.h"
#include "MainScene.h"
#include "Utility.h"

USING_NS_CC;
using namespace ECS;

QTreeScene* QTreeScene::createScene()
{
	QTreeScene* newQTreeScene = QTreeScene::create();
	return newQTreeScene;
}

bool QTreeScene::init()
{
	if (!CCScene::init())
	{
		return false;
	}

	ECS::Entity::idCounter = 0;
	
	//init node
	this->areaNode = cocos2d::Node::create();
	this->areaNode->setPosition(cocos2d::Vec2::ZERO);
	this->areaNode->retain();
	this->addChild(this->areaNode);

	this->qTreeLineNode = QTreeLineNode::createNode();
	this->qTreeLineNode->setPosition(cocos2d::Vec2::ZERO);
	this->qTreeLineNode->retain();
	this->addChild(this->qTreeLineNode);

	// init flags
	pause = false;
	duplicationCheck = true;
	showGrid = true;
	collisionResolve = false;

	// init index
	lastTrackingEntityID = -1;
	
	// Get window size
	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	// init action
	this->clickAnimation = cocos2d::Sequence::create(cocos2d::ScaleTo::create(0, 0.85f), cocos2d::DelayTime::create(0.25f), cocos2d::ScaleTo::create(0, 1.0f), nullptr);
	this->clickAnimation->retain();

	// Get boundary
	this->displayBoundary = cocos2d::Rect(0, 0, winSize.height, winSize.height);

	// Init label
	this->backLabel = cocos2d::Label::createWithTTF("BACK(ESC)", "fonts/Marker Felt.ttf", 30);
	this->backLabel->setPosition(cocos2d::Vec2(winSize.width - 75.0f, 20.0f));
	this->addChild(this->backLabel);

	// init numbers
	entityCount = 0;
	collisionsCount = 0;
	collisionChecksCount = 0;
	bruteforceChecksCount = 0;
	collisionCheckWithOutRepeatCount = 0;
	fps = 0;
	fpsElapsedTime = 0;

	// init more labels
	entityCountLabel = cocos2d::Label::createWithTTF("Entities: " + std::to_string(entityCount) , "fonts/Marker Felt.ttf", 25);
	entityCountLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	entityCountLabel->setPosition(cocos2d::Vec2(winSize.height + 20.0f, winSize.height - 20.0f));
	this->addChild(entityCountLabel);
	
	collisionChecksCountLabel = cocos2d::Label::createWithTTF("Collision check: " + std::to_string(collisionChecksCount), "fonts/Marker Felt.ttf", 25);
	collisionChecksCountLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	collisionChecksCountLabel->setPosition(cocos2d::Vec2(winSize.height + 20.0f, winSize.height - 50.0f));
	this->addChild(collisionChecksCountLabel);

	collisionCheckWithOutRepeatCountLabel = cocos2d::Label::createWithTTF("Collision check w/o duplication: " + std::to_string(bruteforceChecksCount), "fonts/Marker Felt.ttf", 25);
	collisionCheckWithOutRepeatCountLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	collisionCheckWithOutRepeatCountLabel->setPosition(cocos2d::Vec2(winSize.height + 20.0f, winSize.height - 80.0f));
	this->addChild(collisionCheckWithOutRepeatCountLabel);

	bruteforceChecksCountLabel = cocos2d::Label::createWithTTF("Brute-froce collision check: " + std::to_string(bruteforceChecksCount), "fonts/Marker Felt.ttf", 25);
	bruteforceChecksCountLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	bruteforceChecksCountLabel->setPosition(cocos2d::Vec2(winSize.height + 20.0f, winSize.height - 110.0f));
	this->addChild(bruteforceChecksCountLabel);

	quadtreeLevelLabel = cocos2d::Label::createWithTTF("Quadtree max level: " + std::to_string(0), "fonts/Marker Felt.ttf", 25);
	quadtreeLevelLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	quadtreeLevelLabel->setPosition(cocos2d::Vec2(winSize.height + 20.0f, winSize.height - 140.0f));
	this->addChild(quadtreeLevelLabel);

	fpsLabel = cocos2d::Label::createWithTTF("FPS: " + std::to_string(cocos2d::Director::getInstance()->getFrameRate()), "fonts/Marker Felt.ttf", 25);
	fpsLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	fpsLabel->setPosition(cocos2d::Vec2(winSize.height + 20.0f, 20.0f));
	this->addChild(fpsLabel);

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("Keys                        (Green = enabled)", "fonts/Marker Felt.ttf", 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("Space = Toggle update", "fonts/Marker Felt.ttf", 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("C = Clear all entities", "fonts/Marker Felt.ttf", 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("A = Add 10 entities", "fonts/Marker Felt.ttf", 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("E = Remove 10 entities(FIFO)", "fonts/Marker Felt.ttf", 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("G = Toggle quadtree grid", "fonts/Marker Felt.ttf", 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->usageLabels.back()->setColor(cocos2d::Color3B::GREEN);
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("D = Toggle duplication check", "fonts/Marker Felt.ttf", 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->usageLabels.back()->setColor(cocos2d::Color3B::GREEN);
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("R = Toggle collision resolve", "fonts/Marker Felt.ttf", 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("1 = Increase quadtree max level", "fonts/Marker Felt.ttf", 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("2 = Decrease quadtree max level", "fonts/Marker Felt.ttf", 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("Mouse", "fonts/Marker Felt.ttf", 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("Left Click (in box) = Add entity", "fonts/Marker Felt.ttf", 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("Left Click (on Entity) = Display queried near entities", "fonts/Marker Felt.ttf", 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("Right click (on entity) = Remove entity", "fonts/Marker Felt.ttf", 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	cocos2d::Vec2 usageStartPos = cocos2d::Vec2(winSize.height + 20.0f, winSize.height * 0.65f);

	auto usageLabelSize = static_cast<int>(this->usageLabels.size());
	for (int i = 0; i < usageLabelSize; i++)
	{
		cocos2d::Vec2 newPos = usageStartPos;
		newPos.y -= (20.0f * static_cast<float>(i));
		this->usageLabels.at(i)->setPosition(newPos);
	}
	
	return true;
}

void QTreeScene::initEntitiesAndQTree()
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
	QTree::lineNode = this->qTreeLineNode;
	// Init quadtree with initial boundary
	this->quadTree = new QTree(this->displayBoundary, 0);
}

Entity* QTreeScene::createNewEntity()
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
	newEntity->components[SPRITE] = spriteComp;
	newEntity->components[QTREE_OBJECT] = new QTreeObject();
	return newEntity;
}

void QTreeScene::onEnter()
{
	cocos2d::CCScene::onEnter();

	initEntitiesAndQTree();
	
	initInputListeners();

	this->scheduleUpdate();
}

void QTreeScene::update(float delta)
{
	// Updates fps count and time 
	updateFPS(delta);

	// Skip entire update process if entities is empty
	if (this->entities.empty())
	{
		return;
	}

	resetQTreeAndUpdatePosition(delta);

	checkCollision();

	updateLabels();
}

void QTreeScene::updateFPS(float delta)
{
	this->fpsElapsedTime += delta;
	if (this->fpsElapsedTime > 1.0f)
	{
		this->fpsElapsedTime -= 1.0f;
		fps++;
		fpsLabel->setString("FPS: " + std::to_string(fps) + " (" + std::to_string(delta).substr(0, 5) + "ms)");
		fps = 0;
	}
	else
	{
		fps++;
	}
}

void QTreeScene::resetQTreeAndUpdatePosition(float delta)
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
		auto qTreeObjComp = (*it)->getComponent<QTreeObject*>(QTREE_OBJECT); 
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

	if (showGrid)
	{
		// Show grids
		this->quadTree->showLines();
	}
}

void QTreeScene::checkCollision()
{
	// If simulation is paused, no need to check collision
	if (pause) return;

	// Reset counters
	collisionChecksCount = 0;
	collisionCheckWithOutRepeatCount = 0;

	for (auto entity : this->entities)
	{
		// Get entity's component
		auto entitySpriteComp = entity->getComponent<ECS::Sprite*>(SPRITE);
		auto entityQTreeObjectComp = entity->getComponent<QTreeObject*>(QTREE_OBJECT);
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
			auto nearEntityQTreeObjectComp = nearEntity->getComponent<QTreeObject*>(QTREE_OBJECT);
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
}

void QTreeScene::checkBoundary(ECS::Sprite & spriteComp, bool& flipX, bool& flipY)
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

void QTreeScene::flipDirVec(const bool flipX, const bool flipY, cocos2d::Vec2& dirVec)
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

void QTreeScene::updateLabels()
{
	entityCount = this->entities.size();
	entityCountLabel->setString("Entities: " + std::to_string(entityCount) + " / 1000");

	collisionChecksCountLabel->setString("Collision check: " + std::to_string(collisionChecksCount));
	if (duplicationCheck)
		collisionCheckWithOutRepeatCountLabel->setString("Collision check w/o duplication: " + std::to_string(collisionCheckWithOutRepeatCount));

	bruteforceChecksCount = entityCount * entityCount;
	bruteforceChecksCountLabel->setString("Brute-froce collision check: " + std::to_string(bruteforceChecksCount));

	quadtreeLevelLabel->setString("Quadtree max level: " + std::to_string(this->quadTree->getCurrentLevelSetting()));
}

void QTreeScene::resolveCollisions(ECS::Sprite & entitySpriteComp, ECS::Sprite & nearEntitySpriteComp, ECS::DirectionVector& entityDirVecComp, ECS::DirectionVector& nearEntityDirVecComp)
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

void QTreeScene::reassignEntityIds()
{
	int counter = 0;
	for (auto entity : this->entities)
	{
		entity->id = counter;
		counter++;
	}
}

void QTreeScene::playUIAnimation(const USAGE_KEY usageKey)
{
	this->usageLabels.at(static_cast<int>(usageKey))->stopAllActions();
	this->usageLabels.at(static_cast<int>(usageKey))->setScale(1.0f);
	this->usageLabels.at(static_cast<int>(usageKey))->runAction(this->clickAnimation);
}

void QTreeScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(QTreeScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(QTreeScene::onMouseDown, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(QTreeScene::onKeyPressed, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void QTreeScene::onMouseMove(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	if (this->backLabel->getBoundingBox().containsPoint(cocos2d::Vec2(x, y)))
	{
		this->backLabel->setScale(1.2f);
	}
	else
	{
		if (this->backLabel->getScale() > 1.0f)
		{
			this->backLabel->setScale(1.0f);
		}
	}
}

void QTreeScene::onMouseDown(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	int mouseButton = mouseEvent->getMouseButton();
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	auto point = cocos2d::Vec2(x, y);

	if (this->backLabel->getBoundingBox().containsPoint(point))
	{
		cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));

		return;
	}

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
					auto entityQTreeObjectComp = entity->getComponent<QTreeObject*>(QTREE_OBJECT);
					auto entityDirVecComp = entity->getComponent<DirectionVector*>(DIRECTION_VECTOR);

					if (entitySpriteComp->sprite->getBoundingBox().containsPoint(point))
					{
						// Clicked on entitiy sprite
						this->playUIAnimation(USAGE_KEY::TRACK);

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
									auto comp = lastEntity->getComponent<ECS::QTreeObject*>(QTREE_OBJECT);
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

				this->playUIAnimation(USAGE_KEY::ADD_ONE);
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
				this->playUIAnimation(USAGE_KEY::REMOVE_ONE);
			}
		}
	}
}

void QTreeScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
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
		this->playUIAnimation(USAGE_KEY::INC_QTREE_LEVEL);
	}
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_2)
	{
		// Decrease quadtree level
		this->quadTree->decreaseLevel();
		this->playUIAnimation(USAGE_KEY::DEC_QTREE_LEVEL);
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_SPACE)
	{
		// Toggle pause simulation. Still counts fps and entity
		pause = !pause;
		if (pause)
		{
			this->usageLabels.at(static_cast<int>(USAGE_KEY::SPACE))->setColor(cocos2d::Color3B::GREEN);
		}
		else
		{
			this->usageLabels.at(static_cast<int>(USAGE_KEY::SPACE))->setColor(cocos2d::Color3B::WHITE);
		}
		this->playUIAnimation(USAGE_KEY::SPACE);
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_R)
	{
		// Toggle collision resolution
		collisionResolve = !collisionResolve;
		if (collisionResolve)
		{
			this->usageLabels.at(static_cast<int>(USAGE_KEY::COL_RESOLVE))->setColor(cocos2d::Color3B::GREEN);
		}
		else
		{
			this->usageLabels.at(static_cast<int>(USAGE_KEY::COL_RESOLVE))->setColor(cocos2d::Color3B::WHITE);
		}
		this->playUIAnimation(USAGE_KEY::COL_RESOLVE);
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_C)
	{
		// Wipe all entities
		for (auto entity : this->entities)
		{
			entity->alive = false;
		}
		this->playUIAnimation(USAGE_KEY::CLEAR);
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
		this->playUIAnimation(USAGE_KEY::ADD_TEN);
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
		this->playUIAnimation(USAGE_KEY::REMOVE_TEN);
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_G)
	{
		// Toggle quadtree grid
		showGrid = !showGrid;
		if (showGrid)
		{
			this->usageLabels.at(static_cast<int>(USAGE_KEY::GRID))->setColor(cocos2d::Color3B::GREEN);
		}
		else
		{
			this->usageLabels.at(static_cast<int>(USAGE_KEY::GRID))->setColor(cocos2d::Color3B::WHITE);
		}
		this->playUIAnimation(USAGE_KEY::GRID);
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_D)
	{
		// Toggle duplication check
		duplicationCheck = !duplicationCheck;
		collisionCheckWithOutRepeatCount = 0;
		collisionCheckWithOutRepeatCountLabel->setString("Collision check w/o duplication: " + std::to_string(collisionCheckWithOutRepeatCount));
		if (duplicationCheck)
		{
			collisionCheckWithOutRepeatCountLabel->setColor(cocos2d::Color3B::WHITE);
			this->usageLabels.at(static_cast<int>(USAGE_KEY::DUPL_CHECK))->setColor(cocos2d::Color3B::GREEN);
		}
		else
		{
			collisionCheckWithOutRepeatCountLabel->setColor(cocos2d::Color3B::GRAY);
			this->usageLabels.at(static_cast<int>(USAGE_KEY::DUPL_CHECK))->setColor(cocos2d::Color3B::WHITE);
		}
		this->playUIAnimation(USAGE_KEY::DUPL_CHECK);
	}
}

void QTreeScene::releaseInputListeners()
{
	if(this->mouseInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->mouseInputListener);
	if(this->keyInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->keyInputListener);
}

void QTreeScene::onExit()
{
	cocos2d::CCScene::onExit();
	releaseInputListeners();
	this->areaNode->release();
	this->qTreeLineNode->release();

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

	this->clickAnimation->release();
}
