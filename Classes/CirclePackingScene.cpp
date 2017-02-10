#include "CirclePackingScene.h"
#include "MainScene.h"
#include <algorithm>	// std::random_shuffle
#include <utility>		// std::swap
#include <random>
#include "Utility.h"	// custom random

#ifdef _WIN32
#include <direct.h>
#include <stdio.h>
#define GetCurrentDir _getcwd
#elif __APPLE__ || linux
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

USING_NS_CC;
using namespace ECS;

CirclePackingScene* CirclePackingScene::createScene()
{
	CirclePackingScene* newCirclePackingScene = CirclePackingScene::create();
	return newCirclePackingScene;
}

bool CirclePackingScene::init()
{
	if (!cocos2d::Scene::init())
	{
		return false;
	}
 
	ECS::Manager::getInstance();

	// Uncomment this to activate update(float) function
	this->scheduleUpdate();

	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	// Init pause flag
	this->showOriginalImage = false;

	// Init labels node
	this->labelsNode = LabelsNode::createNode();
	this->labelsNode->setSharedLabelPosition(LabelsNode::SHARED_LABEL_POS_TYPE::CIRCLE_PACKING_SCENE);
	this->addChild(this->labelsNode);

	// Only here, set anchorpoint x to mid
	this->labelsNode->titleLabel->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
	this->labelsNode->initTitleStr("Circle Packing", cocos2d::Vec2(winSize.width * 0.5f, winSize.height - 30.0f));

	const int customLabelSize = 25;

	float labelX = winSize.width * 0.68f;
	float labelY = winSize.height * 0.8f;
	this->labelsNode->customLabelStartPos = cocos2d::Vec2(labelX, labelY);

	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Status: Waiting", customLabelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Image size (w x h): NA", customLabelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Possible spawn points: 0", customLabelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Total circles: 0", customLabelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Growing circles: 0", customLabelSize);

	const float customLastY = this->labelsNode->customLabels.back()->getBoundingBox().getMinY();
	const float blockGap = 22.0f;

	this->labelsNode->keyboardUsageLabelStartPos = cocos2d::Vec2(labelX, customLastY - blockGap);

	const int headerSize = 25;
	const int labelSize = 20;

	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Keys (Green = enabled)", headerSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Space: Toggle update", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "I: Toggle original image", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "R: Restart algorithm", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "S: Save circle packing as image", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "C: Stop and clear algorithm", labelSize);

	this->circleNode = cocos2d::Node::create();
	this->circleNode->setPosition(winSize * 0.5f);
	this->addChild(this->circleNode);

	std::string fontPath = "fonts/Rubik-Medium.ttf";

	// Image name and purpose label
	const float imageNameY = winSize.height - 80.0f;
	this->imageNameLabel = cocos2d::Label::createWithTTF("", fontPath, 27);
	this->imageNameLabel->setPosition(cocos2d::Vec2(winSize.width * 0.5f, imageNameY));
	this->addChild(this->imageNameLabel);

	this->imageTestPurposeLabel = cocos2d::Label::createWithTTF("", fontPath, 20);
	this->imageTestPurposeLabel->setPosition(cocos2d::Vec2(winSize.width * 0.5f, imageNameY - 20.0f));
	this->addChild(this->imageTestPurposeLabel);


	this->imageSelectInstructionLabel = cocos2d::Label::createWithTTF("<- Select image to start", fontPath, 40);
	this->imageSelectInstructionLabel->setPosition(cocos2d::Vec2(winSize.width * 0.35f, winSize.height * 0.5f));
	this->addChild(this->imageSelectInstructionLabel);

	this->imageSelectNode = cocos2d::Node::create();
	this->imageSelectNode->setPosition(cocos2d::Vec2(0, winSize.height * 0.5f));
	this->addChild(this->imageSelectNode);

	this->imageSelectPanelBg = cocos2d::Sprite::createWithSpriteFrameName("dot.png");
	this->imageSelectPanelBg->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->imageSelectPanelBg->setScaleY(winSize.height);
	this->imageSelectPanelBg->setScaleX(120.0f);
	this->imageSelectPanelBg->setColor(cocos2d::Color3B::GRAY);
	this->imageSelectPanelBg->setOpacity(30);
	this->imageSelectNode->addChild(this->imageSelectPanelBg);
    
	// Init images
	initImages();

	// speed modifier
	this->simulationSpeedModifier = 1.0f;

	this->sliderLabelNode = SliderLabelNode::createNode();
	this->sliderLabelNode->sliderStartPos = cocos2d::Vec2(winSize.width * 0.5f, 40);
	this->sliderLabelNode->addSlider("Simulation Speed", "Slider", 50, CC_CALLBACK_1(CirclePackingScene::onSliderClick, this));
	this->sliderLabelNode->sliderLabels.back().label->setAnchorPoint(cocos2d::Vec2(0.5f, 0));
	this->sliderLabelNode->sliderLabels.back().slider->setAnchorPoint(cocos2d::Vec2(0.5f, 0));
	this->addChild(this->sliderLabelNode);

	return true;
}

