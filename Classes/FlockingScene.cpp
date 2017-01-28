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
    
    // init id counter to 0
	ECS::Entity::idCounter = 0;

	this->smoothSteering = true;
    
    // Init mouse point
    this->curMousePosition = cocos2d::Vec2(-1.0f, -1.0f);

	// Uncomment this to activate update(float) function
	this->scheduleUpdate();

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

    // Intialize area node where entities get rendered
	this->areaNode = cocos2d::Node::create();
	this->areaNode->setPosition(cocos2d::Vec2::ZERO);
	this->areaNode->retain();
	this->addChild(this->areaNode);
    
    // Init labels node
    this->labelsNode = LabelsNode::createNode();
    this->addChild(this->labelsNode);
    
    // Init custom labels
    float labelX = winSize.height - 10.0f;
    float labelY = winSize.height - 45.0f;
    this->labelsNode->customLabelStartPos = cocos2d::Vec2(labelX, labelY);

	const int titleSize = 35;
	const int customLabelSize = 25;
	const int blankLineSize = 15;

	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Flocking Algorithm Visualization", titleSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, " ", blankLineSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Entities: 0", customLabelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Weights (Click buttons to modify weights)", customLabelSize);
    
    // Init button node
    this->buttonModifierNode = ButtonModifierNode::createNode();
    this->buttonModifierNode->setPosition(cocos2d::Vec2::ZERO);
    this->buttonModifierNode->retain();
    this->addChild(this->buttonModifierNode);

	const float customLastY = this->labelsNode->customLabels.back()->getBoundingBox().getMinY();
	const float blockGap = 22.0f;
    
	float buttonLabelY = customLastY - blockGap;
    
    const std::string leftStr = "left";
    const std::string rightStr = "right";
    const std::string buttonStr = "Button";
    const std::string format = ".png";
    
    this->buttonModifierNode->buttonLabelStartPos = cocos2d::Vec2(labelX, buttonLabelY);
    
    this->buttonModifierNode->leftButtonXOffset = 160.0f;
    this->buttonModifierNode->valueLabelXOffset = 190.0f;
    this->buttonModifierNode->rightButtonXOffset = 240.0f;

	const int labelSize = 20;
    this->buttonModifierNode->addButton("Alignment",
										labelSize,
                                        ECS::FlockingData::ALIGNMENT_WEIGHT,
                                        leftStr,
                                        rightStr,
                                        buttonStr,
                                        format,
                                        static_cast<int>(ACTION_TAG::ALIGNMENT_LEFT),
                                        static_cast<int>(ACTION_TAG::ALIGNMENT_RIGHT),
                                        CC_CALLBACK_1(FlockingScene::onButtonPressed, this));
    
    this->buttonModifierNode->addButton("Cohesion",
										labelSize,
                                        ECS::FlockingData::COHENSION_WEIGHT,
                                        leftStr,
                                        rightStr,
                                        buttonStr,
                                        format,
                                        static_cast<int>(ACTION_TAG::COHESION_LEFT),
                                        static_cast<int>(ACTION_TAG::COHESION_RIGHT),
                                        CC_CALLBACK_1(FlockingScene::onButtonPressed, this));
    
    this->buttonModifierNode->addButton("Separation",
										labelSize,
                                        ECS::FlockingData::SEPARATION_WEIGHT,
                                        leftStr,
                                        rightStr,
                                        buttonStr,
                                        format,
                                        static_cast<int>(ACTION_TAG::SEPARATION_LEFT),
                                        static_cast<int>(ACTION_TAG::SEPARATION_RIGHT),
                                        CC_CALLBACK_1(FlockingScene::onButtonPressed, this));
    
    this->buttonModifierNode->addButton("Avoid",
										labelSize,
                                        ECS::FlockingData::AVOID_WEIGHT,
                                        leftStr,
                                        rightStr,
                                        buttonStr,
                                        format,
                                        static_cast<int>(ACTION_TAG::AVOID_LEFT),
                                        static_cast<int>(ACTION_TAG::AVOID_RIGHT),
                                        CC_CALLBACK_1(FlockingScene::onButtonPressed, this));

	this->lastTrackingBoidId = -1;

	this->rangeChecker = cocos2d::Sprite::createWithSpriteFrameName("boidRangeChecker.png");
	this->rangeChecker->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
	this->rangeChecker->setVisible(false);
	this->rangeChecker->setOpacity(128);
	this->rangeChecker->setScale(ECS::FlockingData::SIGHT_RADIUS * 2.0f / 100.0f);
	this->areaNode->addChild(rangeChecker);
    
    // init more labels    
	const int headerSize = 25;

	const float weightLastY = this->buttonModifierNode->buttonLabels.back()->getBoundingBox().getMinY();
	this->labelsNode->keyboardUsageLabelStartPos = cocos2d::Vec2(labelX, weightLastY - blockGap);

    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Keys (Green = enabled)", headerSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Space = Toggle update", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "C = Clear all entities", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "A = Add 10 Boids", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "E = Remove 10 Boids", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "S = Toggle smooth steering", labelSize);
    
	const float keysLastY = this->labelsNode->keyboardUsageLabels.back()->getBoundingBox().getMinY();
    this->labelsNode->mouseOverAndKeyLabelStartPos = cocos2d::Vec2(labelX, keysLastY - blockGap);
    
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE_OVER_AND_KEY, "Mouse over and key", headerSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE_OVER_AND_KEY, "O (In box) = Add Obstacle on point", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE_OVER_AND_KEY, "O (On Obstacle) = Remove Obstacle", labelSize);
    
	const float mouseOverLastY = this->labelsNode->mouseOverAndKeyUsageLabels.back()->getBoundingBox().getMinY();
    this->labelsNode->mouseUsageLabelStartPos = cocos2d::Vec2(labelX, mouseOverLastY - blockGap);
    
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Mouse", headerSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Left Click (In box) = Add Boid", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Left Click (On Boid) = Toggle Boid tracking", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Right Click (On Boid) = Remove Boid)", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Middle Click (In box) = Add Obstacle)", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Middle Click (On Obstacle) = Remove Obstacle", labelSize);

	// speed modifier
	this->simulationSpeedModifier = 1.0f;

	this->sliderLabelNode = SliderLabelNode::createNode();
	const float mouseLastY = this->labelsNode->mouseUsageLabels.back()->getBoundingBox().getMinY();
	this->sliderLabelNode->sliderStartPos = cocos2d::Vec2(labelX, mouseLastY - blockGap);
	this->sliderLabelNode->addSlider("Simulation Speed", "Slider", 50, CC_CALLBACK_1(FlockingScene::onSliderClick, this));
	this->addChild(this->sliderLabelNode);

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
	this->labelsNode->updateFPSLabel(delta);

	if (!pause)
	{
		resetQTreeAndPurge();

		delta *= this->simulationSpeedModifier;
		updateFlockingAlgorithm(delta);
	}
    
    this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::ENTITIES), "Entities: " + std::to_string(this->entities.size()) + " / " + std::to_string(ECS::Entity::maxEntitySize));
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
				auto diffVec = finalVec - entityDirVecComp->dirVec;
				diffVec *= (delta * ECS::FlockingData::steerSpeed);
				entityDirVecComp->dirVec += diffVec;
			}
			else
			{
				// Steer instantly
				entityDirVecComp->dirVec = finalVec;
			}

			entityDirVecComp->dirVec.normalize();

			// update position
			auto movedDir = entityDirVecComp->dirVec * delta * ECS::FlockingData::movementSpeed;
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

void FlockingScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(FlockingScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(FlockingScene::onMouseDown, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(FlockingScene::onKeyPressed, this);
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
	this->quadTree = new QuadTree(this->displayBoundary, 0);
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
    auto tag = static_cast<ACTION_TAG>(button->getActionTag());
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
        this->buttonModifierNode->updateValue(static_cast<int>(WEIGHT_INDEX::ALIGNMENT), ECS::FlockingData::ALIGNMENT_WEIGHT);
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
        this->buttonModifierNode->updateValue(static_cast<int>(WEIGHT_INDEX::ALIGNMENT), ECS::FlockingData::ALIGNMENT_WEIGHT);
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
        this->buttonModifierNode->updateValue(static_cast<int>(WEIGHT_INDEX::COHESION), ECS::FlockingData::COHENSION_WEIGHT);
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
        this->buttonModifierNode->updateValue(static_cast<int>(WEIGHT_INDEX::COHESION), ECS::FlockingData::COHENSION_WEIGHT);
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
        this->buttonModifierNode->updateValue(static_cast<int>(WEIGHT_INDEX::SEPARATION), ECS::FlockingData::SEPARATION_WEIGHT);
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
        this->buttonModifierNode->updateValue(static_cast<int>(WEIGHT_INDEX::SEPARATION), ECS::FlockingData::SEPARATION_WEIGHT);
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
        this->buttonModifierNode->updateValue(static_cast<int>(WEIGHT_INDEX::AVOID), ECS::FlockingData::AVOID_WEIGHT);
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
        this->buttonModifierNode->updateValue(static_cast<int>(WEIGHT_INDEX::AVOID), ECS::FlockingData::AVOID_WEIGHT);
		break;
	default:
		break;
	}
}

