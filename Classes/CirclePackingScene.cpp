#include "CirclePackingScene.h"
#include "MainScene.h"
#include "Circle.h"
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
	this->searchSpawnPointWidthOffset = 2;
	this->searchSpawnPointHeightOffset = 2;

	// Init images
	initImages();

	// Set max circles to draw
	this->maxCircles = 500;

	// Initializes deactivated circles
	initCircles();

	// Set number of circles to spawn at start and every tick
	this->initialCircleCount = 10;
	this->circleSpawnRate = 1;

	//temp
	this->spawnedCircleCount = 0;
	findCircleSpawnPoint(IMAGE_INDEX::DEAULT);
	spawnCircles(this->initialCircleCount);
	this->currentImageIndex = IMAGE_INDEX::DEAULT;

	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	std::string fontPath = "fonts/Rubik-Medium.ttf";

	this->backLabel = cocos2d::Label::createWithTTF("BACK(ESC)", fontPath, 20);
	this->backLabel->setPosition(cocos2d::Vec2(winSize.width - 60.0f, 20.0f));
	this->addChild(this->backLabel);

	cocos2d::DrawNode* drawNode = cocos2d::DrawNode::create();
	this->addChild(drawNode);

	this->pause = true;

	this->simulateSpeedMultiplier = 1.5f;

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
		case IMAGE_INDEX::DEAULT:
		{
			this->images.back()->initWithImageFile("Images/C++.png");

			cocos2d::Texture2D* tex = new cocos2d::Texture2D();
			tex->initWithImage(this->images.back());
			tex->autorelease();

			cocos2d::Sprite* sprite = cocos2d::Sprite::createWithTexture(tex);
			sprite->retain();
			sprite->setAnchorPoint(cocos2d::Vec2(0, 0));
			auto sizeHalf = sprite->getContentSize() * 0.5f;
			auto winSizeHalf = winSize * 0.5f;
			sprite->setPosition(cocos2d::Vec2(winSizeHalf - sizeHalf));
			sprite->setVisible(false);

			this->addChild(sprite, SPRITE_Z_ORDER::BEHIND_CIRCLES);
			this->imageSprites.push_back(sprite);
		}
			break;
		case IMAGE_INDEX::CAT:
			break;
		case IMAGE_INDEX::MAX_SIZE:
			break;
		default:
			break;
		}
	}
}

void CirclePackingScene::findCircleSpawnPoint(const IMAGE_INDEX imageIndex)
{
	if (this->circleSpawnPoints.empty() == false)
	{
		std::queue<cocos2d::Vec2> empty;
		std::swap(this->circleSpawnPoints, empty);
	}

	unsigned int index = static_cast<unsigned int>(imageIndex);

	int x = 3;
	if (this->images.at(index)->hasAlpha())
	{
		x = 4;
	}

	unsigned char *data = new unsigned char[this->images.at(index)->getDataLen()*x];
	data = this->images.at(index)->getData();
	// [0][0] => Left-Top Pixel !
	// But cocos2d Location Y-axis is Bottom(0) to Top(max)

	int width = this->images.at(index)->getWidth();
	int height = this->images.at(index)->getHeight();

	std::vector<cocos2d::Vec2> points;

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
			case IMAGE_INDEX::DEAULT:
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
						auto point = CC_POINT_PIXELS_TO_POINTS(cocos2d::Vec2(i, j));
						point += this->imageSprites.at(index)->getPosition();
						points.push_back(point);
					}
					else
					{
						continue;
					}
				}
			}
				break;
			default:
				break;
			}
		}
	}

	delete[] data;

	if (!points.empty())
	{
		std::random_shuffle(std::begin(points), std::end(points));
		int size = static_cast<int>(points.size());

		for (int i = 0; i < size; i++)
		{
			this->circleSpawnPoints.push(points.at(i));
		}
	}
}

void CirclePackingScene::initCircles()
{
	for (int i = 0; i < this->maxCircles; i++)
	{
		this->circles.push_back(new Circle(cocos2d::Vec2::ZERO, 0));
	}
}

void CirclePackingScene::spawnCircles(const int rate)
{
	int count = 0;
	if (!this->circleSpawnPoints.empty() && this->spawnedCircleCount < this->maxCircles)
	{
		while (count < rate && this->circleSpawnPoints.size() > 0)
		{
			auto point = this->circleSpawnPoints.front();

			bool inCircle = false;

			for (int i = 0; i < this->spawnedCircleCount; i++)
			{
				float distance = this->circles.at(i)->position.distance(point);
				if (distance <= this->circles.at(i)->radius)
				{
					// point is in the circle. skip
					inCircle = true;
					break;
				}
			}

			if (inCircle)
			{
				this->circleSpawnPoints.pop();
				continue;
			}

			cocos2d::Color4F color = cocos2d::Color4F::WHITE;

			if (this->currentImageIndex == IMAGE_INDEX::DEAULT)
			{
				color = cocos2d::Color4F(cocos2d::RandomHelper::random_real<float>(0, 1.0f),
					cocos2d::RandomHelper::random_real<float>(0, 1.0f),
					cocos2d::RandomHelper::random_real<float>(0, 1.0f),
					1.0f);
			}

			this->circles.at(this->spawnedCircleCount)->activate(point, 1.0f, color);
			this->spawnedCircleCount++;

			this->circleSpawnPoints.pop();
			count++;
		}
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

	delta *= this->simulateSpeedMultiplier;

	// Spawn circles
	spawnCircles(this->circleSpawnRate);

	// Update growth
	for (int i = 0; i < this->spawnedCircleCount; i++)
	{
		if (this->circles.at(i)->alive)
		{
			this->circles.at(i)->update(delta);
		}
	}

	// Update collision
	for (int i = 0; i < this->spawnedCircleCount; i++)
	{
		for (int j = 0; j < this->spawnedCircleCount; j++)
		{
			if (i != j)
			{
				if (this->circles.at(i)->growing || this->circles.at(j)->growing)
				{
					//either one of circle must be growing. If both are not growing, don't need to check
					float distance = this->circles.at(i)->position.distance(this->circles.at(j)->position);
					float minDistance = this->circles.at(i)->radius + this->circles.at(j)->radius;
					if (distance <= minDistance)
					{
						// These two circle touched each other. Stop growing
						this->circles.at(i)->growing = false;
						this->circles.at(j)->growing = false;
					}
				}
			}
		}
	}

	// Clear all buffer
	this->drawNode->clear();

	// Draw dot
	for (int i = 0; i < this->spawnedCircleCount; i++)
	{
		this->drawNode->drawDot(this->circles.at(i)->position, this->circles.at(i)->radius, this->circles.at(i)->color);
		if (this->circles.at(i)->growing)
		{
			//this->drawNode->drawCircle(this->circles.at(i)->position, this->circles.at(i)->radius, 360.0f, 50, false, cocos2d::Color4F::WHITE);
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
		this->imageSprites.front()->setVisible(!this->imageSprites.front()->isVisible());
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_SPACE)
	{
		// Terminate 
		this->pause = !this->pause;
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

	for (auto circle : this->circles)
	{
		if (circle != nullptr)
		{
			delete circle;
		}
	}

	this->circles.clear();
}