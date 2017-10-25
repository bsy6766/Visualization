#include "MainScene.h"
#include "QuadTreeScene.h"
#include "FlockingScene.h"
#include "CirclePackingScene.h"
#include "RectPackingScene.h"
#include "EarClippingScene.h"
#include "AStarScene.h"
#include "VisibilityScene.h"
#include "SortMenuScene.h"
#include "Utility.h"

USING_NS_CC;

MainScene* MainScene::createScene()
{
	MainScene* newMainScene = MainScene::create();
	return newMainScene;
}

bool MainScene::init()
{
	if (!cocos2d::Scene::init())
	{
		return false;
	}
	
	auto ss = cocos2d::SpriteFrameCache::getInstance();
	ss->addSpriteFramesWithFile("spritesheets/spritesheet.plist");

	hoveringLableIndex = -1;

	Utility::Random::init();

	std::string fontPath = "fonts/Rubik-Medium.ttf";

	auto titleLabel = cocos2d::Label::createWithTTF("Visualizations", fontPath, 50);

	cocos2d::Size winSize = cocos2d::Director::getInstance()->getVisibleSize();
	titleLabel->setPosition(cocos2d::Vec2(winSize.width * 0.5f, winSize.height - 50.0f));
	this->addChild(titleLabel);
	
    int fontSize = 30;
    
	this->labels.push_back(cocos2d::Label::createWithTTF("Quad Tree", fontPath, fontSize));
	this->labels.push_back(cocos2d::Label::createWithTTF("Flocking", fontPath, fontSize));
    this->labels.push_back(cocos2d::Label::createWithTTF("Circle Packing", fontPath, fontSize));
    this->labels.push_back(cocos2d::Label::createWithTTF("Rect Packing", fontPath, fontSize));
    this->labels.push_back(cocos2d::Label::createWithTTF("Ear Clipping", fontPath, fontSize));
    this->labels.push_back(cocos2d::Label::createWithTTF("A Star Pathfinding", fontPath, fontSize));
	this->labels.push_back(cocos2d::Label::createWithTTF("Visibility", fontPath, fontSize));
	this->labels.push_back(cocos2d::Label::createWithTTF("Sort", fontPath, fontSize));
	this->labels.push_back(cocos2d::Label::createWithTTF("EXIT(ESC)", fontPath, fontSize));

	this->versionLabel = cocos2d::Label::createWithTTF("v0.11", fontPath, 20);
	this->versionLabel->setPosition(winSize.width * 0.5f, 12.0f);
	this->addChild(this->versionLabel);

	this->descriptionLabel = cocos2d::Label::createWithTTF("", fontPath, 25);
	this->descriptionLabel->setPosition(winSize.width * 0.5f, 55.0f);
	this->addChild(this->descriptionLabel);
	
	int index = 0;
	cocos2d::Vec2 start = cocos2d::Vec2(winSize.width * 0.5f, winSize.height - 120.0f);
	for (auto label : this->labels)
	{
		cocos2d::Vec2 pos = start;
		pos.y -= (index * 40.0f);
		label->setPosition(pos);
		this->addChild(label);
		index++;
	}

	return true;
}

void MainScene::setDescriptionLabel()
{
	if (this->hoveringLableIndex != -1)
	{
		MENU_INDEX menuIndex = static_cast<MENU_INDEX>(this->hoveringLableIndex);
		switch (menuIndex)
		{
		case MainScene::MENU_INDEX::QUAD_TREE:
			this->descriptionLabel->setString("Visualizes collision check and resolution with Quad Tree");
			break;
		case MainScene::MENU_INDEX::FLOCKING:
			this->descriptionLabel->setString("Visualizes flocking algorithm");
			break;
		case MainScene::MENU_INDEX::CIRCLE_PACKING:
			this->descriptionLabel->setString("Visualizes circle packing based on image");
			break;
		case MainScene::MENU_INDEX::RECT_PACKING:
			this->descriptionLabel->setString("Visualizes rectangle packing square area");
			break;
        case MainScene::MENU_INDEX::EAR_CLIPPING:
            this->descriptionLabel->setString("Visualizes ear clipping (polygon triangulation) with drawing mode");
            break;
		case MainScene::MENU_INDEX::A_STAR_PATHFINDING:
			this->descriptionLabel->setString("Visualizes A* path finding");
			break;
		case MainScene::MENU_INDEX::VISIBILITY:
			this->descriptionLabel->setString("Visualizes visibility");
			break;
		case MainScene::MENU_INDEX::SORT:
			this->descriptionLabel->setString("Visualizes various sorting algorithms");
			break;
		case MainScene::MENU_INDEX::EXIT:
			this->descriptionLabel->setString("Exit");
			break;
		default:
			break;
		}
	}
	else
	{
		this->descriptionLabel->setString("");
	}
}

