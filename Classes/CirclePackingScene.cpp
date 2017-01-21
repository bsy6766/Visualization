#include "CirclePackingScene.h"
#include "MainScene.h"
#include <algorithm>	// std::random_shuffle
#include <utility>		// std::swap
#include <random>
#include "Component.h"
#include "Utility.h"	// custom random

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

	// Uncomment this to activate update(float) function
	this->scheduleUpdate();

	// Initialize drawNode
	this->growingDrawNode = cocos2d::DrawNode::create();
	this->addChild(this->growingDrawNode, static_cast<int>(SPRITE_Z_ORDER::CIRCLES));
	this->allGrownCircleDrawNode = cocos2d::DrawNode::create();
	this->addChild(this->allGrownCircleDrawNode, static_cast<int>(SPRITE_Z_ORDER::CIRCLES));

	// Set spawn point serach offsets
	this->searchSpawnPointWidthOffset = 4;
	this->searchSpawnPointHeightOffset = 4;

	// Init current image index
	this->currentImageIndex = IMAGE_INDEX::NONE;

	// init max Circles to 0. Will be initialized later
	this->maxCircles = 0;

	// Set number of circles to spawn at start and every tick
	this->initialCircleCount = 15;
	this->circleSpawnRate = 2;

	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	this->viewingImageSelectPanel = false;
	this->pause = false;

	std::string fontPath = "fonts/Rubik-Medium.ttf";

	this->backLabel = cocos2d::Label::createWithTTF("BACK(ESC)", fontPath, 20);
	this->backLabel->setPosition(cocos2d::Vec2(winSize.width - 60.0f, 20.0f));
	this->addChild(this->backLabel);

	this->imageNameLabel = cocos2d::Label::createWithTTF("", fontPath, 30);
	this->imageNameLabel->setPosition(cocos2d::Vec2(winSize.width * 0.5f, winSize.height - 20.0f));
	this->addChild(this->imageNameLabel);

	this->imageTestPurposeLabel = cocos2d::Label::createWithTTF("", fontPath, 20);
	this->imageTestPurposeLabel->setPosition(cocos2d::Vec2(winSize.width * 0.5f, winSize.height - 40.0f));
	this->addChild(this->imageTestPurposeLabel);

	this->leftArrow = cocos2d::Sprite::createWithSpriteFrameName("arrowLeft.png");
	this->leftArrow->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->leftArrow->setPosition(cocos2d::Vec2(10.0f, winSize.height * 0.5f));
	this->addChild(this->leftArrow);

	this->imageSelectInstructionLabel = cocos2d::Label::createWithTTF("Move your mouse to left edge\nto select images.", fontPath, 20);
	this->imageSelectInstructionLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	auto arrowSize = this->leftArrow->getContentSize();
	this->imageSelectInstructionLabel->setPosition(cocos2d::Vec2(15.0f + arrowSize.width, winSize.height * 0.5f));
	this->addChild(this->imageSelectInstructionLabel);

	this->imageSelectNode = cocos2d::Node::create();
	this->imageSelectNode->setPosition(cocos2d::Vec2(0, winSize.height * 0.5f));
	this->imageSelectNode->setVisible(false);
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

	this->simulateSpeedMultiplier = 1.5f;
	this->growingCircleCount = 0;

	fps = 0;
	fpsElapsedTime = 0;

	this->fpsLabel = cocos2d::Label::createWithTTF("FPS: " + std::to_string(cocos2d::Director::getInstance()->getFrameRate()), fontPath, 25);
	this->fpsLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->fpsLabel->setPosition(cocos2d::Vec2(5.0f, 20.0f));
	this->addChild(this->fpsLabel);

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
	if (this->pause)
	{
		return;
	}

	updateFPS(delta);

	// check current image
	if (this->currentImageIndex == IMAGE_INDEX::NONE) return;

	// Modify time by multiplier
	delta *= this->simulateSpeedMultiplier;

	// Reset QuadTree
	insertEntitiesToQuadTree();

	// Spawn circles
	spawnCircles(this->circleSpawnRate);

	// Update growth
	updateCircleRadius(delta);

	// Reset QuadTree
	insertEntitiesToQuadTree();

	// Update growth
	updateCircleGrowthWithCollision();

	// Move all grown circles to another list
	bool clearAllGrownDrawNode = this->moveAllGrownCircles();

	// Draw dot
	//updateDrawNodes(clearAllGrownDrawNode);
}

void CirclePackingScene::updateFPS(const float delta)
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

void CirclePackingScene::updateCircleRadius(const float delta)
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
		else
		{
			break;
		}
	}
}

