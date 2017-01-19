#include "CirclePackingScene.h"
#include "MainScene.h"
#include <algorithm>	// std::random_shuffle
#include <utility>		// std::swap
#include <random>

USING_NS_CC;

CirclePackingScene* CirclePackingScene::createScene()
{
	CirclePackingScene* newCirclePackingScene = CirclePackingScene::create();
	return newCirclePackingScene;
}

bool CirclePackingScene::init()
{
	if (!CCScene::init())
	{
		return false;
	}

	// Uncomment this to activate update(float) function
	this->scheduleUpdate();

	// Initialize drawNode
	this->drawNode = cocos2d::DrawNode::create();
	this->addChild(this->drawNode, SPRITE_Z_ORDER::CIRCLES);

	// Set spawn point serach offsets
	this->searchSpawnPointWidthOffset = 4;
	this->searchSpawnPointHeightOffset = 4;

	this->currentImageIndex = IMAGE_INDEX::NONE;

	// Init images
	initImages();

	// init max Circles to 0. Will be initialized later
	this->maxCircles = 0;

	// Set number of circles to spawn at start and every tick
	this->initialCircleCount = 10;
	this->circleSpawnRate = 5;

	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	std::string fontPath = "fonts/Rubik-Medium.ttf";

	this->backLabel = cocos2d::Label::createWithTTF("BACK(ESC)", fontPath, 20);
	this->backLabel->setPosition(cocos2d::Vec2(winSize.width - 60.0f, 20.0f));
	this->addChild(this->backLabel);

	cocos2d::DrawNode* drawNode = cocos2d::DrawNode::create();
	this->addChild(drawNode);

	this->pause = true;

	this->simulateSpeedMultiplier = 1.5f;

	fps = 0;
	fpsElapsedTime = 0;

	this->fpsLabel = cocos2d::Label::createWithTTF("FPS: " + std::to_string(cocos2d::Director::getInstance()->getFrameRate()), fontPath, 25);
	this->fpsLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->fpsLabel->setPosition(cocos2d::Vec2(5.0f, 20.0f));
	this->addChild(this->fpsLabel);

	return true;
}

void CirclePackingScene::initImages()
{
	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();
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
			initImageAndSprite("Images/C++.png");
		}
			break;
		case IMAGE_INDEX::CAT:
		{
			initImageAndSprite("Images/cat.png");
		}
			break;
		case IMAGE_INDEX::THE_SCREAM:
		{
			initImageAndSprite("Images/TheScream.png");
		}
		break;
		default:
			break;
		}
	}
}

void CirclePackingScene::initImageAndSprite(const std::string & imageName)
{
	this->images.back()->initWithImageFile(imageName);

	cocos2d::Texture2D* tex = new cocos2d::Texture2D();
	tex->initWithImage(this->images.back());
	tex->autorelease();

	cocos2d::Sprite* sprite = cocos2d::Sprite::createWithTexture(tex);
	sprite->retain();
	sprite->setAnchorPoint(cocos2d::Vec2(0, 0));
	auto sizeHalf = sprite->getContentSize() * 0.5f;
	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();
	auto winSizeHalf = winSize * 0.5f;
	sprite->setPosition(cocos2d::Vec2(winSizeHalf - sizeHalf));
	sprite->setVisible(false);

	this->addChild(sprite, SPRITE_Z_ORDER::BEHIND_CIRCLES);
	this->imageSprites.push_back(sprite);
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
					// For cat, all points that are visible will be spawn point
					auto point = this->pixelToPoint(i, j, height, this->imageSprites.at(index)->getPosition());

					points.push_back(SpawnPoint{ point, cocos2d::Color4F(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f) });
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
		
		std::random_shuffle(std::begin(points), std::end(points));
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
		this->freshCircles.resize(this->maxCircles);
		return;
	}
	else
	{
		// Fill fresh circles by max size. If there are fresh circles left, resue them
		for (int i = this->freshCircles.size(); i < this->maxCircles; i++)
		{
			this->freshCircles.push_back(std::unique_ptr<Circle>(new Circle(cocos2d::Vec2::ZERO, 0)));
		}
	}
}