void MainScene::onEnter()
{
	cocos2d::Scene::onEnter();
	initInputListeners();
}

void MainScene::checkMouseOver(const cocos2d::Vec2 mousePos)
{
	int index = 0;
	for (auto label : this->labels)
	{
        auto labelBB = label->getBoundingBox();
        // Extend the bounding box on left and right
        float pad = 70.0f;
        labelBB.origin.x -= pad;
        labelBB.size.width += (pad*2.0f);
		if (labelBB.containsPoint(mousePos))
		{
			if (this->hoveringLableIndex == -1)
			{
				// wasn't hovering any menu.
				this->hoveringLableIndex = index;
				label->setScale(1.2f);
				label->setString("> " + label->getString() + " <");
				this->setDescriptionLabel();
				return;
			}
			else if(this->hoveringLableIndex == index)
			{
				// Hovering same. return
				return;
			}
			else
			{
				// Was hovering something, and wasn't the same
				this->labels.at(this->hoveringLableIndex)->setScale(1.0f);
				std::string labelStr = this->labels.at(this->hoveringLableIndex)->getString();
				this->labels.at(this->hoveringLableIndex)->setString(labelStr.substr(2, labelStr.size() - 4));

				label->setScale(1.2f);
				hoveringLableIndex = index;
				label->setString("> " + label->getString() + " <");
				this->setDescriptionLabel();
				return;
			}
		}
		else
		{
			if (label->getScale() > 1.0f)
			{
				label->setScale(1.0f);
			}
		}
		index++;
	}

	// If reach here, mouse wasn't hovering any.
	if (this->hoveringLableIndex != -1)
	{
		std::string labelStr = this->labels.at(this->hoveringLableIndex)->getString();
		this->labels.at(this->hoveringLableIndex)->setString(labelStr.substr(2, labelStr.size() - 4));
		this->hoveringLableIndex = -1;
		this->setDescriptionLabel();
	}
}

void MainScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(MainScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(MainScene::onMouseDown, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(MainScene::onKeyPressed, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void MainScene::onMouseMove(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	checkMouseOver(cocos2d::Vec2(x, y));
}

void MainScene::onMouseDown(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	int mouseButton = mouseEvent->getMouseButton();
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
	if (hoveringLableIndex == -1)
	{
		return;
	}
	else
	{
		if (mouseButton == 0)
		{
			MENU_INDEX menuIndex = static_cast<MENU_INDEX>(this->hoveringLableIndex);
			switch (menuIndex)
			{
			case MENU_INDEX::QUAD_TREE:
			{
				cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, QuadTreeScene::create(), cocos2d::Color3B::BLACK));
			}
				break;
			case MENU_INDEX::FLOCKING:
			{
				cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, FlockingScene::create(), cocos2d::Color3B::BLACK));
			}
				break;
			case MENU_INDEX::CIRCLE_PACKING:
			{
				cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, CirclePackingScene::create(), cocos2d::Color3B::BLACK));
			}
				break;
			case MENU_INDEX::RECT_PACKING:
			{
				cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, RectPackingScene::create(), cocos2d::Color3B::BLACK));
			}
                    break;
            case MENU_INDEX::EAR_CLIPPING:
            {
                cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, EarClippingScene::create(), cocos2d::Color3B::BLACK));
            }
                break;
			case MENU_INDEX::A_STAR_PATHFINDING:
			{
				cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, AStarScene::create(), cocos2d::Color3B::BLACK));
			}
				break;
			case MENU_INDEX::VISIBILITY:
			{
				cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, VisibilityScene::create(), cocos2d::Color3B::BLACK));
			}
				break;
			case MENU_INDEX::SORT:
			{
				cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, SortMenuScene::create(), cocos2d::Color3B::BLACK));
			}
				break;
			case MENU_INDEX::EXIT:
			{
				cocos2d::Director::getInstance()->end();
			}
				break;
			default:
				break;
			}
		}
	}
}

void MainScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE)
	{
		cocos2d::Director::getInstance()->end();
	}
}

void MainScene::releaseInputListeners()
{
	if(this->mouseInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->mouseInputListener);
	if(this->keyInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->keyInputListener);
}

void MainScene::onExit()
{
	cocos2d::Scene::onExit();
	releaseInputListeners();
}
