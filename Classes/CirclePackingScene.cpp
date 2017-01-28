#include "CirclePackingScene.h"
#include "MainScene.h"
#include <algorithm>	// std::random_shuffle
#include <utility>		// std::swap
#include <random>
#include "Component.h"
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
    
    this->finished = false;

	// Uncomment this to activate update(float) function
	this->scheduleUpdate();

	// Set spawn point serach offsets
	this->searchSpawnPointWidthOffset = 4;
	this->searchSpawnPointHeightOffset = 4;

	// Init current image index
	this->currentImageIndex = IMAGE_INDEX::NONE;

	// init max Circles to 0. Will be initialized later
	this->maxCircles = 0;

	// Set number of circles to spawn at start and every tick
	this->initialCircleCount = 10;
	this->circleSpawnRate = 2;

	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	// Init pause flag
	this->pause = false;
	this->showOriginalImage = false;

	// const
	const float imagePanelWidth = 120.0f;

	// Init labels node
	this->labelsNode = LabelsNode::createNode();
	this->labelsNode->setSharedLabelPosition(LabelsNode::SHARED_LABEL_POS_TYPE::CIRCLE_PACKING_SCENE);
	this->addChild(this->labelsNode);

	const int titleSize = 35;

	// Only here, set anchorpoint x to mid
	this->labelsNode->titleLabel->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
	this->labelsNode->initTitleStr("Circle Packing", cocos2d::Vec2(winSize.width * 0.5f, winSize.height - 30.0f));

	const int customLabelSize = 25;
	const int blankLineSize = 15;

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

	this->growingCircleCount = 0;

	this->quadTree = nullptr;

	return true;
}

void CirclePackingScene::onEnter()
{
	cocos2d::Scene::onEnter();

	initInputListeners();
}

void CirclePackingScene::update(float delta)
{
	this->labelsNode->updateFPSLabel(delta);

	if (this->pause)
	{
		return;
	}

	// check current image
	if (this->currentImageIndex == IMAGE_INDEX::NONE) return;
    
    if(this->finished)
    {
        // Algorithm is finished
        return;
    }

	// Modify time by multiplier
	delta *= this->simulationSpeedModifier;

	// Reset QuadTree
	insertEntitiesToQuadTree();

	// Spawn circles
	int spawnedCircleCount = spawnCircles();
    
    if(spawnedCircleCount == 0 && this->growingCircleCount == 0)
    {
        // Nothing has spawned and growing. Don't need to update
        // Check if algoritm is finished
        if(this->circleSpawnPointsWithColor.empty() ||  this->freshCircles.empty())
        {
            if(this->finished == false)
            {
                this->finished = true;
				this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Finished");
            }
        }
        return;
    }
    
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::SPAWNED_CIRCLES), "Spawned circles: " + std::to_string(this->activeCircles.size()));

	// Update growth
	updateCircleGrowth(delta);

	// Reset QuadTree
	insertEntitiesToQuadTree();

	// Update growth
	updateCircleCollisionResolution();

	// Move all grown circles to another list
	this->moveAllGrownCircles();
    
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::GROWING_CIRCLES), "Growing circles: " + std::to_string(this->growingCircleCount));
}

void CirclePackingScene::updateCircleGrowth(const float delta)
{
	for (auto& circle : this->activeCircles)
	{
		auto dataComp = circle->getComponent<CirclePackingData*>(CIRCLE_PACKING_DATA);
		auto spriteComp = circle->getComponent<ECS::Sprite*>(SPRITE);
		if (dataComp->growing)
		{
			dataComp->update(delta);

			if (spriteComp)
			{
				float newRadius = dataComp->radius;
				spriteComp->sprite->setScale(newRadius / 50.0f);
			}
		}
	}
}

