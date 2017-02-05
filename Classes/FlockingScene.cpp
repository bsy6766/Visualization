#include "FlockingScene.h"
#include "MainScene.h"
#include "Utility.h"

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

	ECS::Manager::getInstance();
    
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
	this->labelsNode->setSharedLabelPosition(LabelsNode::SHARED_LABEL_POS_TYPE::QUADTREE_SCENE);
	this->addChild(this->labelsNode);

	// Starting pos
	float labelX = winSize.height - 10.0f;
	float labelY = winSize.height - 45.0f;

	// Set title
	this->labelsNode->initTitleStr("Flocking Algorithm", cocos2d::Vec2(labelX, labelY));

	labelY -= 50.0f;

	// Init custom labels
	this->labelsNode->customLabelStartPos = cocos2d::Vec2(labelX, labelY);

	// Set size
	const int customLabelSize = 25;

    this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Boids: 0", customLabelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Obstacles: 0", customLabelSize);
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
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Space: Toggle update", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "C: Clear all entities", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "A: Add 10 Boids", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "E: Remove 10 Boids", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "S: Toggle smooth steering", labelSize);
	this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::SMOOTH_STEERING), cocos2d::Color3B::GREEN, false);
    
	const float keysLastY = this->labelsNode->keyboardUsageLabels.back()->getBoundingBox().getMinY();
    this->labelsNode->mouseOverAndKeyLabelStartPos = cocos2d::Vec2(labelX, keysLastY - blockGap);
    
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE_OVER_AND_KEY, "Mouse over and key", headerSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE_OVER_AND_KEY, "O (In box): Add Obstacle on point", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE_OVER_AND_KEY, "O (On Obstacle): Remove Obstacle", labelSize);
    
	const float mouseOverLastY = this->labelsNode->mouseOverAndKeyUsageLabels.back()->getBoundingBox().getMinY();
    this->labelsNode->mouseUsageLabelStartPos = cocos2d::Vec2(labelX, mouseOverLastY - blockGap);
    
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Mouse", headerSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Left Click (In box): Add Boid", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Left Click (On Boid): Toggle Boid tracking", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Right Click (On Boid): Remove Boid)", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Middle Click (In box): Add Obstacle)", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Middle Click (On Obstacle): Remove Obstacle", labelSize);

	// speed modifier
	this->simulationSpeedModifier = 1.0f;

	this->sliderLabelNode = SliderLabelNode::createNode();
	const float mouseLastY = this->labelsNode->mouseUsageLabels.back()->getBoundingBox().getMinY();
	this->sliderLabelNode->sliderStartPos = cocos2d::Vec2(labelX, mouseLastY - blockGap);
	this->sliderLabelNode->addSlider("Simulation Speed", "Slider", 50, CC_CALLBACK_1(FlockingScene::onSliderClick, this));
	this->addChild(this->sliderLabelNode);

	return true;
}

void FlockingScene::onEnter()
{
	cocos2d::Scene::onEnter();

	initInputListeners();

	initECS();
}

void FlockingScene::initECS()
{
	ECS::Manager* m = ECS::Manager::getInstance();
	m->createEntityPool("FB", 512);
	m->createEntityPool("FO", 32);

	// Create 40 entities at start
	const int initialEntityCount = 40;
	for (int i = 0; i < initialEntityCount; i++)
	{
		createNewBoid();
	}

	auto system = m->createSystem<ECS::FlockingSystem>();
	system->disbaleDefafultEntityPool();
	system->addEntityPoolName("FB");
	system->addEntityPoolName("FO");
	// Obstacle doesn't have direction vector. So don't add that as a component ttype
	system->addComponentType<ECS::Sprite>();
	system->addComponentType<ECS::FlockingData>();

	system->initQuadTree(this->displayBoundary);
	system->displayBoundary = this->displayBoundary;
}

