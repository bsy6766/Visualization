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

	ECS::Manager::getInstance();
	
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
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Space: Toggle update", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "C: Clear all Entities", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "A: Add 10 Entities", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "E: Remove 10 Entities", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "G: Toggle quad tree subdivision grid", labelSize);
    this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::GRID), cocos2d::Color3B::GREEN);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "D: Toggle duplication check", labelSize);
    this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::DUPL_CHECK), cocos2d::Color3B::GREEN);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "R: Toggle collision resolution", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "1: Increase Quad Tree max level", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "2: Decrease Quad Tree max level", labelSize);
    
	const float keysLastY = this->labelsNode->keyboardUsageLabels.back()->getBoundingBox().getMinY();
    this->labelsNode->mouseUsageLabelStartPos = cocos2d::Vec2(labelX, keysLastY - blockGap);
    
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Mouse", headerSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Left Click (In box): Add Entity", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Left Click (On Entity): Toggle Entity tracking", labelSize);
    this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Right Click (On Entity): Remove Entity", labelSize);

	// speed modifier
	this->simulationSpeedModifier = 1.0f;

	this->sliderLabelNode = SliderLabelNode::createNode();
	const float mouseLastY = this->labelsNode->mouseUsageLabels.back()->getBoundingBox().getMinY();
	this->sliderLabelNode->sliderStartPos = cocos2d::Vec2(labelX, mouseLastY - blockGap);
	this->sliderLabelNode->addSlider("Simulation Speed", "Slider", 50, CC_CALLBACK_1(QuadTreeScene::onSliderClick, this));
	this->addChild(this->sliderLabelNode);

	return true;
}

void QuadTreeScene::initECS()
{
	ECS::Manager* m = ECS::Manager::getInstance();
	m->createEntityPool("QT", 1024);

	// Create 40 entities at start
	const int initialEntityCount = 40;
	for (int i = 0; i < initialEntityCount; i++)
	{
		createNewEntity();
	}

	auto system = m->createSystem<QuadTreeSystem>();
	system->disbaleDefafultEntityPool();
	system->addEntityPoolName("QT");
	system->addComponentType<ECS::DirectionVector>();
	system->addComponentType<ECS::Sprite>();
	system->addComponentType<ECS::QTreeData>();

	system->initQuadTree(this->displayBoundary);
	system->quadTree->lineDrawNode = this->quadTreeLineNode->drawNode;
	system->displayBoundary = this->displayBoundary;

	// Not ECS stuff but init here.
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::QUAD_TREE_MAX_LEVEL), "Quad Tree max level: " + std::to_string(system->quadTree->getCurrentLevelSetting()));
}

void QuadTreeScene::createNewEntity(const cocos2d::Vec2& position)
{
	ECS::Manager* m = ECS::Manager::getInstance();
	ECS::Entity* newEntity = m->createEntity("QT");

	if (newEntity == nullptr)
	{
		return;
	}

	newEntity->addComponent<ECS::DirectionVector>();
	ECS::Sprite* spriteComp = m->createComponent<ECS::Sprite>();
	spriteComp->sprite = cocos2d::Sprite::createWithSpriteFrameName("quadTreeEntityBox.png");
	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();
	spriteComp->sprite->setPosition(winSize.height * 0.5f, winSize.height * 0.5f);
	spriteComp->sprite->retain();
	this->areaNode->addChild(spriteComp->sprite);
	spriteComp->sprite->setScaleX(Utility::Random::randomReal<float>(0.25f, 1.0f));
	spriteComp->sprite->setScaleY(Utility::Random::randomReal<float>(0.25f, 1.0f));
	spriteComp->sprite->setLocalZOrder(static_cast<int>(Z_ORDER::ENTITY));
	if (position == cocos2d::Vec2::ZERO)
	{
		spriteComp->setRandomPosInBoundary(this->displayBoundary);
	}
	else
	{
		spriteComp->sprite->setPosition(position);
	}
	newEntity->addComponent<ECS::Sprite>(spriteComp);
	newEntity->addComponent<ECS::QTreeData>();
}

void QuadTreeScene::onEnter()
{
	cocos2d::Scene::onEnter();

	initECS();
	
	initInputListeners();

	this->scheduleUpdate();
}