void CirclePackingScene::updateCircleCollisionResolution()
{
	if (this->growingCircleCount <= 0)
	{
		// No circles are growing at this moment
		return;
	}
    
    int size = static_cast<int>(this->activeCircles.size());
    int totalComparisonCount = 0;
    
	for (auto activeCircle : this->activeCircles)
	{
		// Create query box
		auto leftSpriteComp = activeCircle->getComponent<ECS::Sprite*>(SPRITE);
		cocos2d::Rect queryBox = leftSpriteComp->sprite->getBoundingBox();
		float pad = 30.0f;
		queryBox.origin.x -= pad;
		queryBox.origin.y -= pad;
		queryBox.size.width += pad * 2.0f;
		queryBox.size.height += pad * 2.0f;

		std::list<ECS::Entity*> nearCircles;
		quadTree->queryAllEntities(queryBox, nearCircles);

		if (nearCircles.empty())
		{
			continue;
		}
        
        totalComparisonCount += static_cast<int>(nearCircles.size());

		for (auto nearCircle : nearCircles)
		{
			if (activeCircle->id != nearCircle->id)
			{
				auto leftDataComp = activeCircle->getComponent<CirclePackingData*>(CIRCLE_PACKING_DATA);
				auto rightDataComp = nearCircle->getComponent<CirclePackingData*>(CIRCLE_PACKING_DATA);

				if (leftDataComp->growing || rightDataComp->growing)
				{
					//either one of circle must be growing. If both are not growing, don't need to check
					float distance = leftDataComp->position.distance(rightDataComp->position);
					float minDistance = leftDataComp->radius + rightDataComp->radius;
					if (distance <= minDistance)
					{
						// These two circle touched each other
						float distanceDiff = fabsf(minDistance - distance);
						assert(distanceDiff >= 0);

						auto leftSpriteComp = activeCircle->getComponent<ECS::Sprite*>(SPRITE);
						auto rightSpriteComp = nearCircle->getComponent<ECS::Sprite*>(SPRITE);

						// Update radius, sprite and stop growing
						if (leftDataComp->growing && rightDataComp->growing)
						{
							// Both were growing. Share overlapped distance
							leftDataComp->radius -= (distanceDiff * 0.5f);
							rightDataComp->radius -= (distanceDiff * 0.5f);

							leftSpriteComp->sprite->setScale(leftDataComp->radius / 50.0f);
							rightSpriteComp->sprite->setScale(rightDataComp->radius / 50.0f);

							leftDataComp->growing = false;
							rightDataComp->growing = false;
						}
						else if (leftDataComp->growing && !rightDataComp->growing)
						{
							// Only left circle was growing. All grown circles should not be modified
							leftDataComp->radius -= distanceDiff;

							leftSpriteComp->sprite->setScale(leftDataComp->radius / 50.0f);

							leftDataComp->growing = false;
						}
						else if (!leftDataComp->growing && rightDataComp->growing)
						{
							// Only right circle was growing. All grown circles should not be modified
							rightDataComp->radius -= distanceDiff;

							rightSpriteComp->sprite->setScale(rightDataComp->radius / 50.0f);

							rightDataComp->growing = false;
						}
						// Else, can't be both circles are all grown
					}
				}
			}
		}
	}
}

void CirclePackingScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(CirclePackingScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(CirclePackingScene::onMouseDown, this);
	this->mouseInputListener->onMouseUp = CC_CALLBACK_1(CirclePackingScene::onMouseUp, this);
	this->mouseInputListener->onMouseScroll = CC_CALLBACK_1(CirclePackingScene::onMouseScroll, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(CirclePackingScene::onKeyPressed, this);
	this->keyInputListener->onKeyReleased = CC_CALLBACK_2(CirclePackingScene::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void CirclePackingScene::initImages()
{
	int size = static_cast<int>(IMAGE_INDEX::MAX_SIZE);
	for (int i = 0; i < size; i++)
	{
		this->images.push_back(new cocos2d::Image());
		this->images.back()->autorelease();
		this->images.back()->retain();

		auto index = static_cast<IMAGE_INDEX>(i);

		switch (index)
		{
		case IMAGE_INDEX::CPP:
		{
			initImageAndSprite("Images/C++.png", BUTTON_TAG::CPP);
		}
			break;
		case IMAGE_INDEX::CAT:
		{
			initImageAndSprite("Images/Shrek Cat.png", BUTTON_TAG::CAT);
		}
			break;
		case IMAGE_INDEX::THE_SCREAM:
		{
			initImageAndSprite("Images/TheScream.png", BUTTON_TAG::THE_SCREAM);
		}
			break;
		case IMAGE_INDEX::GRADIENT:
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

void CirclePackingScene::findCircleSpawnPoint(const IMAGE_INDEX imageIndex)
{
	if (this->circleSpawnPointsWithColor.empty() == false)
	{
		std::queue<SpawnPoint> empty;
		std::swap(this->circleSpawnPointsWithColor, empty);
	}

	unsigned int index = static_cast<unsigned int>(imageIndex);

	int x = 3;
	if (this->images.at(index)->hasAlpha())
	{
		x = 4;
	}

	unsigned char *data = this->images.at(index)->getData();

	// [0][0] => Left-Top Pixel !
	// But cocos2d Location Y-axis is Bottom(0) to Top(max)

	int width = this->images.at(index)->getWidth();
	int height = this->images.at(index)->getHeight();

	std::vector<SpawnPoint> points;

	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	for (int i = 0; i < width; i += this->searchSpawnPointWidthOffset)
	{
		for (int j = 0; j < height; j += this->searchSpawnPointHeightOffset)
		{
			unsigned char *pixel = data + (i + j * width) * x;

			// You can see/change pixels' RGBA value(0-255) here !
			unsigned char r = *pixel;
			unsigned char g = *(pixel + 1);
			unsigned char b = *(pixel + 2);
			unsigned char a = *(pixel + 3);

			switch (imageIndex)
			{
			case IMAGE_INDEX::CPP:
			{
				if (a <= 0)
				{
					// ignore if pixel is completely transparent.
					continue;
				}
				else
				{
					// On default image, only detect white pixel
					if (r == 255 && g == 255 && b == 255)
					{
						auto point = this->pixelToPoint(i, j, height, this->imageSprites.at(index)->getPosition());
						point -= (winSize * 0.5f);

						cocos2d::Color4F color = cocos2d::Color4F(cocos2d::RandomHelper::random_real<float>(0, 1.0f),
								cocos2d::RandomHelper::random_real<float>(0, 1.0f),
								cocos2d::RandomHelper::random_real<float>(0, 1.0f),
								1.0f);
						points.push_back(SpawnPoint{ point, color });
					}
					else
					{
						continue;
					}
				}
			}
				break;
			case IMAGE_INDEX::CAT:
			case IMAGE_INDEX::THE_SCREAM:
			{
				if (a > 0)
				{
					// For cat and The scream, we are doing color test, all points that are visible will be spawn point
					auto point = this->pixelToPoint(i, j, height, this->imageSprites.at(index)->getPosition());
					point -= (winSize * 0.5f);

					points.push_back(SpawnPoint{ point, cocos2d::Color4F(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f) });
				}
				else
				{
					continue;
				}
			}
				break;
			case IMAGE_INDEX::GRADIENT:
			{
				if (a > 0)
				{
					// For cat, all points that are visible will be spawn point
					auto point = this->pixelToPoint(i, j, height, this->imageSprites.at(index)->getPosition());
					point -= (winSize * 0.5f);
					cocos2d::Color4F color = cocos2d::Color4F::WHITE;
					color.a = a / 255.0f;

					points.push_back(SpawnPoint{ point, color });
				}
				else
				{
					continue;
				}
			}
				break;
			default:
				break;
			}
		}
	}

	if (!points.empty())
	{
		int size = static_cast<int>(points.size());
		int newSize = size / 10 * 8;
		this->maxCircles = newSize;
		
		std::shuffle(std::begin(points), std::end(points), std::mt19937(std::random_device{}()));
		points.resize(newSize);

		for (int i = 0; i < newSize; i++)
		{
			this->circleSpawnPointsWithColor.push(points.at(i));
		}
	}

	points.clear();
}

void CirclePackingScene::initCircles()
{
	int size = static_cast<int>(this->freshCircles.size());

	if (size >= this->maxCircles)
	{
		// We already have enough circles. resize. Circles will be deallocated
		auto it = this->freshCircles.begin();
		std::advance(it, this->maxCircles);
		for (; it != this->freshCircles.end();)
		{
			delete (*it);
			it = this->freshCircles.erase(it);
			continue;
		}

		assert(this->freshCircles.size() == this->maxCircles);

		return;
	}
	else
	{
		// Fill fresh circles by max size. If there are fresh circles left, resue them
		for (int i = this->freshCircles.size(); i < this->maxCircles; i++)
		{
			this->freshCircles.push_back(this->createNewEntity());
		}
	}
}

const bool CirclePackingScene::moveAllGrownCircles()
{
	this->growingCircleCount = 0;
	bool grownCircleFound = false;
	auto it = this->activeCircles.begin();
	for (;it != this->activeCircles.end();)
	{
		// Circles that are growing are placed at front of list
		// Circles that are all grown are placed at the back of list.
		// So iterate until it finds alive and all grown circle
		auto dataComp = (*it)->getComponent<CirclePackingData*>(CIRCLE_PACKING_DATA);
		if (dataComp->growing == false)
		{
			// Move all grown circle back to list
			this->activeCircles.splice(this->activeCircles.end(), this->activeCircles, it);
			grownCircleFound = true;
		}
		else
		{
			this->growingCircleCount++;
		}

		it++;
	}

	return grownCircleFound;
}

const int CirclePackingScene::spawnCircles()
{
	int count = 0;
    int attempt = 0;
	// Both spawn point and fresh circles must not be empty
	if (!this->circleSpawnPointsWithColor.empty() && !this->freshCircles.empty())
	{
		// Run until spawn point exists and fill spawn rate
		while (count < this->circleSpawnRate && this->circleSpawnPointsWithColor.size() > 0)
		{
            // increment attempt counter
            attempt++;
            
            if(attempt > this->MAXIMUM_SPAWN_ATTEMPT)
            {
                // If this function attempted 30 times to spawn point, end it.
                return count;
            }
            
			// get first spawn point
			auto spawnPoint = this->circleSpawnPointsWithColor.front();

			// flag
			bool inCircle = false;

			// Query quad tree
			cocos2d::Rect queryingArea = cocos2d::Rect::ZERO;
			queryingArea.origin = spawnPoint.point;
			const float pad = 50.0f;
			queryingArea.origin.x -= pad;
			queryingArea.origin.y -= pad;
			queryingArea.size.width += pad * 2.0f;
			queryingArea.size.height += pad * 2.0f;

			std::list<ECS::Entity*> nearCircles;

			this->quadTree->queryAllEntities(queryingArea, nearCircles);

			for (auto activeCircle : nearCircles)
			{
				auto dataComp = activeCircle->getComponent<CirclePackingData*>(CIRCLE_PACKING_DATA);
				float distance = dataComp->position.distance(spawnPoint.point);
                float pad = CirclePackingData::initialRadius * 0.5f;
				if (distance <= (dataComp->radius + pad))
				{
					// Spawn point is already covered by another circle. 
					inCircle = true;
					break;
				}
			}

			if (inCircle)
			{
				// Spawn point in circle. Pop this point and continue
                this->circleSpawnPointsWithColor.pop();
                
                // update label
				this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::POSSIBLE_SPAWN_POINTS), "Possible spawn points: " + std::to_string(this->circleSpawnPointsWithColor.size()));
				continue;
			}

			// Point is not covered by other circles.
			auto it_front = this->freshCircles.begin();
			// activate circle
			auto dataComp = (*it_front)->getComponent<CirclePackingData*>(CIRCLE_PACKING_DATA);
			dataComp->activate(spawnPoint.point, 1.0f, spawnPoint.color);
			// Move to activeCircles
			this->activeCircles.splice(this->activeCircles.begin(), this->freshCircles, it_front);
			// pop spawn point
            this->circleSpawnPointsWithColor.pop();
            
            // update label
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::POSSIBLE_SPAWN_POINTS), "Possible spawn points: " + std::to_string(this->circleSpawnPointsWithColor.size()));
			// update sprite comp
			auto spriteComp = (*it_front)->getComponent<ECS::Sprite*>(SPRITE);
			if (spriteComp)
			{
				spriteComp->sprite->setScale(0.02f);
				spriteComp->sprite->setPosition(spawnPoint.point);
				spriteComp->sprite->setColor(cocos2d::Color3B(spawnPoint.color));
				spriteComp->sprite->setOpacity(spawnPoint.color.a * 255.0f);
				spriteComp->sprite->setVisible(true);
			}
			// inc count
			count++;
		}
	}
    
    return count;
}

void CirclePackingScene::resetCircles()
{
	for (auto& circle : this->activeCircles)
	{
		auto dataComp = circle->getComponent<CirclePackingData*>(CIRCLE_PACKING_DATA);
		dataComp->deactivate();
		auto spriteComp = circle->getComponent<ECS::Sprite*>(SPRITE);
		if (spriteComp)
		{
			spriteComp->sprite->setVisible(false);
		}
	}

	this->freshCircles.splice(this->freshCircles.begin(), this->activeCircles);
	
	this->maxCircles = 0;
}

cocos2d::Vec2 CirclePackingScene::pixelToPoint(const int x, const int y, const int height, const cocos2d::Vec2& spritePos)
{
	cocos2d::Vec2 point = cocos2d::Vec2(x, y);
	point.y = height - point.y;
	point += spritePos;
	return point;
}

void CirclePackingScene::runCirclePacking(const IMAGE_INDEX imageIndex)
{
	// Hide instruction
	if (this->imageSelectInstructionLabel->isVisible())
	{
		this->imageSelectInstructionLabel->setVisible(false);
	}

    // Reset flag
    this->finished = false;
	// Delete quadTree
	releaseQuadTree();
	// Reset counter
	this->growingCircleCount = 0;
	// Reset existing circles
	this->resetCircles();
	// update image index
	this->currentImageIndex = imageIndex;
	// reset spawn point
	this->findCircleSpawnPoint(imageIndex);
	// set max circles size
	this->maxCircles = this->circleSpawnPointsWithColor.size();
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::POSSIBLE_SPAWN_POINTS), "Possible spawn points: " + std::to_string(this->circleSpawnPointsWithColor.size()));
	// initialize circles
	initCircles();
	// update image name label
	this->setImageNameAndSizeLabel();
	// Re-initialize QuadTree
	initQuadTree();
    // Change status
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Running");
}