void CirclePackingScene::moveAllGrownCircles()
{
	auto it = this->activeCircles.begin();
	for (;it != this->activeCircles.end();)
	{
		// Circles that are growing are placed at front of list
		// Circles that are all grown are placed at the back of list.
		// So iterate until it finds alive and all grown circle
		if ((*it)->growing == false)
		{
			// Move all grown circle back to list
			this->activeCircles.splice(this->activeCircles.end(), this->activeCircles, it);
		}

		it++;
	}
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

			for (auto& activeCircle : this->activeCircles)
			{
				float distance = activeCircle->position.distance(spawnPoint.point);
				if (distance <= activeCircle->radius)
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
			(*it_front)->activate(spawnPoint.point, 1.0f, spawnPoint.color);
			// Move to activeCircles
			this->activeCircles.splice(this->activeCircles.begin(), this->freshCircles, it_front);
			// pop spawn point
			this->circleSpawnPointsWithColor.pop();
			// inc count
			count++;
		}
	}
}

void CirclePackingScene::resetCircles()
{
	for (auto& circle : this->activeCircles)
	{
		circle->deactivate();
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
	// Reset existing circles
	this->resetCircles();
	// clear draw buffer
	this->drawNode->clear();
	// update image index
	this->currentImageIndex = imageIndex;
	// reset spawn point
	this->findCircleSpawnPoint(imageIndex);
	// set max circles size
	this->maxCircles = this->circleSpawnPointsWithColor.size();
	// initialize circles
	initCircles();
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

void CirclePackingScene::onEnter()
{
	cocos2d::CCScene::onEnter();

	initInputListeners();

	this->pause = false;
}

void CirclePackingScene::update(float delta)
{
	if (this->pause)
	{
		return;
	}

	updateFPS(delta);

	// Modify time by multiplier
	delta *= this->simulateSpeedMultiplier;

	// check current image
	if (this->currentImageIndex == IMAGE_INDEX::NONE) return;

	// Spawn circles
	spawnCircles(this->circleSpawnRate);

	// Update growth
	for (auto& circle : this->activeCircles)
	{
		if (circle->growing)
		{
			circle->update(delta);
		}
	}

	// Update collision
	int growingCircleSize = 0;
	for (auto& activeCircle : this->activeCircles)
	{
		if (activeCircle->growing)
		{
			growingCircleSize++;
		}
		else
		{
			break;
		}
	}

	auto& left_it = this->activeCircles.begin();

	auto size = static_cast<int>(this->activeCircles.size());

	for (int i = 0; i < size; i++)
	{
		auto& right_it = this->activeCircles.begin();
		for (int j = 0; j < size; j++)
		{
			if (i != j)
			{
				if ((*left_it)->growing || (*right_it)->growing)
				{
					//either one of circle must be growing. If both are not growing, don't need to check
					float distance = (*left_it)->position.distance((*right_it)->position);
					float minDistance = (*left_it)->radius + (*right_it)->radius;
					if (distance <= minDistance)
					{
						// These two circle touched each other. Stop growing
						(*left_it)->growing = false;
						(*right_it)->growing = false;
					}
				}
			}

			right_it++;
		}

		left_it++;
	}

	// Move all grown circles to another list
	this->moveAllGrownCircles();

	// Clear all buffer
	this->drawNode->clear();

	// Draw dot
	for (auto& activeCircle : this->activeCircles)
	{
		this->drawNode->drawDot(activeCircle->position, activeCircle->radius, activeCircle->color);
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

void CirclePackingScene::onMouseMove(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
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
	cocos2d::CCScene::onExit();
	// Uncomment this if you are using initInputListeners()
	releaseInputListeners(); 

	this->images.clear();

	this->imageSprites.clear();

	this->freshCircles.clear();
	this->activeCircles.clear();
}