void CirclePackingScene::onEnter()
{
	cocos2d::Scene::onEnter();
	initECS();
	initInputListeners();
}

void CirclePackingScene::initECS()
{
	ECS::Manager* m = ECS::Manager::getInstance();
	
	m->createEntityPool("ALL_GROWN", 2048);
	m->createEntityPool("GROWING", 1024);

	auto system = m->createSystem<ECS::CirclePackingSystem>();
	system->addComponentType<ECS::Sprite>();
	system->addComponentType<ECS::CirclePackingData>();
	system->disbaleDefafultEntityPool();
	system->addEntityPoolName("ALL_GROWN");
	system->addEntityPoolName("GROWING");

	// Init current image index
	system->currentImageIndex = ECS::CirclePackingSystem::IMAGE_INDEX::NONE;
}

void CirclePackingScene::update(float delta)
{
	this->labelsNode->updateFPSLabel(delta);

	auto m = ECS::Manager::getInstance();
	auto system = m->getSystem<ECS::CirclePackingSystem>();

	if (!system->isActive())
	{
		return;
	}

	if (system->finished || system->pause)
	{
		// Algorithm is finished
		return;
	}

	// Apply simulation speed modifier
	delta *= this->simulationSpeedModifier;

	Utility::Time::start();

	// build quad tree with current circles
	std::vector<ECS::Entity*> allCircles;
	m->getAllEntitiesForSystem<ECS::CirclePackingSystem>(allCircles);
	system->insertCirclesToQuadTree(allCircles);

	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::SPAWNED_CIRCLES), "Spawned circles: " + std::to_string(allCircles.size()));

	// Spawn new circles
	int spawnedCircleCount = system->spawnCircles(*this->circleNode);
	
	// Update circle growth
	std::vector<ECS::Entity*> growingCircles;
	m->getAllEntitiesInPool(growingCircles, "GROWING");
	system->updateCircleGrowth(delta, growingCircles);

	// insert all circles to quad tree with new circles
	allCircles.clear();
	m->getAllEntitiesForSystem<ECS::CirclePackingSystem>(allCircles);
	system->insertCirclesToQuadTree(allCircles);

	// Update circle collision
	system->updateCircleCollisionResolution(growingCircles);

	// Move all grown circles to ALL_GROWN entityPool
	int growingCirclesCount = system->moveAllGrownCircles(growingCircles);

	Utility::Time::stop();

	std::string timeTakenStr = Utility::Time::getElaspedTime();	// Microseconds
	float timeTakenF = std::stof(timeTakenStr);	// to float
	timeTakenF *= 0.001f; // To milliseconds

	this->labelsNode->updateTimeTakenLabel(std::to_string(timeTakenF).substr(0, 5));

	if (system->circleSpawnPointsWithColor.empty() && spawnedCircleCount == 0 && growingCirclesCount == 0)
	{
		if (system->finished == false)
		{
			system->finished = true;
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Finished");
			this->labelsNode->updateTimeTakenLabel("0");
		}
	}

	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::GROWING_CIRCLES), "Growing circles: " + std::to_string(growingCirclesCount));

	// update label
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::POSSIBLE_SPAWN_POINTS), "Possible spawn points: " + std::to_string(system->circleSpawnPointsWithColor.size()));
}

void CirclePackingScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(CirclePackingScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(CirclePackingScene::onMouseDown, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(CirclePackingScene::onKeyPressed, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void CirclePackingScene::initImages()
{
	int size = static_cast<int>(ECS::CirclePackingSystem::IMAGE_INDEX::MAX_SIZE);
	for (int i = 0; i < size; i++)
	{
		this->images.push_back(new cocos2d::Image());
		this->images.back()->autorelease();
		this->images.back()->retain();

		auto index = static_cast<ECS::CirclePackingSystem::IMAGE_INDEX>(i);

		switch (index)
		{
		case ECS::CirclePackingSystem::IMAGE_INDEX::CPP:
		{
			initImageAndSprite("Images/C++.png", BUTTON_TAG::CPP);
		}
			break;
		case ECS::CirclePackingSystem::IMAGE_INDEX::CAT:
		{
			initImageAndSprite("Images/Shrek Cat.png", BUTTON_TAG::CAT);
		}
			break;
		case ECS::CirclePackingSystem::IMAGE_INDEX::THE_SCREAM:
		{
			initImageAndSprite("Images/TheScream.png", BUTTON_TAG::THE_SCREAM);
		}
			break;
		case ECS::CirclePackingSystem::IMAGE_INDEX::GRADIENT:
		{
			initImageAndSprite("Images/Gradient.png", BUTTON_TAG::GRADIENT);
		}
			break;
		default:
			break;
		}
	}

	int index = 0;
	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();
	cocos2d::Vec2 pos = cocos2d::Vec2(this->imageSelectPanelBg->getBoundingBox().size.width * 0.5f, (winSize.height * 0.5f) - 10.0f);
	for (auto icon : this->imageSpritesIconButtons)
	{
		float iconHeight = icon->getBoundingBox().size.height;
		if (index == 0)
		{
			pos.y -= iconHeight * 0.5f;
		}
		else
		{
			float prevIconHeight = this->imageSpritesIconButtons.at(index - 1)->getBoundingBox().size.height;
			pos.y -= ((prevIconHeight * 0.5f) + (iconHeight * 0.5f) + 15.0f);
		}
		icon->setPosition(pos);
		index++;
	}
}

void CirclePackingScene::initImageAndSprite(const std::string& imageName, const BUTTON_TAG buttonTag)
{
	this->images.back()->initWithImageFile(imageName);

	cocos2d::Texture2D* tex = new cocos2d::Texture2D();
	tex->initWithImage(this->images.back());
	tex->autorelease();

	//Sprite
	cocos2d::Sprite* sprite = cocos2d::Sprite::createWithTexture(tex);
	sprite->retain();
	sprite->setAnchorPoint(cocos2d::Vec2(0, 0));
	auto sizeHalf = sprite->getContentSize() * 0.5f;
	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();
	auto winSizeHalf = winSize * 0.5f;
	sprite->setPosition(cocos2d::Vec2(winSizeHalf - sizeHalf));
	sprite->setVisible(false);
	this->addChild(sprite, static_cast<int>(SPRITE_Z_ORDER::ABOVE_CIRCLES));
	this->imageSprites.push_back(sprite);

	// Icon Buttons
	cocos2d::ui::Button* button = cocos2d::ui::Button::create(imageName);
	button->setScale(100.0f / sprite->getContentSize().width);
	button->setActionTag(buttonTag);
	button->addClickEventListener(CC_CALLBACK_1(CirclePackingScene::onButtonPressed, this));
	this->imageSelectNode->addChild(button);
	this->imageSpritesIconButtons.push_back(button);
}

void CirclePackingScene::runCirclePacking(const ECS::CirclePackingSystem::IMAGE_INDEX imageIndex)
{
	// Hide instruction
	this->imageSelectInstructionLabel->setVisible(false);

	auto m = ECS::Manager::getInstance();
	auto system = m->getSystem<ECS::CirclePackingSystem>();

	system->finished = false;
	system->releaseQuadTree();
	system->resetCircles();
	system->currentImageIndex = imageIndex;
	system->findCircleSpawnPoint(*this->images.at(static_cast<int>(system->currentImageIndex)), *this->imageSprites.at(static_cast<int>(system->currentImageIndex)));

	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::POSSIBLE_SPAWN_POINTS), "Possible spawn points: " + std::to_string(system->circleSpawnPointsWithColor.size()));

	this->setImageNameAndSizeLabel();
	this->initQuadTree();

	if (system->pause)
	{
		this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Paused");
	}
	else
	{
		this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Running");
	}

    for(auto sprite : this->imageSprites)
    {
        sprite->setVisible(false);
    }
    
    this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::SHOW_ORIGINAL_IMAGE), cocos2d::Color3B::WHITE, false);
    
    this->showOriginalImage = false;
}