void FlockingScene::onMouseMove(cocos2d::Event* event) 
{
    
    auto mouseEvent = static_cast<EventMouse*>(event);
    float x = mouseEvent->getCursorX();
    float y = mouseEvent->getCursorY();
    
    auto point = cocos2d::Vec2(x, y);
    
    this->labelsNode->updateMouseHover(point);
    
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
    
    bool ret = this->labelsNode->updateMouseDown(point);
    if(ret)
    {
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
                        
                        this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::TRACK), cocos2d::Color3B::WHITE);
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
								comp->tracking = false;
								break;
							}
						}
					}

					// Set this entity to new tracking entity
					this->lastTrackingBoidId = entity->id;

					this->rangeChecker->setVisible(true);
					this->rangeChecker->setPosition(entity->getComponent<ECS::Sprite*>(SPRITE)->sprite->getPosition());
                    this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::TRACK), cocos2d::Color3B::GREEN);
					return;
				}
			}

			// If it didn't track any entity, create one
			if (static_cast<int>(this->entities.size()) < ECS::Entity::maxEntitySize)
			{
				this->entities.push_back(createNewEntity(point));
                this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::ADD_ONE), cocos2d::Color3B::WHITE);
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
                    
                    this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::REMOVE_ONE), cocos2d::Color3B::WHITE);
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
                    this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::REMOVE_OBSTACLE), cocos2d::Color3B::GREEN);
					return;
				}
			}

			this->entities.push_back(createNewObstacleEntity(point));
            this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::ADD_OBSTACLE), cocos2d::Color3B::GREEN);
		}
	}
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
		if (this->pause)
        {
            this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::SPACE), cocos2d::Color3B::GREEN);
		}
		else
        {
            this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::SPACE), cocos2d::Color3B::WHITE);
		}
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_C)
	{
		// Terminate 
		for (auto entity : this->entities)
		{
            entity->alive = false;
            this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::CLEAR), cocos2d::Color3B::WHITE);
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
        this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::ADD_TEN), cocos2d::Color3B::WHITE);
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
                count++;
			}
        }
        this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::REMOVE_TEN), cocos2d::Color3B::WHITE);
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_S)
	{
		this->smoothSteering = !this->smoothSteering;

		for (auto entity : this->entities)
		{
			auto dirVecComp = entity->getComponent<DirectionVector*>(DIRECTION_VECTOR);
			dirVecComp->smoothSteer = this->smoothSteering;
		}

		if (this->smoothSteering)
		{
			this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::SMOOTH_STEERING), cocos2d::Color3B::GREEN);
		}
		else
		{
			this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::SMOOTH_STEERING), cocos2d::Color3B::WHITE);
		}
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
                    this->labelsNode->setColor(LabelsNode::TYPE::MOUSE_OVER_AND_KEY, static_cast<int>(USAGE_MOUSE_OVER_AND_KEY::REMOVE_OBSTACLE), cocos2d::Color3B::WHITE);
                    return;
                }
            }
            
            this->entities.push_back(createNewObstacleEntity(this->curMousePosition));
            this->labelsNode->setColor(LabelsNode::TYPE::MOUSE_OVER_AND_KEY, static_cast<int>(USAGE_MOUSE_OVER_AND_KEY::ADD_OBSTACLE), cocos2d::Color3B::WHITE);
        }
    }
}

void FlockingScene::onSliderClick(cocos2d::Ref* sender)
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
    this->displayBoundaryBoxNode->release();
    this->buttonModifierNode->release();

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