void CirclePackingScene::setImageNameAndSizeLabel()
{
	switch (this->currentImageIndex)
	{
	case IMAGE_INDEX::CPP:
	{
		this->imageNameLabel->setString("C++");
		this->imageTestPurposeLabel->setString("Circle Packing on white pixels");
	}
		break;
	case IMAGE_INDEX::CAT:
	{
		this->imageNameLabel->setString("Shrek Cat");
		this->imageTestPurposeLabel->setString("Circle Packing with color");
	}
		break;
	case IMAGE_INDEX::THE_SCREAM:
	{
		this->imageNameLabel->setString("Edvard Munch's 'The Scream'");
		this->imageTestPurposeLabel->setString("Circle Packing with color");
	}
		break;
	case IMAGE_INDEX::GRADIENT:
	{
		this->imageNameLabel->setString("Gradient");
		this->imageTestPurposeLabel->setString("Circle Packing related to alpha channel");
	}
		break;
	default:
        return;
		break;
	}
    
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::IMAGE_SIZE), "IMAGE SIZE = (w)" + std::to_string(this->images.at(static_cast<int>(this->currentImageIndex))->getWidth()) + ", (h)" + std::to_string(this->images.at(static_cast<int>(this->currentImageIndex))->getHeight()));
}

ECS::Entity* CirclePackingScene::createNewEntity()
{
	Entity* newEntity = new Entity();

	// attach component and return
	auto circlePackingData = new CirclePackingData(cocos2d::Vec2::ZERO, 0, cocos2d::Color4F::WHITE);
	newEntity->components[CIRCLE_PACKING_DATA] = circlePackingData;

	auto spriteComp = new ECS::Sprite(*this->circleNode, "circle_100.png");
	spriteComp->sprite->setScale(0.02f);	//radius 50.0f to 1.0f
	spriteComp->sprite->setVisible(false);
	newEntity->components[SPRITE] = spriteComp;

	return newEntity;
}