void FlockingScene::update(float delta)
{
	// Update fps label always
	this->labelsNode->updateFPSLabel(delta);

	// Apply simulation speed modifier
	delta *= this->simulationSpeedModifier;

	auto m = ECS::Manager::getInstance();
	auto system = m->getSystem<ECS::FlockingSystem>();

	int boidCount = static_cast<int>(m->getAliveEntityCountInEntityPool("FB"));
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::BOIDS), "Boids: " + std::to_string(boidCount) + " / 512");
	int obstacleCount = static_cast<int>(m->getAliveEntityCountInEntityPool("FO"));
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::OBSTACLES), "Obstacles: " + std::to_string(obstacleCount) + " / 32");

	if (!system->isActive())
	{
		// Rebuild quad tree when system is offline. Don't update algorithm
		std::vector<ECS::Entity*> entities;
		m->getAllEntitiesForSystem<ECS::FlockingSystem>(entities);
		system->rebuildQuadTree(entities);
		return;
	}

	Utility::Time::start();

	m->update(delta);
	if (system->lastTrackingBoidId != ECS::INVALID_E_ID)
	{
		this->rangeChecker->setPosition(m->getEntityById(system->lastTrackingBoidId)->getComponent<ECS::Sprite>()->sprite->getPosition());
	}

	Utility::Time::stop();

	std::string timeTakenStr = Utility::Time::getElaspedTime();	// Microseconds
	float timeTakenF = std::stof(timeTakenStr);	// to float
	timeTakenF *= 0.001f; // To milliseconds

	this->labelsNode->updateTimeTakenLabel(std::to_string(timeTakenF).substr(0, 5));
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

void FlockingScene::createNewBoid(const cocos2d::Vec2& pos)
{
	auto m = ECS::Manager::getInstance();
	ECS::Entity* e = m->createEntity("FB");
	if (e == nullptr)
	{
		return;
	}

	auto dirVecComp = m->createComponent<ECS::DirectionVector>();
	dirVecComp->smoothSteer = true;
	auto spriteComp = m->createComponent<ECS::Sprite>();
	spriteComp->sprite = cocos2d::Sprite::createWithSpriteFrameName("boidEntity.png");
	spriteComp->sprite->retain();
	if (pos == cocos2d::Vec2::ZERO)
	{
		spriteComp->setRandomPosInBoundary(this->displayBoundary);
	}
	else
	{
		spriteComp->sprite->setPosition(pos);
	}
	this->areaNode->addChild(spriteComp->sprite);

	const float angle = dirVecComp->getAngle();

	spriteComp->rotateToDirVec(-angle);

	e->addComponent<ECS::Sprite>(spriteComp);
	e->addComponent<ECS::DirectionVector>(dirVecComp);
	e->addComponent<ECS::FlockingData>();
}