void QuadTreeScene::update(float delta)
{
	// Updates fps count and time 
    this->labelsNode->updateFPSLabel(delta);

	//Speed modifier
	delta *= this->simulationSpeedModifier;

	auto m = ECS::Manager::getInstance();
	auto system = m->getSystem<QuadTreeSystem>();

	int entityCount = static_cast<int>(m->getAliveEntityCountInEntityPool("QT"));
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::ENTITIES), "Entities: " + std::to_string(entityCount) + " / 1024");

	if(!system->isActive())
	{
		std::vector<ECS::Entity*> entities;
		m->getAllEnttitiesInPool(entities, "QT");
		system->rebuildQuadTree(entities);
		return;
	}

	Utility::Time::start();

	m->update(delta);

	Utility::Time::stop();

	std::string timeTakenStr = Utility::Time::getElaspedTime();	// Microseconds
	float timeTakenF = std::stof(timeTakenStr);	// to float
	timeTakenF *= 0.001f; // To milliseconds

	this->labelsNode->updateTimeTakenLabel(std::to_string(timeTakenF).substr(0, 5));

	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::COLLISION), "Collision check: " + std::to_string(system->collisionChecksCount));
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::COLLISION_WO_DUP_CHECK), "Collision check w/o duplication: " + std::to_string(system->collisionCheckWithOutRepeatCount));

    this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::BRUTE_FORCE), "Brute-force check: " + std::to_string(entityCount * entityCount));
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

	auto system = ECS::Manager::getInstance()->getSystem<QuadTreeSystem>();

	if (this->displayBoundary.containsPoint(point))
	{
		if (mouseButton == 0)
		{
			bool entityClicked = system->updateMouseDown(0, point);
			if (entityClicked)
			{
				// tracking
				// Clicked on entitiy sprite
				this->labelsNode->playAnimation(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::TRACK));
			}
			else
			{
				this->createNewEntity(point);
				this->labelsNode->playAnimation(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::ADD_ONE));
			}
		}
		else if (mouseButton == 1)
		{
			bool entityClicked = system->updateMouseDown(1, point);
			if (entityClicked)
			{
				// tracking
				// Clicked on entitiy sprite
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
		auto system = ECS::Manager::getInstance()->getSystem<QuadTreeSystem>();
		system->quadTree->increaseLevel();
        this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::QUAD_TREE_MAX_LEVEL), "Quad Tree max level: " + std::to_string(system->quadTree->getCurrentLevelSetting()));
        
        this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::INC_QTREE_LEVEL));
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_2)
	{
		// Decrease quadtree level
		auto system = ECS::Manager::getInstance()->getSystem<QuadTreeSystem>();
		system->quadTree->decreaseLevel();
        this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::QUAD_TREE_MAX_LEVEL), "Quad Tree max level: " + std::to_string(system->quadTree->getCurrentLevelSetting()));
        
        this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::DEC_QTREE_LEVEL));
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_SPACE)
	{
		// Toggle pause simulation. Still counts fps and entity
		auto system = ECS::Manager::getInstance()->getSystem<QuadTreeSystem>();
		if (system->isActive())
		{
			system->deactivate();
			this->labelsNode->updateTimeTakenLabel("0");
		}
		else
		{
			system->activate();
		}

        this->toggleColor(!system->isActive(), LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::PAUSE));
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_R)
	{
		// Toggle collision resolution
		auto system = ECS::Manager::getInstance()->getSystem<QuadTreeSystem>();
		system->collisionResolve = !system->collisionResolve;
        this->toggleColor(system->collisionResolve, LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::COL_RESOLVE));
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_C)
	{
		// Wipe all entities
 		std::vector<ECS::Entity*> entities;
		ECS::Manager::getInstance()->getAllEnttitiesInPool(entities, "QT");
		for (auto entity : entities)
		{
			entity->kill();
		}

        this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::CLEAR));

		this->labelsNode->updateTimeTakenLabel("0");
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_A)
	{
		// Add ten entities
		for (int i = 0; i < 10; i++)
		{
			createNewEntity();
		}
        
        this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::ADD_TEN));
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_E)
	{
		// Remove last 10 entities
		std::vector<ECS::Entity*> entities;
		ECS::Manager::getInstance()->getAllEnttitiesInPool(entities, "QT");
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
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_G)
	{
		// Toggle quadtree grid
		auto system = ECS::Manager::getInstance()->getSystem<QuadTreeSystem>();
		system->showGrid = !system->showGrid;
        this->toggleColor(system->showGrid, LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::GRID));
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_D)
	{
		// Toggle duplication check
		auto system = ECS::Manager::getInstance()->getSystem<QuadTreeSystem>();
		system->duplicationCheck = !system->duplicationCheck;
        if(system->duplicationCheck)
        {
            this->labelsNode->setColor(LabelsNode::TYPE::CUSTOM, static_cast<int>(CUSTOM_LABEL_INDEX::COLLISION_WO_DUP_CHECK), cocos2d::Color3B::WHITE, false);
        }
        else
        {
            this->labelsNode->setColor(LabelsNode::TYPE::CUSTOM, static_cast<int>(CUSTOM_LABEL_INDEX::COLLISION_WO_DUP_CHECK), cocos2d::Color3B::GRAY, false);
        }
        
        this->toggleColor(system->duplicationCheck, LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::DUPL_CHECK));
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

	ECS::Manager::deleteInstance();
}