void CirclePackingScene::initQuadTree()
{
	if (this->currentImageIndex == IMAGE_INDEX::NONE) return;

	// Init quadtree with initial boundary
	auto bb = this->imageSprites.at(static_cast<int>(this->currentImageIndex))->getBoundingBox();
	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	bb.origin -= (winSize * 0.5f);
	this->quadTree = new QuadTree(bb, 0);
}

void CirclePackingScene::insertEntitiesToQuadTree()
{
	this->quadTree->clear();

	// Need to insert all active circles
	for (auto activeCircle : this->activeCircles)
	{
		this->quadTree->insert(activeCircle);
	}
}

void CirclePackingScene::releaseQuadTree()
{
	if (this->quadTree != nullptr)
	{
		delete this->quadTree;
		this->quadTree = nullptr;
	}
}

void CirclePackingScene::onButtonPressed(cocos2d::Ref * sender)
{
	cocos2d::ui::Button* button = dynamic_cast<cocos2d::ui::Button*>(sender);
	const BUTTON_TAG tag = static_cast<BUTTON_TAG>(button->getActionTag());

	switch (tag)
	{
	case BUTTON_TAG::CPP:
		this->runCirclePacking(IMAGE_INDEX::CPP);
		break;
	case BUTTON_TAG::CAT:
		this->runCirclePacking(IMAGE_INDEX::CAT);
		break;
	case BUTTON_TAG::THE_SCREAM:
		this->runCirclePacking(IMAGE_INDEX::THE_SCREAM);
		break;
	case BUTTON_TAG::GRADIENT:
		this->runCirclePacking(IMAGE_INDEX::GRADIENT);
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
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	//int mouseButton = mouseEvent->getMouseButton();
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void CirclePackingScene::onMouseUp(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	//int mouseButton = mouseEvent->getMouseButton();
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void CirclePackingScene::onMouseScroll(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//float x = mouseEvent->getScrollX();
	//float y = mouseEvent->getScrollY();
}

void CirclePackingScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE)
	{
		cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_I)
	{
		// toggle original image
		this->showOriginalImage = !this->showOriginalImage;
		if (this->currentImageIndex != IMAGE_INDEX::NONE)
		{
			int index = static_cast<int>(this->currentImageIndex);
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
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_SPACE)
	{
		this->pause = !this->pause;
		if (this->pause)
		{
			this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::PAUSE), cocos2d::Color3B::GREEN);
		}
		else
		{
			this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::PAUSE), cocos2d::Color3B::WHITE);
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_R)
	{
		// Restart current circle packing
		if (this->currentImageIndex != IMAGE_INDEX::NONE)
		{
			this->runCirclePacking(static_cast<IMAGE_INDEX>(this->currentImageIndex));
			this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::RESTART));
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_C)
	{
		if (this->currentImageIndex != IMAGE_INDEX::NONE)
		{
			// Stop and clear algorithm
			// Reset flag
			this->finished = false;
			// Delete quadTree
			releaseQuadTree();
			// Reset counter
			this->growingCircleCount = 0;
			// Reset existing circles
			this->resetCircles();
			// Reset image index
			this->currentImageIndex = IMAGE_INDEX::NONE;
			this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::CLEAR));

			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Waiting");
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::IMAGE_SIZE), "Image size (w x h): NA");
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::POSSIBLE_SPAWN_POINTS), "Possible spawn points: 0");
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::SPAWNED_CIRCLES), "Total circles: 0");
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::GROWING_CIRCLES), "Growing circles: 0");
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_S)
	{
		// Save to image
		if (this->currentImageIndex != IMAGE_INDEX::NONE)
		{
			this->labelsNode->playAnimation(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::SAVE));

			int w = this->images.at(static_cast<int>(this->currentImageIndex))->getWidth() + 50.0f;
			int h = this->images.at(static_cast<int>(this->currentImageIndex))->getHeight() + 50.0f;

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
}

void CirclePackingScene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{

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

	this->freshCircles.clear();
	this->activeCircles.clear();
}