void CirclePackingScene::setImageNameAndSizeLabel()
{
	
	switch (ECS::Manager::getInstance()->getSystem<ECS::CirclePackingSystem>()->currentImageIndex)
	{
	case ECS::CirclePackingSystem::IMAGE_INDEX::CPP:
	{
		this->imageNameLabel->setString("C++");
		this->imageTestPurposeLabel->setString("Circle Packing on white pixels");
	}
		break;
	case ECS::CirclePackingSystem::IMAGE_INDEX::CAT:
	{
		this->imageNameLabel->setString("Shrek Cat");
		this->imageTestPurposeLabel->setString("Circle Packing with color");
	}
		break;
	case ECS::CirclePackingSystem::IMAGE_INDEX::THE_SCREAM:
	{
		this->imageNameLabel->setString("Edvard Munch's 'The Scream'");
		this->imageTestPurposeLabel->setString("Circle Packing with color");
	}
		break;
	case ECS::CirclePackingSystem::IMAGE_INDEX::GRADIENT:
	{
		this->imageNameLabel->setString("Gradient");
		this->imageTestPurposeLabel->setString("Circle Packing related to alpha channel");
	}
		break;
	default:
        return;
		break;
	}

	auto system = ECS::Manager::getInstance()->getSystem<ECS::CirclePackingSystem>();
    
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::IMAGE_SIZE), "IMAGE SIZE = (w)" + std::to_string(this->images.at(static_cast<int>(system->currentImageIndex))->getWidth()) + ", (h)" + std::to_string(this->images.at(static_cast<int>(system->currentImageIndex))->getHeight()));
}

void CirclePackingScene::initQuadTree()
{
	auto system = ECS::Manager::getInstance()->getSystem<ECS::CirclePackingSystem>();
	if (system->currentImageIndex == ECS::CirclePackingSystem::IMAGE_INDEX::NONE) return;

	// Init quadtree with initial boundary
	auto bb = this->imageSprites.at(static_cast<int>(system->currentImageIndex))->getBoundingBox();
	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	bb.origin -= (winSize * 0.5f);

	ECS::Manager::getInstance()->getSystem<ECS::CirclePackingSystem>()->initQuadTree(bb);
}

void CirclePackingScene::onButtonPressed(cocos2d::Ref * sender)
{
	cocos2d::ui::Button* button = dynamic_cast<cocos2d::ui::Button*>(sender);
	const BUTTON_TAG tag = static_cast<BUTTON_TAG>(button->getActionTag());

	switch (tag)
	{
	case BUTTON_TAG::CPP:
		this->runCirclePacking(ECS::CirclePackingSystem::IMAGE_INDEX::CPP);
		break;
	case BUTTON_TAG::CAT:
		this->runCirclePacking(ECS::CirclePackingSystem::IMAGE_INDEX::CAT);
		break;
	case BUTTON_TAG::THE_SCREAM:
		this->runCirclePacking(ECS::CirclePackingSystem::IMAGE_INDEX::THE_SCREAM);
		break;
	case BUTTON_TAG::GRADIENT:
		this->runCirclePacking(ECS::CirclePackingSystem::IMAGE_INDEX::GRADIENT);
		break;
	default:
		break;
	}
}

void CirclePackingScene::onMouseMove(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	auto point = cocos2d::Vec2(x, y);

	this->labelsNode->updateMouseHover(point);
}

void CirclePackingScene::onMouseDown(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
//	int mouseButton = mouseEvent->getMouseButton();
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	auto point = cocos2d::Vec2(x, y);

	bool ret = this->labelsNode->updateMouseDown(point);
	if (ret)
	{
		return;
	}
}

void CirclePackingScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE)
	{
		cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
	}

	auto m = ECS::Manager::getInstance();
	auto system = m->getSystem<ECS::CirclePackingSystem>();
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_SPACE)
	{
		system->pause = !system->pause;
		if (system->pause)
		{
			this->labelsNode->updateTimeTakenLabel("0");
			this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::PAUSE), cocos2d::Color3B::GREEN);
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Paused");
		}
		else
		{
			this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::PAUSE), cocos2d::Color3B::WHITE);
			if (system->currentImageIndex == ECS::CirclePackingSystem::IMAGE_INDEX::NONE)
			{
				this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Waiting");
			}
			else
			{
				this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Running");
			}
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_R)
	{
		// Restart current circle packing
		if (system->currentImageIndex != ECS::CirclePackingSystem::IMAGE_INDEX::NONE)
		{
			this->runCirclePacking(system->currentImageIndex);
			this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::RESTART));
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_C)
	{
		if (system->currentImageIndex != ECS::CirclePackingSystem::IMAGE_INDEX::NONE)
		{
			this->imageSelectInstructionLabel->setVisible(false);
			// Stop and clear algorithm
			// Reset flag
			system->finished = true;
			// Delete quadTree
			system->releaseQuadTree();
			// Reset existing circles
			system->resetCircles();
			// Reset image index
			system->currentImageIndex = ECS::CirclePackingSystem::IMAGE_INDEX::NONE;

			this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::CLEAR));

			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Waiting");
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::IMAGE_SIZE), "Image size (w x h): NA");
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::POSSIBLE_SPAWN_POINTS), "Possible spawn points: 0");
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::SPAWNED_CIRCLES), "Total circles: 0");
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::GROWING_CIRCLES), "Growing circles: 0");

			this->labelsNode->updateTimeTakenLabel("0");
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_S)
	{
		// Save to image
		if (system->currentImageIndex != ECS::CirclePackingSystem::IMAGE_INDEX::NONE)
		{
			this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::SAVE));

			int w = this->images.at(static_cast<int>(system->currentImageIndex))->getWidth() + 50.0f;
			int h = this->images.at(static_cast<int>(system->currentImageIndex))->getHeight() + 50.0f;

			auto renderTexture = cocos2d::RenderTexture::create(w, h);
			auto winSize = cocos2d::Director::getInstance()->getVisibleSize();
			auto pos = cocos2d::Vec2::ZERO;
			pos.x = static_cast<float>(w) * 0.5f;
			pos.y = static_cast<float>(h) * 0.5f;
			this->circleNode->setPosition(pos);
			renderTexture->beginWithClear(0, 0, 0, 1);
			this->circleNode->visit();
			renderTexture->end();
			cocos2d::Director::getInstance()->getRenderer()->render();
			this->circleNode->setPosition(winSize * 0.5f);

			std::string workingDirectoryStr;

			char workingDirectory[FILENAME_MAX];
			size_t wdSize = sizeof(workingDirectory);
			if (!getcwd(workingDirectory, wdSize))
			{
				workingDirectoryStr = std::string();
			}
			else
			{
				// There is working directory.
				workingDirectoryStr = std::string(workingDirectory);
			}

			std::replace(workingDirectoryStr.begin(), workingDirectoryStr.end(), '\\', '/');
			cocos2d::Image* image = renderTexture->newImage();
			image->saveToFile(workingDirectoryStr + "/CirclePackingImage.png");
			delete image;
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_I)
	{
		// toggle original image
		this->showOriginalImage = !this->showOriginalImage;
		if (system->currentImageIndex != ECS::CirclePackingSystem::IMAGE_INDEX::NONE)
		{
			int index = static_cast<int>(system->currentImageIndex);
			if (index >= 0 && index < static_cast<int>(this->imageSprites.size()))
			{
				this->imageSprites.at(index)->setVisible(this->showOriginalImage);
				if (this->showOriginalImage)
				{
					this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::SHOW_ORIGINAL_IMAGE), cocos2d::Color3B::GREEN);
				}
				else
				{
					this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::SHOW_ORIGINAL_IMAGE), cocos2d::Color3B::WHITE);
				}
			}
		}
	}
}

void CirclePackingScene::onSliderClick(cocos2d::Ref* sender)
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

void CirclePackingScene::releaseInputListeners()
{
	if(this->mouseInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->mouseInputListener);
	if(this->keyInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->keyInputListener);
}

void CirclePackingScene::onExit()
{
	cocos2d::Scene::onExit();
	// Uncomment this if you are using initInputListeners()
	releaseInputListeners(); 

	this->images.clear();

	this->imageSprites.clear();
	
	ECS::Manager::deleteInstance();
}