void FlockingScene::createNewObstacle(const cocos2d::Vec2& pos)
{
	auto m = ECS::Manager::getInstance();
	ECS::Entity* e = m->createEntity("FO");
	if (e == nullptr)
	{
		return;
	}

	auto spriteComp = m->createComponent<ECS::Sprite>();
	spriteComp->sprite = cocos2d::Sprite::createWithSpriteFrameName("circle.png");
	spriteComp->sprite->setColor(cocos2d::Color3B::RED);
	spriteComp->sprite->retain();
	spriteComp->sprite->setPosition(pos);
	this->areaNode->addChild(spriteComp->sprite);

	e->addComponent<ECS::Sprite>(spriteComp);

	auto dataComp = m->createComponent<ECS::FlockingData>();
	dataComp->type = ECS::FlockingData::TYPE::OBSTACLE;

	e->addComponent<ECS::FlockingData>(dataComp);
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

	auto m = ECS::Manager::getInstance();
	auto system = m->getSystem<ECS::FlockingSystem>();

	if (this->displayBoundary.containsPoint(point))
	{
		if (mouseButton == 0)
		{
			bool entityClicked = system->updateMouseDown(mouseButton, point);
			if (entityClicked)
			{
				if (system->lastTrackingBoidId != ECS::INVALID_E_ID)
				{
					this->rangeChecker->setVisible(true);
					auto e = m->getEntityById(system->lastTrackingBoidId);
					this->rangeChecker->setPosition(e->getComponent<ECS::Sprite>()->sprite->getPosition());
					this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::TRACK), cocos2d::Color3B::GREEN);
				}
				else
				{
					this->rangeChecker->setVisible(false);
					this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::TRACK), cocos2d::Color3B::WHITE);
				}
			}
			else
			{
				this->createNewBoid(point);
				this->labelsNode->playAnimation(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::ADD_ONE));
			}
		}
		else if (mouseButton == 1)
		{
			bool wasTracking = system->lastTrackingBoidId != ECS::INVALID_E_ID ? true : false;
			bool entityClicked = system->updateMouseDown(1, point);
			if (entityClicked)
			{
				this->labelsNode->playAnimation(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::REMOVE_ONE));
			}

			if (wasTracking && system->lastTrackingBoidId == ECS::INVALID_E_ID)
			{
				this->rangeChecker->setVisible(false);
				this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::TRACK), cocos2d::Color3B::WHITE);
			}
		}
		else if (mouseButton == 2)
		{
			bool entityClicked = system->updateMouseDown(mouseButton, point);
			if (!entityClicked)
			{
				this->createNewObstacle(point);
				this->labelsNode->playAnimation(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::ADD_OBSTACLE));
			}
			else
			{
				this->labelsNode->playAnimation(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::REMOVE_OBSTACLE));
			}
		}
	}

	/*
	if (mouseButton == 0)
	{
		// Left click
		if (this->displayBoundary.containsPoint(point))
		{
			
		}
	}
	else if (mouseButton == 1)
	{
		if (this->displayBoundary.containsPoint(point))
		{

		}
	}
	else if (mouseButton == 2)
	{
		// Middle click
		if (this->displayBoundary.containsPoint(point))
		{


			this->entities.push_back(createNewObstacle(point));
            this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::ADD_OBSTACLE), cocos2d::Color3B::GREEN);
		}
	}
	*/
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
		// Toggle pause simulation. Still counts fps and entity
		auto system = ECS::Manager::getInstance()->getSystem<ECS::FlockingSystem>();
		if (system->isActive())
		{
			system->deactivate();
			this->labelsNode->updateTimeTakenLabel("0");
			this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::PAUSE), cocos2d::Color3B::GREEN);
		}
		else
		{
			system->activate();
			this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::PAUSE), cocos2d::Color3B::WHITE);
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_C)
	{
		std::vector<ECS::Entity*> entities;
		ECS::Manager::getInstance()->getAllEntitiesForSystem<ECS::FlockingSystem>(entities);

		for (auto entity : entities)
		{
			entity->kill();
		}

		this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::CLEAR));
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_A)
	{
		const int size = ECS::Manager::getInstance()->getAliveEntityCountInEntityPool("FB");
		if (size != 512)
		{
			// Add ten entities
			for (int i = 0; i < 10; i++)
			{
				createNewBoid();
			}

			this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::ADD_TEN));
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_E)
	{
		// Remove last 10 entities
		std::vector<ECS::Entity*> entities;
		ECS::Manager::getInstance()->getAllEntitiesInPool(entities, "FB");
		int count = 0;
		for (auto entity : entities)
		{
			if (count == 10)
			{
				break;
			}
			entity->kill();
			count++;
		}

		this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::REMOVE_TEN));
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_S)
	{
		auto system = ECS::Manager::getInstance()->getSystem<ECS::FlockingSystem>();
		system->smoothSteering = !system->smoothSteering;

		std::vector<ECS::Entity*> entities;
		ECS::Manager::getInstance()->getAllEntitiesInPool(entities, "FB");

		for (auto entity : entities)
		{
			auto dataComp = entity->getComponent<ECS::FlockingData>();
			if (dataComp->type == ECS::FlockingData::TYPE::BOID)
			{
				auto dirVecComp = entity->getComponent<ECS::DirectionVector>();
				dirVecComp->smoothSteer = system->smoothSteering;
			}
		}

		if (system->smoothSteering)
		{
			this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::SMOOTH_STEERING), cocos2d::Color3B::GREEN);
		}
		else
		{
			this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::SMOOTH_STEERING), cocos2d::Color3B::WHITE);
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_O)
    {
        // This is added for laptop users because they might not have middle click
        if(this->curMousePosition.x == -1.0f || this->curMousePosition.y == -1.0f)
        {
            // mouse position must be positive. It's either out of window or hasn't intialized
            return;
        }

        if (this->displayBoundary.containsPoint(this->curMousePosition))
        {
			std::vector<ECS::Entity*> entities;
			ECS::Manager::getInstance()->getAllEntitiesInPool(entities, "FO");

			for (auto entity : entities)
			{
				auto dataComp = entity->getComponent<ECS::FlockingData>();
				if (dataComp->type == ECS::FlockingData::TYPE::OBSTACLE)
				{
					auto spriteComp = entity->getComponent<ECS::Sprite>();
					if (spriteComp->sprite->getPosition().distance(this->curMousePosition) < 6.0f)
					{
						entity->kill();
						this->labelsNode->setColor(LabelsNode::TYPE::MOUSE_OVER_AND_KEY, static_cast<int>(USAGE_MOUSE_OVER_AND_KEY::REMOVE_OBSTACLE), cocos2d::Color3B::WHITE);
						return;
					}
				}
			}
            
			// Create obstacle
			if (entities.size() < 32)
			{
				this->createNewObstacle(this->curMousePosition);
				this->labelsNode->setColor(LabelsNode::TYPE::MOUSE_OVER_AND_KEY, static_cast<int>(USAGE_MOUSE_OVER_AND_KEY::ADD_OBSTACLE), cocos2d::Color3B::WHITE);
			}
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

	ECS::Manager::deleteInstance();
}