void CirclePackingScene::updateCircleGrowthWithCollision()
{
	if (this->growingCircleCount <= 0)
	{
		// No circles are growing at this moment
		return;
	}

	auto size = static_cast<int>(this->activeCircles.size());

	for (auto activeCircle : this->activeCircles)
	{
		// Create query box
		auto leftSpriteComp = activeCircle->getComponent<ECS::Sprite*>(SPRITE);
		cocos2d::Rect queryBox = leftSpriteComp->sprite->getBoundingBox();
		float pad = 20.0f;
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

void CirclePackingScene::updateDrawNodes(const bool clearAllGrownDrawNode)
{
	// Clear all buffer
	this->growingDrawNode->clear();

	if (clearAllGrownDrawNode)
	{
		this->allGrownCircleDrawNode->clear();
	}


	for (auto& activeCircle : this->activeCircles)
	{
		auto dataComp = activeCircle->getComponent<CirclePackingData*>(CIRCLE_PACKING_DATA);

		if (dataComp->growing)
		{
			this->growingDrawNode->drawDot(dataComp->position, dataComp->radius, dataComp->color);
		}
		else
		{
			this->allGrownCircleDrawNode->drawDot(dataComp->position, dataComp->radius, dataComp->color);
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
	this->addChild(sprite, static_cast<int>(SPRITE_Z_ORDER::BEHIND_CIRCLES));
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
						//auto point = cocos2d::Vec2(i, j);
						auto point = this->pixelToPoint(i, j, height, this->imageSprites.at(index)->getPosition());
						//auto point = CC_POINT_PIXELS_TO_POINTS(cocos2d::Vec2(i, j));

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
			//this->freshCircles.push_back(std::unique_ptr<Circle>(new Circle(cocos2d::Vec2::ZERO, 0)));
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

void CirclePackingScene::spawnCircles(const int spawnRate)
{
	int count = 0;
	// Both spawn point and fresh circles must not be empty
	if (!this->circleSpawnPointsWithColor.empty() && !this->freshCircles.empty())
	{
		// Run until spawn point exists and fill spawn rate
		while (count < spawnRate && this->circleSpawnPointsWithColor.size() > 0)
		{
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
				float pad = 2.0f;
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
	// Delete quadTree
	releaseQuadTree();
	// Reset counter
	this->growingCircleCount = 0;
	// Reset existing circles
	this->resetCircles();
	// clear draw buffer
	this->growingDrawNode->clear();
	this->allGrownCircleDrawNode->clear();
	// update image index
	this->currentImageIndex = imageIndex;
	// reset spawn point
	this->findCircleSpawnPoint(imageIndex);
	// set max circles size
	this->maxCircles = this->circleSpawnPointsWithColor.size();
	// initialize circles
	initCircles();
	// update image name label
	this->setImageNameLabel();
	// Re-initialize QuadTree
	initQuadTree();
}

void CirclePackingScene::setImageNameLabel()
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
		break;
	}
}

ECS::Entity* CirclePackingScene::createNewEntity()
{
	Entity* newEntity = new Entity();

	// attach component and return
	auto circlePackingData = new CirclePackingData(cocos2d::Vec2::ZERO, 0, cocos2d::Color4F::WHITE);
	newEntity->components[CIRCLE_PACKING_DATA] = circlePackingData;

	auto spriteComp = new ECS::Sprite(*this, "circle_100.png");
	spriteComp->sprite->setScale(0.02f);	//radius 50.0f to 1.0f
	spriteComp->sprite->setVisible(false);
	newEntity->components[SPRITE] = spriteComp;

	return newEntity;
}

void CirclePackingScene::initQuadTree()
{
	if (this->currentImageIndex == IMAGE_INDEX::NONE) return;

	// Init quadtree with initial boundary
	this->quadTree = new QTree(this->imageSprites.at(static_cast<int>(this->currentImageIndex))->getBoundingBox(), 0);
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
	cocos2d::Vec2 point = cocos2d::Vec2(mouseEvent->getCursorX(), mouseEvent->getCursorY());

	if (0 < point.x && point.x < 30.0f)
	{
		// In detection range
		if (!this->viewingImageSelectPanel)
		{
			// Not viewing image select panel
			this->viewingImageSelectPanel = true;
			this->imageSelectNode->setVisible(true);
		}
		//else, already viewing. do nothing

		// hide insturction if visible
		if (this->imageSelectInstructionLabel->isVisible())
		{
			this->imageSelectInstructionLabel->setVisible(false);
		}

		if (this->leftArrow->isVisible())
		{
			this->leftArrow->setVisible(false);
		}
	}
	else if (point.x > 120.0f)
	{
		// Out of panel range
		if (this->viewingImageSelectPanel)
		{
			// viewing panel. hide
			this->viewingImageSelectPanel = false;
			this->imageSelectNode->setVisible(false);
		}
	}
	else if(point.x < 0 || point.y < 0 || point.y > cocos2d::Director::getInstance()->getVisibleSize().height)
	{
		// out of window. exit
		this->viewingImageSelectPanel = false;
		this->imageSelectNode->setVisible(false);
	}
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

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_S)
	{
		// toggle sprite
		this->imageSprites.at(static_cast<int>(this->currentImageIndex))->setVisible(!this->imageSprites.at(static_cast<int>(this->currentImageIndex))->isVisible());
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_SPACE)
	{
		// Terminate 
		this->pause = !this->pause;
	}


	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_1)
	{
		// C++
		this->runCirclePacking(IMAGE_INDEX::CPP);
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_2)
	{
		// Cat
		this->runCirclePacking(IMAGE_INDEX::CAT);
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_3)
	{
		// The Scream
		this->runCirclePacking(IMAGE_INDEX::THE_SCREAM);
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_4)
	{
		// The Scream
		this->runCirclePacking(IMAGE_INDEX::GRADIENT);
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_R)
	{
		// reset
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_H)
	{
		// Toggle circles hidden
		this->growingDrawNode->setVisible(!this->growingDrawNode->isVisible());
		this->allGrownCircleDrawNode->setVisible(!this->allGrownCircleDrawNode->isVisible());
	}
}

void CirclePackingScene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{

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