#include "FlockingScene.h"
#include "MainScene.h"

USING_NS_CC;

FlockingScene* FlockingScene::createScene()
{
	FlockingScene* newFlockingScene = FlockingScene::create();
	return newFlockingScene;
}

bool FlockingScene::init()
{
    if (!cocos2d::Scene::init())
	{
		return false;
	}

	ECS::Entity::idCounter = 0;
    
    // Init mouse point
    this->curMousePosition = cocos2d::Vec2(-1.0f, -1.0f);

	// Uncomment this to activate update(float) function
	this->scheduleUpdate();

	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	this->displayBoundary = cocos2d::Rect(0, 0, winSize.height, winSize.height);

	std::string fontPath = "fonts/Rubik-Medium.ttf";

	this->areaNode = cocos2d::Node::create();
	this->areaNode->setPosition(cocos2d::Vec2::ZERO);
	this->areaNode->retain();
	this->addChild(this->areaNode);

	// init action
	this->clickAnimation = cocos2d::Sequence::create(cocos2d::ScaleTo::create(0, 0.85f), cocos2d::DelayTime::create(0.25f), cocos2d::ScaleTo::create(0, 1.0f), nullptr);
	this->clickAnimation->retain();

	this->blackArea = cocos2d::Sprite::createWithSpriteFrameName("dot.png");
	this->blackArea->setColor(cocos2d::Color3B::BLACK);
	this->blackArea->setScaleX(winSize.height * 0.5f);
	this->blackArea->setScaleY(winSize.height);
	this->blackArea->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->blackArea->setPosition(cocos2d::Vec2(winSize.height, winSize.height * 0.5f));
	this->addChild(this->blackArea);

	this->backLabel = cocos2d::Label::createWithTTF("BACK(ESC)", fontPath, 20);
	this->backLabel->setPosition(cocos2d::Vec2(winSize.width - 60.0f, 20.0f));
	this->addChild(this->backLabel);

	float labelX = winSize.height + 5.0f;

	entityCountLabel = cocos2d::Label::createWithTTF("Entities: " + std::to_string(this->entities.size()), fontPath, 25);
	entityCountLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	entityCountLabel->setPosition(cocos2d::Vec2(labelX, winSize.height - 20.0f));
	this->addChild(entityCountLabel);

	this->weightLabel = cocos2d::Label::createWithTTF("WEIGHTS (Click buttons to modify)", fontPath, 25);
	this->weightLabel->setAnchorPoint(cocos2d::Vec2(0, 1.0f));
	this->weightLabel->setPosition(cocos2d::Vec2(labelX, winSize.height - 35.0f));
	this->addChild(this->weightLabel);

	float weightLabelY = winSize.height - 50.0f;
	float leftButtonX = winSize.height + 160.0f;
	float weightLabelX = leftButtonX + 30.0f;
	float rightButtonX = leftButtonX + 60.0f;
	float weightLabelYOffset = 25.0f;
	float buttonYOffset = 25.0f;

	this->alignmentLabel = cocos2d::Label::createWithTTF("ALIGNMENT", fontPath, 20);
	this->alignmentLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->alignmentLabel->setPosition(cocos2d::Vec2(labelX, weightLabelY - weightLabelYOffset));
	this->addChild(this->alignmentLabel);

	this->alignmentWeightLabel = cocos2d::Label::createWithTTF(std::to_string(ECS::FlockingData::ALIGNMENT_WEIGHT).substr(0, 3), fontPath, 20);
	this->alignmentWeightLabel->setPosition(cocos2d::Vec2(weightLabelX, weightLabelY - weightLabelYOffset));
	this->addChild(this->alignmentWeightLabel);

	this->leftAlignmentButton = cocos2d::ui::Button::create("leftButton.png", "leftSelectedButton.png", "leftDisabledButton.png", cocos2d::ui::Widget::TextureResType::PLIST);
	this->leftAlignmentButton->addClickEventListener(CC_CALLBACK_1(FlockingScene::onButtonPressed, this));
	this->leftAlignmentButton->setActionTag(ACTION_TAG::ALIGNMENT_LEFT);
	this->leftAlignmentButton->setPosition(cocos2d::Vec2(leftButtonX, weightLabelY - buttonYOffset));
	this->addChild(this->leftAlignmentButton);

	this->rightAlignmentButton = cocos2d::ui::Button::create("rightButton.png", "rightSelectedButton.png", "rightDisabledButton.png", cocos2d::ui::Widget::TextureResType::PLIST);
	this->rightAlignmentButton->addClickEventListener(CC_CALLBACK_1(FlockingScene::onButtonPressed, this));
	this->rightAlignmentButton->setActionTag(ACTION_TAG::ALIGNMENT_RIGHT);
	this->rightAlignmentButton->setPosition(cocos2d::Vec2(rightButtonX, weightLabelY - buttonYOffset));
	this->addChild(this->rightAlignmentButton);


	this->cohesionLabel = cocos2d::Label::createWithTTF("COHESION", fontPath, 20);
	this->cohesionLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->cohesionLabel->setPosition(cocos2d::Vec2(labelX, weightLabelY - (weightLabelYOffset * 2.0f)));
	this->addChild(this->cohesionLabel);

	this->cohesionWeightLabel = cocos2d::Label::createWithTTF(std::to_string(ECS::FlockingData::COHENSION_WEIGHT).substr(0, 3), fontPath, 20);
	this->cohesionWeightLabel->setPosition(cocos2d::Vec2(weightLabelX, weightLabelY - (weightLabelYOffset * 2.0f)));
	this->addChild(this->cohesionWeightLabel);

	this->leftCohesionButton = cocos2d::ui::Button::create("leftButton.png", "leftSelectedButton.png", "leftDisabledButton.png", cocos2d::ui::Widget::TextureResType::PLIST);
	this->leftCohesionButton->addClickEventListener(CC_CALLBACK_1(FlockingScene::onButtonPressed, this));
	this->leftCohesionButton->setActionTag(ACTION_TAG::COHESION_LEFT);
	this->leftCohesionButton->setPosition(cocos2d::Vec2(leftButtonX, weightLabelY - (buttonYOffset * 2.0f)));
	this->addChild(this->leftCohesionButton);

	this->rightCohesionButton = cocos2d::ui::Button::create("rightButton.png", "rightSelectedButton.png", "rightDisabledButton.png", cocos2d::ui::Widget::TextureResType::PLIST);
	this->rightCohesionButton->addClickEventListener(CC_CALLBACK_1(FlockingScene::onButtonPressed, this));
	this->rightCohesionButton->setActionTag(ACTION_TAG::COHESION_RIGHT);
	this->rightCohesionButton->setPosition(cocos2d::Vec2(rightButtonX, weightLabelY - (buttonYOffset * 2.0f)));
	this->addChild(this->rightCohesionButton);


	this->separationLabel = cocos2d::Label::createWithTTF("SEPARATION", fontPath, 20);
	this->separationLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->separationLabel->setPosition(cocos2d::Vec2(labelX, weightLabelY - (weightLabelYOffset * 3.0f)));
	this->addChild(this->separationLabel);

	this->separationWeightLabel = cocos2d::Label::createWithTTF(std::to_string(ECS::FlockingData::SEPARATION_WEIGHT).substr(0, 3), fontPath, 20);
	this->separationWeightLabel->setPosition(cocos2d::Vec2(weightLabelX, weightLabelY - (weightLabelYOffset * 3.0f)));
	this->addChild(this->separationWeightLabel);

	this->leftSeparationButton = cocos2d::ui::Button::create("leftButton.png", "leftSelectedButton.png", "leftDisabledButton.png", cocos2d::ui::Widget::TextureResType::PLIST);
	this->leftSeparationButton->addClickEventListener(CC_CALLBACK_1(FlockingScene::onButtonPressed, this));
	this->leftSeparationButton->setActionTag(ACTION_TAG::SEPARATION_LEFT);
	this->leftSeparationButton->setPosition(cocos2d::Vec2(leftButtonX, weightLabelY - (buttonYOffset * 3.0f)));
	this->addChild(this->leftSeparationButton);

	this->rightSeparationButton = cocos2d::ui::Button::create("rightButton.png", "rightSelectedButton.png", "rightDisabledButton.png", cocos2d::ui::Widget::TextureResType::PLIST);
	this->rightSeparationButton->addClickEventListener(CC_CALLBACK_1(FlockingScene::onButtonPressed, this));
	this->rightSeparationButton->setActionTag(ACTION_TAG::SEPARATION_RIGHT);
	this->rightSeparationButton->setPosition(cocos2d::Vec2(rightButtonX, weightLabelY - (buttonYOffset * 3.0f)));
	this->addChild(this->rightSeparationButton);


	this->avoidLabel = cocos2d::Label::createWithTTF("AVOID", fontPath, 20);
	this->avoidLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->avoidLabel->setPosition(cocos2d::Vec2(labelX, weightLabelY - (weightLabelYOffset * 4.0f)));
	this->addChild(this->avoidLabel);

	this->avoidWeightLabel = cocos2d::Label::createWithTTF(std::to_string(ECS::FlockingData::AVOID_WEIGHT).substr(0, 3), fontPath, 20);
	this->avoidWeightLabel->setPosition(cocos2d::Vec2(weightLabelX, weightLabelY - (weightLabelYOffset * 4.0f)));
	this->addChild(this->avoidWeightLabel);

    this->leftAvoidButton = cocos2d::ui::Button::create("leftButton.png", "leftSelectedButton.png", "leftDisabledButton.png", cocos2d::ui::Widget::TextureResType::PLIST);
	this->leftAvoidButton->addClickEventListener(CC_CALLBACK_1(FlockingScene::onButtonPressed, this));
	this->leftAvoidButton->setActionTag(ACTION_TAG::AVOID_LEFT);
	this->leftAvoidButton->setPosition(cocos2d::Vec2(leftButtonX, weightLabelY - (buttonYOffset * 4.0f)));
	this->addChild(this->leftAvoidButton);

    this->rightAvoidButton = cocos2d::ui::Button::create("rightButton.png", "rightSelectedButton.png", "rightDisabledButton.png", cocos2d::ui::Widget::TextureResType::PLIST);
	this->rightAvoidButton->addClickEventListener(CC_CALLBACK_1(FlockingScene::onButtonPressed, this));
	this->rightAvoidButton->setActionTag(ACTION_TAG::AVOID_RIGHT);
	this->rightAvoidButton->setPosition(cocos2d::Vec2(rightButtonX, weightLabelY - (buttonYOffset * 4.0f)));
	this->addChild(this->rightAvoidButton);

	this->lastTrackingBoidId = -1;

	this->rangeChecker = cocos2d::Sprite::createWithSpriteFrameName("boidRangeChecker.png");
	this->rangeChecker->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
	this->rangeChecker->setVisible(false);
	this->rangeChecker->setOpacity(128);
	this->rangeChecker->setScale(ECS::FlockingData::SIGHT_RADIUS * 2.0f / 100.0f);
	this->areaNode->addChild(rangeChecker);

	fps = 0;
	fpsElapsedTime = 0;

	this->fpsLabel = cocos2d::Label::createWithTTF("FPS: " + std::to_string(cocos2d::Director::getInstance()->getFrameRate()), fontPath, 25);
	this->fpsLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->fpsLabel->setPosition(cocos2d::Vec2(labelX, 20.0f));
	this->addChild(this->fpsLabel);

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("Keys                        (Green = enabled)", fontPath, 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("Space = Toggle update", fontPath, 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("C = Clear all entities", fontPath, 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("A = Add 10 entities", fontPath, 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("E = Remove 10 entities(FIFO)", fontPath, 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    this->addChild(this->usageLabels.back());
    
    this->usageLabels.push_back(cocos2d::Label::createWithTTF("Mouse Hover and Key (Press key while hovering)", fontPath, 20));
    this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    this->addChild(this->usageLabels.back());
    
    this->usageLabels.push_back(cocos2d::Label::createWithTTF("O (hovering on box) = Add Obstacle", fontPath, 20));
    this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    this->addChild(this->usageLabels.back());
    
    this->usageLabels.push_back(cocos2d::Label::createWithTTF("O (hovering on Obstacle) = Remove Obstacle", fontPath, 19));
    this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("Mouse Click", fontPath, 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("Left Click (in box) = Add Boid", fontPath, 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("Left Click (on Boid) = Track Boid", fontPath, 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	this->usageLabels.push_back(cocos2d::Label::createWithTTF("Right click (on Boid) = Remove Boid", fontPath, 20));
	this->usageLabels.back()->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->usageLabels.back());

	cocos2d::Vec2 usageStartPos = cocos2d::Vec2(labelX, winSize.height * 0.65f);

	auto usageLabelSize = static_cast<int>(this->usageLabels.size());
	for (int i = 0; i < usageLabelSize; i++)
	{
		cocos2d::Vec2 newPos = usageStartPos;
		newPos.y -= (20.0f * static_cast<float>(i));
		this->usageLabels.at(i)->setPosition(newPos);
	}

	// Limit max entity to 400 in this case
	ECS::Entity::maxEntitySize = 400;

	this->pause = false;

	return true;
}

void FlockingScene::onEnter()
{
	cocos2d::Scene::onEnter();

	initInputListeners();

	initEntitiesAndQTree();
}

void FlockingScene::update(float delta)
{
	if (!pause)
	{
		resetQTreeAndPurge();
		updateFlockingAlgorithm(delta);
	}

	updateFPS(delta);
	entityCountLabel->setString("Entities: " + std::to_string(this->entities.size()) + " / " + std::to_string(ECS::Entity::maxEntitySize));
}

void FlockingScene::resetQTreeAndPurge()
{
	this->quadTree->clear();

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

		// Re-insert to quadtree
		this->quadTree->insert((*it));

		// Reset color to white. Only boids
		if ((*it)->getComponent<ECS::FlockingData*>(FLOCKING_DATA)->type == ECS::FlockingData::TYPE::BOID)
		{
			(*it)->getComponent<ECS::Sprite*>(SPRITE)->sprite->setColor(cocos2d::Color3B::WHITE);
		}

		// next
		it++;
	}
}

void FlockingScene::updateFlockingAlgorithm(const float delta)
{
	// iterate entities
	for (auto entity : entities)
	{
		auto entityFlockingObjComp = entity->getComponent<ECS::FlockingData*>(FLOCKING_DATA);
		if (entityFlockingObjComp->type == ECS::FlockingData::TYPE::BOID)
		{
			// If entity is boid, update flocking algorithm
			auto entitySpriteComp = entity->getComponent<ECS::Sprite*>(SPRITE);
			std::list<Entity*> nearEntities;

			// Create query rect and query near entities
			cocos2d::Rect queryingArea = cocos2d::Rect::ZERO;
			float pad = ECS::FlockingData::SIGHT_RADIUS;
			queryingArea.origin = (entitySpriteComp->sprite->getPosition() - cocos2d::Vec2(pad, pad));
			queryingArea.size = cocos2d::Vec2(pad * 2.0f, pad * 2.0f);

			this->quadTree->queryAllEntities(queryingArea, nearEntities);

			std::list<Entity*> nearBoids;
			std::list<Entity*> nearAvoids;

			// Iterate near entieis
			for (auto nearEntity : nearEntities)
			{
				auto nearEntityFlockingObjComp = nearEntity->getComponent<ECS::FlockingData*>(FLOCKING_DATA);
				auto nearBoidSpriteComp = nearEntity->getComponent<ECS::Sprite*>(SPRITE);
				auto entityPos = entitySpriteComp->sprite->getPosition();
				auto nearBoidPos = nearBoidSpriteComp->sprite->getPosition();
				float distance = nearBoidPos.distance(entityPos);
				if (nearEntityFlockingObjComp->type == ECS::FlockingData::TYPE::BOID)
				{
					// If near entity is boid, check distance
					if (distance <= ECS::FlockingData::SIGHT_RADIUS)
					{
						// Add near entity as near boid
						nearBoids.push_back(nearEntity);
						if (entityFlockingObjComp->tracking)
						{
							nearBoidSpriteComp->sprite->setColor(cocos2d::Color3B::GREEN);
						}
					}
				}
				else if (nearEntityFlockingObjComp->type == ECS::FlockingData::TYPE::OBSTACLE)
				{
					// If near entity is obstacle, check distance
					if (distance <= ECS::FlockingData::AVOID_RADIUS)
					{
						// Add near entity as near obstacle
						nearAvoids.push_back(nearEntity);
					}
				}
			}

			// Update direction vector
			auto entityDirVecComp = entity->getComponent<ECS::DirectionVector*>(DIRECTION_VECTOR);
			
			cocos2d::Vec2 finalVec = cocos2d::Vec2::ZERO;
			cocos2d::Vec2 avoidVec = cocos2d::Vec2::ZERO;
			if (!nearAvoids.empty())
			{
				// Apply avoid direction
				avoidVec = this->getAvoid(entity, nearAvoids);
				finalVec += (avoidVec * ECS::FlockingData::AVOID_WEIGHT);
			}

			if (!nearBoids.empty())
			{
				// Apply core 3 steer behavior.
				cocos2d::Vec2 alignmentVec = this->getAlignment(entity, nearBoids) * ECS::FlockingData::ALIGNMENT_WEIGHT;
				cocos2d::Vec2 cohesionVec = this->getCohesion(entity, nearBoids) * ECS::FlockingData::COHENSION_WEIGHT;
				cocos2d::Vec2 separationVec = this->getSeparation(entity, nearBoids) * ECS::FlockingData::SEPARATION_WEIGHT;
				finalVec += (alignmentVec + cohesionVec + separationVec);
			}

			// normalize and save
			finalVec.normalize();

			if (entityDirVecComp->smoothSteer)
			{
				// Steer boid's direction smooothly
				auto newDirVec = entityDirVecComp->dirVec;
				auto diffVec = finalVec - newDirVec;
				diffVec *= (delta * ECS::FlockingData::steerSpeed);
				entityDirVecComp->dirVec += diffVec;
			}
			else
			{
				// Steer instantly
				entityDirVecComp->dirVec = finalVec;
			}

			// update position
			auto movedDir = entityDirVecComp->dirVec * ECS::FlockingData::movementSpeed;
			auto newPos = entitySpriteComp->sprite->getPosition() + movedDir;
			entitySpriteComp->sprite->setPosition(newPos);

			float angle = entityDirVecComp->getAngle();
			entitySpriteComp->sprite->setRotation(-angle);

			if (!this->displayBoundary.containsPoint(newPos))
			{
				// wrap position if boid is out of boundary
				entitySpriteComp->wrapPositionWithInBoundary(this->displayBoundary);
			}

			if (entityFlockingObjComp->tracking)
			{
				entitySpriteComp->sprite->setColor(cocos2d::Color3B::BLUE);
				this->rangeChecker->setPosition(entitySpriteComp->sprite->getPosition());
			}
		}
	}
}

void FlockingScene::updateFPS(const float delta)
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

void FlockingScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(FlockingScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(FlockingScene::onMouseDown, this);
	this->mouseInputListener->onMouseUp = CC_CALLBACK_1(FlockingScene::onMouseUp, this);
	this->mouseInputListener->onMouseScroll = CC_CALLBACK_1(FlockingScene::onMouseScroll, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(FlockingScene::onKeyPressed, this);
	this->keyInputListener->onKeyReleased = CC_CALLBACK_2(FlockingScene::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void FlockingScene::initEntitiesAndQTree()
{
	// Create 40 entities at start
	const int initialEntityCount = 100;
	for (int i = 0; i < initialEntityCount; i++)
	{
		auto newEntity = createNewEntity();
		if (newEntity != nullptr)
		{
			this->entities.push_back(newEntity);
		}
	}

	// Init quadtree with initial boundary
	this->quadTree = new QTree(this->displayBoundary, 0);
}

ECS::Entity * FlockingScene::createNewEntity()
{
	Entity* newEntity = new Entity();

	// attach component and return
	auto dirVecComp = new ECS::DirectionVector();
	dirVecComp->smoothSteer = true;
	newEntity->components[DIRECTION_VECTOR] = dirVecComp;
	auto spriteComp = new ECS::Sprite(*this->areaNode, "boidEntity.png");
	const float angle = dirVecComp->getAngle();

	spriteComp->rotateToDirVec(-angle);
	spriteComp->setRandomPosInBoundary(this->displayBoundary);
	newEntity->components[SPRITE] = spriteComp;
	newEntity->components[FLOCKING_DATA] = new ECS::FlockingData(ECS::FlockingData::TYPE::BOID);

	return newEntity;
}

ECS::Entity * FlockingScene::createNewEntity(const cocos2d::Vec2 & pos)
{
	Entity* newEntity = createNewEntity();
	auto spriteComp = newEntity->getComponent<ECS::Sprite*>(SPRITE);
	spriteComp->sprite->setPosition(pos);
	return newEntity;
}

ECS::Entity * FlockingScene::createNewObstacleEntity(const cocos2d::Vec2& pos)
{
	Entity* newEntity = new Entity();
	auto spriteComp = new ECS::Sprite(*this->areaNode, "circle.png");
	spriteComp->sprite->setColor(cocos2d::Color3B::RED);
	spriteComp->sprite->setPosition(pos);
	newEntity->components[SPRITE] = spriteComp;
	newEntity->components[FLOCKING_DATA] = new ECS::FlockingData(ECS::FlockingData::TYPE::OBSTACLE);
	return newEntity;
}

const cocos2d::Vec2 FlockingScene::getAlignment(Entity* boid, std::list<Entity*>& nearBoids)
{
	cocos2d::Vec2 sumDirVec = cocos2d::Vec2::ZERO;

	const cocos2d::Vec2 boidPos = boid->getComponent<ECS::Sprite*>(SPRITE)->sprite->getPosition();

	for (auto nearBoid : nearBoids)
	{
		auto dirVecComp = nearBoid->getComponent<ECS::DirectionVector*>(DIRECTION_VECTOR);
		sumDirVec += dirVecComp->dirVec;
	}

	const float countF = static_cast<float>(nearBoids.size());
	sumDirVec.x /= countF;
	sumDirVec.y /= countF;
	sumDirVec.normalize();
	return sumDirVec;
}

const cocos2d::Vec2 FlockingScene::getCohesion(Entity* boid, std::list<Entity*>& nearBoids)
{
	cocos2d::Vec2 sumPosVec = cocos2d::Vec2::ZERO;

	const cocos2d::Vec2 boidPos = boid->getComponent<ECS::Sprite*>(SPRITE)->sprite->getPosition();

	for (auto nearBoid : nearBoids)
	{
		auto nearBoidSpriteComp = nearBoid->getComponent<ECS::Sprite*>(SPRITE);
		sumPosVec += nearBoidSpriteComp->sprite->getPosition();
	}

	const float countF = static_cast<float>(nearBoids.size());
	sumPosVec.x /= countF;
	sumPosVec.y /= countF;

	cocos2d::Vec2 cohesionVec = sumPosVec - boidPos;

	cohesionVec.normalize();

	return cohesionVec;
}

const cocos2d::Vec2 FlockingScene::getSeparation(Entity* boid, std::list<Entity*>& nearBoids)
{
	cocos2d::Vec2 sumDistVec = cocos2d::Vec2::ZERO;

	const cocos2d::Vec2 boidPos = boid->getComponent<ECS::Sprite*>(SPRITE)->sprite->getPosition();

	for (auto nearBoid : nearBoids)
	{
		auto nearBoidSpriteComp = nearBoid->getComponent<ECS::Sprite*>(SPRITE);
		auto nearBoidPos = nearBoidSpriteComp->sprite->getPosition();
		auto distVec = nearBoidPos - boidPos;
		float distance = nearBoidPos.distance(boidPos);
		if (distance <= 0)
		{
			sumDistVec += distVec;
		}
		else
		{
			sumDistVec += (distVec * (1.0f / distance));
		}
	}

	const float countF = static_cast<float>(nearBoids.size());
	sumDistVec.x /= countF;
	sumDistVec.y /= countF;

	sumDistVec *= -1;

	sumDistVec.normalize();

	return sumDistVec;
}

const cocos2d::Vec2 FlockingScene::getAvoid(Entity* boid, std::list<Entity*>& nearAvoids)
{
	int count = 0;
	cocos2d::Vec2 sumDistVec = cocos2d::Vec2::ZERO;

	const cocos2d::Vec2 boidPos = boid->getComponent<ECS::Sprite*>(SPRITE)->sprite->getPosition();

	for (auto nearAvoid : nearAvoids)
	{
		auto nearAvoidSpriteComp = nearAvoid->getComponent<ECS::Sprite*>(SPRITE);
		auto nearAvoidPos = nearAvoidSpriteComp->sprite->getPosition();
		cocos2d::Vec2 distVec = boidPos - nearAvoidPos;
		sumDistVec += distVec;
		count++;
	}

	if (count > 0)
	{
		const float countF = static_cast<float>(count);
		sumDistVec.x /= countF;
		sumDistVec.y /= countF;
		sumDistVec.normalize();
	}

	return sumDistVec;
}

void FlockingScene::onButtonPressed(cocos2d::Ref * sender)
{
	auto button = dynamic_cast<cocos2d::ui::Button*>(sender);
	auto tag = button->getActionTag();
	switch (tag)
	{
	case ACTION_TAG::ALIGNMENT_LEFT:
		if (ECS::FlockingData::ALIGNMENT_WEIGHT > 0.1f)
		{
			ECS::FlockingData::ALIGNMENT_WEIGHT -= 0.1f;
		}
		else
		{
			ECS::FlockingData::ALIGNMENT_WEIGHT = 0;
		}
		this->alignmentWeightLabel->setString(std::to_string(ECS::FlockingData::ALIGNMENT_WEIGHT).substr(0, 3));
		break;
	case ACTION_TAG::ALIGNMENT_RIGHT:
		if (ECS::FlockingData::ALIGNMENT_WEIGHT < 2.0f)
		{
			ECS::FlockingData::ALIGNMENT_WEIGHT += 0.1f;
		}
		else
		{
			ECS::FlockingData::ALIGNMENT_WEIGHT = 2.0f;
		}
		this->alignmentWeightLabel->setString(std::to_string(ECS::FlockingData::ALIGNMENT_WEIGHT).substr(0, 3));
		break;
	case ACTION_TAG::COHESION_LEFT:
		if (ECS::FlockingData::COHENSION_WEIGHT > 0.1f)
		{
			ECS::FlockingData::COHENSION_WEIGHT -= 0.1f;
		}
		else
		{
			ECS::FlockingData::COHENSION_WEIGHT = 0;
		}
		this->cohesionWeightLabel->setString(std::to_string(ECS::FlockingData::COHENSION_WEIGHT).substr(0, 3));
		break;
	case ACTION_TAG::COHESION_RIGHT:
		if (ECS::FlockingData::COHENSION_WEIGHT < 2.0f)
		{
			ECS::FlockingData::COHENSION_WEIGHT += 0.1f;
		}
		else
		{
			ECS::FlockingData::COHENSION_WEIGHT = 2.0f;
		}
		this->cohesionWeightLabel->setString(std::to_string(ECS::FlockingData::COHENSION_WEIGHT).substr(0, 3));
		break;
	case ACTION_TAG::SEPARATION_LEFT:
		if (ECS::FlockingData::SEPARATION_WEIGHT > 0.1f)
		{
			ECS::FlockingData::SEPARATION_WEIGHT -= 0.1f;
		}
		else
		{
			ECS::FlockingData::SEPARATION_WEIGHT = 0;
		}
		this->separationWeightLabel->setString(std::to_string(ECS::FlockingData::SEPARATION_WEIGHT).substr(0, 3));
		break;
	case ACTION_TAG::SEPARATION_RIGHT:
		if (ECS::FlockingData::SEPARATION_WEIGHT < 2.0f)
		{
			ECS::FlockingData::SEPARATION_WEIGHT += 0.1f;
		}
		else
		{
			ECS::FlockingData::SEPARATION_WEIGHT = 2.0f;
		}
		this->separationWeightLabel->setString(std::to_string(ECS::FlockingData::SEPARATION_WEIGHT).substr(0, 3));
		break;
	case ACTION_TAG::AVOID_LEFT:
		if (ECS::FlockingData::AVOID_WEIGHT > 0.1f)
		{
			ECS::FlockingData::AVOID_WEIGHT -= 0.1f;
		}
		else
		{
			ECS::FlockingData::AVOID_WEIGHT = 0;
		}
		this->avoidWeightLabel->setString(std::to_string(ECS::FlockingData::AVOID_WEIGHT).substr(0, 3));
		break;
	case ACTION_TAG::AVOID_RIGHT:
		if (ECS::FlockingData::AVOID_WEIGHT < 2.0f)
		{
			ECS::FlockingData::AVOID_WEIGHT += 0.1f;
		}
		else
		{
			ECS::FlockingData::AVOID_WEIGHT = 2.0f;
		}
		this->avoidWeightLabel->setString(std::to_string(ECS::FlockingData::AVOID_WEIGHT).substr(0, 3));
		break;
	default:
		break;
	}
}

void FlockingScene::playUIAnimation(const USAGE_KEY usageKey)
{
	this->usageLabels.at(static_cast<int>(usageKey))->stopAllActions();
	this->usageLabels.at(static_cast<int>(usageKey))->setScale(1.0f);
	this->usageLabels.at(static_cast<int>(usageKey))->runAction(this->clickAnimation);
}

void FlockingScene::onMouseMove(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();
    
    const cocos2d::Vec2 point = cocos2d::Vec2(x, y);

	if (this->backLabel->getBoundingBox().containsPoint(point))
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
    
    this->curMousePosition = point;
}

void FlockingScene::onMouseDown(cocos2d::Event* event) 
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
			for (auto entity : this->entities)
			{
				auto spriteComp = entity->getComponent<ECS::Sprite*>(SPRITE);
				if (spriteComp->sprite->getBoundingBox().containsPoint(point))
				{
					auto entityFlockingObjComp = entity->getComponent<ECS::FlockingData*>(FLOCKING_DATA);

					// Clicked boid sprite
					if (entity->id == this->lastTrackingBoidId)
					{
						// Already tracking this boid. Disble tracking
						entityFlockingObjComp->tracking = false;
						this->lastTrackingBoidId = -1;
						this->rangeChecker->setVisible(false);
						this->playUIAnimation(USAGE_KEY::TRACK);
						this->usageLabels.at(static_cast<int>(USAGE_KEY::TRACK))->setColor(cocos2d::Color3B::WHITE);
						return;
					}
					
					// New entity to track
					entityFlockingObjComp->tracking = true;

					if (this->lastTrackingBoidId != -1)
					{
						// Already tracking entity
						for (auto lastEntity : this->entities)
						{
							if (lastEntity->id == this->lastTrackingBoidId)
							{
								// Disable tracking on last tracking entitiy
								auto comp = lastEntity->getComponent<ECS::FlockingData*>(FLOCKING_DATA);
								this->playUIAnimation(USAGE_KEY::TRACK);
								this->usageLabels.at(static_cast<int>(USAGE_KEY::TRACK))->setColor(cocos2d::Color3B::GREEN);
								comp->tracking = false;
								break;
							}
						}
					}

					// Set this entity to new tracking entity
					this->lastTrackingBoidId = entity->id;

					this->rangeChecker->setVisible(true);
					this->rangeChecker->setPosition(entity->getComponent<ECS::Sprite*>(SPRITE)->sprite->getPosition());
					this->playUIAnimation(USAGE_KEY::TRACK);
					return;
				}
			}

			// If it didn't track any entity, create one
			if (static_cast<int>(this->entities.size()) < ECS::Entity::maxEntitySize)
			{
				this->entities.push_back(createNewEntity(point));
				this->playUIAnimation(USAGE_KEY::ADD_ONE);
			}
		}
	}
	else if (mouseButton == 1)
	{
		if (this->displayBoundary.containsPoint(point))
		{
			for (auto entity : this->entities)
			{
				auto spriteComp = entity->getComponent<ECS::Sprite*>(SPRITE);
				if (spriteComp->sprite->getBoundingBox().containsPoint(point))
				{
					entity->alive = false;
					auto flockingObjComp = entity->getComponent<ECS::FlockingData*>(FLOCKING_DATA);
					if (flockingObjComp->tracking)
					{
						this->rangeChecker->setVisible(false);
					}
					this->playUIAnimation(USAGE_KEY::REMOVE_ONE);
					return;
				}
			}
		}
	}
	else if (mouseButton == 2)
	{
		// Middle click
		if (this->displayBoundary.containsPoint(point))
		{
			for (auto entity : this->entities)
			{
				auto spriteComp = entity->getComponent<ECS::Sprite*>(SPRITE);
				if (spriteComp->sprite->getPosition().distance(point) < 6.0f)
				{
					entity->alive = false;
					this->playUIAnimation(USAGE_KEY::REMOVE_OBSTACLE);
					return;
				}
			}

			this->entities.push_back(createNewObstacleEntity(point));
			this->playUIAnimation(USAGE_KEY::ADD_OBSTACLE);
		}
	}
}

void FlockingScene::onMouseUp(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	//int mouseButton = mouseEvent->getMouseButton();
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void FlockingScene::onMouseScroll(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//float x = mouseEvent->getScrollX();
	//float y = mouseEvent->getScrollY();
}

void FlockingScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE)
	{
		// Terminate 
		cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_SPACE)
	{
		// Terminate 
		this->pause = !this->pause;
		this->playUIAnimation(USAGE_KEY::SPACE);
		if (this->pause)
		{
			this->usageLabels.at(USAGE_KEY::SPACE)->setColor(cocos2d::Color3B::GREEN);
		}
		else
		{
			this->usageLabels.at(USAGE_KEY::SPACE)->setColor(cocos2d::Color3B::WHITE);
		}
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_C)
	{
		// Terminate 
		for (auto entity : this->entities)
		{
			entity->alive = false;
			this->playUIAnimation(USAGE_KEY::CLEAR);
			this->rangeChecker->setVisible(false);
		}
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_A)
	{
		// Terminate 
		int count = 0;
		while (this->entities.size() < 400 && count < 10)
		{
			this->entities.push_back(createNewEntity());
			count++;
		}
		this->playUIAnimation(USAGE_KEY::ADD_TEN);
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_E)
	{
		int count = 0;
		for (auto entity : this->entities)
		{
			if (count < 10)
			{
				entity->alive = false;
				auto flockingObjComp = entity->getComponent<ECS::FlockingData*>(FLOCKING_DATA);
				if (flockingObjComp->tracking)
				{
					this->rangeChecker->setVisible(false);
				}
			}
		}
		this->playUIAnimation(USAGE_KEY::REMOVE_TEN);
	}
    
    if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_O)
    {
        // This is added for laptop users because they might not have middle click
        if(this->curMousePosition.x == -1.0f || this->curMousePosition.y == -1.0f)
        {
            // mouse position must be positive. It's either out of window or hasn't intialized
            return;
        }
        
        if (this->displayBoundary.containsPoint(this->curMousePosition))
        {
            for (auto entity : this->entities)
            {
                auto spriteComp = entity->getComponent<ECS::Sprite*>(SPRITE);
                if (spriteComp->sprite->getPosition().distance(this->curMousePosition) < 6.0f)
                {
                    entity->alive = false;
                    this->playUIAnimation(USAGE_KEY::REMOVE_OBSTACLE);
                    return;
                }
            }
            
            this->entities.push_back(createNewObstacleEntity(this->curMousePosition));
            this->playUIAnimation(USAGE_KEY::ADD_OBSTACLE);
        }
    }
}

void FlockingScene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{

}

void FlockingScene::releaseInputListeners()
{
	if(this->mouseInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->mouseInputListener);
	if(this->keyInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->keyInputListener);
}

void FlockingScene::onExit()
{
	cocos2d::Scene::onExit();

	releaseInputListeners();

	this->areaNode->release();

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