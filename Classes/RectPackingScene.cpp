#include "RectPackingScene.h"
#include "MainScene.h"
#include "Utility.h"
#include "Component.h"
#include <algorithm>
#include <random>

USING_NS_CC;

RectPackingScene* RectPackingScene::createScene()
{
	RectPackingScene* newRectPackingScene = RectPackingScene::create();
	return newRectPackingScene;
}

bool RectPackingScene::init()
{
	if (!CCScene::init())
	{
		return false;
	}

	this->scheduleUpdate();

	this->padding = 0.0f;
	this->drawDivisionLine = false;
	this->pause = false;
	this->finished = true;
	this->stepDuration = 0.01f;
	this->elapsedTime = 0;

	this->root = nullptr;

	// Init labels node
	this->labelsNode = LabelsNode::createNode();
	this->labelsNode->setSharedLabelPosition(LabelsNode::SHARED_LABEL_POS_TYPE::RECT_PACKING_SCENE);
	this->addChild(this->labelsNode);

	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();
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

	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Status: Waiting", customLabelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Total packed rectangles = 0", customLabelSize);

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
	this->buttonModifierNode->addButton("Border padding",
										labelSize,
										this->padding,
										leftStr,
										rightStr,
										buttonStr,
										format,
										static_cast<int>(ACTION_TAG::LEFT),
										static_cast<int>(ACTION_TAG::RIGHT),
										CC_CALLBACK_1(RectPackingScene::onButtonPressed, this));

	// init more labels    
	const int headerSize = 25;

	const float weightLastY = this->buttonModifierNode->buttonLabels.back()->getBoundingBox().getMinY();
	this->labelsNode->keyboardUsageLabelStartPos = cocos2d::Vec2(labelX, weightLastY - blockGap);

	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Keys (Green = enabled)", headerSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Space: Toggle update", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "L: Toggle rect division line", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "R: Restart", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "C: Clear", labelSize);

	// speed modifier
	this->simulationSpeedModifier = 1.0f;

	this->sliderLabelNode = SliderLabelNode::createNode();
	const float mouseLastY = this->labelsNode->keyboardUsageLabels.back()->getBoundingBox().getMinY();
	this->sliderLabelNode->sliderStartPos = cocos2d::Vec2(labelX, mouseLastY - blockGap);
	this->sliderLabelNode->addSlider("Simulation Speed", "Slider", 50, CC_CALLBACK_1(RectPackingScene::onSliderClick, this));
	this->addChild(this->sliderLabelNode);

	// init display boundary box node which draws outer line of simulation display box
	this->displayBoundaryBoxNode = DisplayBoundaryBoxNode::createNode();
	this->displayBoundaryBoxNode->setPosition(cocos2d::Vec2::ZERO);
	this->displayBoundaryBoxNode->displayBoundary = cocos2d::Rect(0, 0, 650, 650);
	this->displayBoundaryBoxNode->drawDisplayBoundaryBox();
	this->displayBoundaryBoxNode->retain();
	this->displayBoundaryShift = this->displayBoundaryBoxNode->displayBoundary.origin;
	this->displayBoundaryBoxNode->drawNode->setLocalZOrder(static_cast<int>(Z_ORDER::BOX));
	this->addChild(this->displayBoundaryBoxNode);

	// Init root entity
	ECS::Entity::idCounter = 0;
	// Can insert 3000 rect
	ECS::Entity::maxEntitySize = 3000;

	this->rectDrawNode = cocos2d::DrawNode::create();
	this->rectDrawNode->setLineWidth(1.0f);
	this->addChild(this->rectDrawNode);

	return true;
}

void RectPackingScene::onEnter()
{
	cocos2d::CCScene::onEnter();
	initInputListeners();

	restart();
}

void RectPackingScene::update(float delta)
{
	this->labelsNode->updateFPSLabel(delta);

	if (pause) return;
	if (finished) return;

	Utility::Time::start();

	delta *= this->simulationSpeedModifier;

	this->elapsedTime += delta;

	while (this->elapsedTime >= this->stepDuration)
	{
		this->elapsedTime -= this->stepDuration;

		if (this->randomSizes.empty())
		{
			this->finished = true;
			cocos2d::log("%d Rects packed!", this->totalRectsPacked);

			this->buttonModifierNode->leftButtons.back()->setEnabled(true);
			this->buttonModifierNode->rightButtons.back()->setEnabled(true);

			this->labelsNode->updateTimeTakenLabel("0");

			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Finished");
			break;
		}
		else
		{
			auto& size = this->randomSizes.front();
			this->randomSizes.pop();
			bool success = this->insert(this->root, size);
			if (success)
			{
				this->totalRectsPacked++;
			}
		}
	}

	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::TOTAL_RECT_PACKED), "Total packed rectangles: " + std::to_string(this->totalRectsPacked));

	this->rectDrawNode->clear();
	this->drawRects(this->root);

	Utility::Time::stop();

	std::string timeTakenStr = Utility::Time::getElaspedTime();	// Microseconds
	float timeTakenF = std::stof(timeTakenStr);	// to float
	timeTakenF *= 0.001f; // To milliseconds
	this->labelsNode->updateTimeTakenLabel(std::to_string(timeTakenF).substr(0, 5));
}

const bool RectPackingScene::insert(ECS::Entity * entity, const cocos2d::Size& rectSize)
{
	if (rectSize.width == 0 || rectSize.height == 0)
	{
		// invalid size of rectangle
		return false;
	}

	// Get component
	auto rectComp = entity->getComponent<ECS::RectPackingNode*>(ECS::COMPONENT_ID::RECT_PACKING_NODE);
	
	if (rectComp->isLeaf() == false)
	{
		if (rectComp->left == nullptr || rectComp->right == nullptr)
		{
			// Both left and right can't be nullptr
			return false;
		}

		// Insert on left
		const bool leftResult = insert(rectComp->left, rectSize);

		if (leftResult == false)
		{
			// left failed. insert on right
			return insert(rectComp->right, rectSize);
		}
	}
	else
	{
		// This entity node is leaf

		// Check if this node already has rectangle assigned
		if (rectComp->rect.equals(cocos2d::Rect::ZERO) == false)
		{
			// This node has rect already. Fail to insert rect.
			return false;
		}
		// else, it's empty

		// Check if the new rectangle fits to this node
		const cocos2d::Vec2 origin = rectComp->area.origin;
		cocos2d::Rect targetRect = cocos2d::Rect(origin, rectSize);

		if (Utility::containsRect(rectComp->area, targetRect) == false)
		{
			// Rectangle is too big. Failed to insert rect
			return false;
		}
		// else, can fit

		// Check if this image perfectly fits to area
		if (rectComp->area.size.equals(rectSize))
		{
			rectComp->rect = targetRect;
			//cocos2d::log("Inserting!!");
			return true;
		}
		// Else, doesn't fit perfectly

		// Then, split the area.
		rectComp->left = this->createNewEntity();
		rectComp->right = this->createNewEntity();

		// Get left and right entities compoennt
		auto leftRectComp = rectComp->left->getComponent<ECS::RectPackingNode*>(ECS::COMPONENT_ID::RECT_PACKING_NODE);
		auto rightRectComp = rectComp->right->getComponent<ECS::RectPackingNode*>(ECS::COMPONENT_ID::RECT_PACKING_NODE);

		// double the padding because we are adding pad on all 4 sides of rect
		float pad = this->padding * 2.0f;

		// Check the size of new rectangle and see which way do split
		const cocos2d::Size dSize = rectComp->area.size - rectSize;


		if(dSize.width > dSize.height)
		{
			// The rectangle's width is larger or equal than height, let's call this wide shape (I know this handles square but just being simple)
			// In this case, we split(cut) child area vertically 
			/*
								  area rectagnle					  area rectangle
								*----------------*					*-----*----------*
								|     | dh       |					|     |          |
								|     |     dw   |					|     |          |
								*-----*----------|		Split		|     |          |
								|XXXXX|          |		---->		|     |          |
			new rectangle	->	|XXXXX|          |					|     |          |
			(filled with X)		|XXXXX|          |					|     |          |
								*-----*----------*					*-----*----------*
			*/

			// Left ofigin equals to area's origin
			cocos2d::Vec2 leftOrigin = origin;
			// Left size. Width equal to new rectangle's widht and height is same as area.
			cocos2d::Size leftSize = cocos2d::Size(rectSize.width, rectComp->area.size.height);
			// Set area
			leftRectComp->area = cocos2d::Rect(leftOrigin, leftSize);

			// Right origin. X starts from area's origin with left size's width, which includes the pad
			float rightX = origin.x + leftSize.width;
			cocos2d::Vec2 rightOrigin = cocos2d::Vec2(rightX, origin.y);
			// Right width is difference between area width and leftSize with.
			cocos2d::Size rightSize = cocos2d::Size(rectComp->area.size.width - leftSize.width, rectComp->area.size.height);
			rightRectComp->area = cocos2d::Rect(rightOrigin, rightSize);
		}
		else
		{
			// The rectangle's height is larger than width, let's call this long shape rectangle.
			// In this case, we split(cut) child area horizontally 

			/*
								  area rectagnle					  area rectangle
								*----------------*					*----------------*
								|         |      |					|                |
								|      dh |      |					|                |
								|         |      |		Split		|                |
								*---------*------|		---->		*----------------*
			new rectangle	->	|XXXXXXXXX|  dw  |					|                |
			(filled with X)		|XXXXXXXXX|      |					|                |
								*---------*------*					*----------------*
			*/

			// Left ofigin equals to area's origin
			cocos2d::Vec2 leftOrigin = origin;
			cocos2d::Size leftSize = cocos2d::Size(rectComp->area.size.width, rectSize.height);
			leftRectComp->area = cocos2d::Rect(leftOrigin, leftSize);

			float rightY = rectSize.height + origin.y;
			const cocos2d::Vec2 rightOrigin = cocos2d::Vec2(origin.x, rightY);
			float rightHeight = rectComp->area.size.height - rectSize.height;
			cocos2d::Size rightSize = cocos2d::Size(rectComp->area.size.width, rightHeight);
			rightRectComp->area = cocos2d::Rect(rightOrigin, rightSize);
		}

		return this->insert(rectComp->left, rectSize);
	}
}

ECS::Entity * RectPackingScene::createNewEntity()
{
	ECS::Entity* newEntity = new ECS::Entity();

	ECS::RectPackingNode* node = new ECS::RectPackingNode();
	auto& area = this->displayBoundaryBoxNode->displayBoundary;
	//area.size.width -= this->padding;
	//area.size.height -= this->padding;
	node->area = area;
	node->area.origin -= this->displayBoundaryShift;
	node->color = cocos2d::Color4F(Utility::Random::randomReal<float>(0, 1.0f), Utility::Random::randomReal<float>(0, 1.0f), Utility::Random::randomReal<float>(0, 1.0f), 1.0f);
	newEntity->components.at(static_cast<int>(ECS::COMPONENT_ID::RECT_PACKING_NODE)) = node;

	return newEntity;
}

void RectPackingScene::drawRects(ECS::Entity* entity)
{
	auto rectComp = entity->getComponent<ECS::RectPackingNode*>(ECS::COMPONENT_ID::RECT_PACKING_NODE);
	if (rectComp->isLeaf())
	{
		// This is a leaf. Check if it has rect that perfectly fits
		if (rectComp->rect.equals(cocos2d::Rect::ZERO))
		{
			// Doesn't have any rectangle that perfectly fits here. It's just empty
			return;
		}
		else
		{
			// Have a rectangle that perfectly fits. Draw.
			cocos2d::Vec2 origin = rectComp->rect.origin;
			cocos2d::Vec2 dest = origin + cocos2d::Vec2(rectComp->rect.size);

			origin.x += (this->padding * 0.5f);
			origin.y += (this->padding * 0.5f);
			dest.x -= (this->padding * 0.5f);
			dest.y -= (this->padding * 0.5f);

			this->rectDrawNode->drawSolidRect(origin + this->displayBoundaryShift, dest + this->displayBoundaryShift, rectComp->color);
		}
	}
	else
	{
		// This is not a leaf
		if (rectComp->left== nullptr || rectComp->right == nullptr)
		{
			// Both left and right can't be nullptr
			return;
		}

		// draw split line
		if (this->drawDivisionLine)
		{
			auto leftRectComp = rectComp->left->getComponent<ECS::RectPackingNode*>(ECS::COMPONENT_ID::RECT_PACKING_NODE);
			auto rightRectComp = rectComp->right->getComponent<ECS::RectPackingNode*>(ECS::COMPONENT_ID::RECT_PACKING_NODE);

			cocos2d::Vec2 start;
			cocos2d::Vec2 end;

			if (leftRectComp->area.getMidY() == rightRectComp->area.getMidY())
			{
				// splited vertically, draw vertical line
				start.x = leftRectComp->area.getMaxX();
				start.y = rectComp->area.getMinY();
				end.x = start.x;
				end.y = start.y + rectComp->area.size.height;
			}
			else
			{
				// splited horizontally, /draw horizontal line
				start.x = leftRectComp->area.getMinX();
				start.y = leftRectComp->area.getMaxY();
				end.x = start.x + rectComp->area.size.width;
				end.y = start.y;
			}

			this->rectDrawNode->drawLine(start + this->displayBoundaryShift, end + this->displayBoundaryShift, cocos2d::Color4F::GREEN);
		}
		
		// Draw left
		this->drawRects(rectComp->left);
		// Draw right
		this->drawRects(rectComp->right);

		return;
	}
}

void RectPackingScene::restart()
{
	clear();

	this->root = this->createNewEntity();

	initRects();

	this->finished = false;

	this->buttonModifierNode->leftButtons.back()->setEnabled(false);
	this->buttonModifierNode->rightButtons.back()->setEnabled(false);

	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Running");
}

void RectPackingScene::initRects()
{
	this->maxRects = 1200;

	// Initialize rects.
	// Create 100 small rects
	std::vector<cocos2d::Size> sizes;
	const int smallCount = static_cast<int>(static_cast<float>(this->maxRects) * 0.5f);
	for (int i = 0; i < smallCount; i++)
	{
		sizes.push_back(cocos2d::Size(Utility::Random::randomInt(5, 20), Utility::Random::randomInt(5, 20)));
	}

	// Create 150 medium rects
	const int mediumCount = static_cast<int>(static_cast<float>(this->maxRects) * 0.25f);
	for (int i = 0; i < mediumCount; i++)
	{
		sizes.push_back(cocos2d::Size(Utility::Random::randomInt(10, 30), Utility::Random::randomInt(10, 30)));
	}
	// Create 50 large rects
	const int largeCount = static_cast<int>(static_cast<float>(this->maxRects) * 0.25f);
	for (int i = 0; i < largeCount; i++)
	{
		sizes.push_back(cocos2d::Size(Utility::Random::randomInt(15, 40), Utility::Random::randomInt(15, 40)));
	}

	cocos2d::log("Total rect created = %d", sizes.size());

	this->maxRects = smallCount + mediumCount + largeCount;

	std::shuffle(std::begin(sizes), std::end(sizes), std::mt19937(std::random_device{}()));

	for (auto size : sizes)
	{
		this->randomSizes.push(size);
	}

	cocos2d::log("Total rect to pack = %d", this->randomSizes.size());
}

void RectPackingScene::clear()
{
	// Stop and clear
	this->maxRects = 0;
	this->totalRectsPacked = 0;
	this->pause = false;
	this->finished = true;
	this->clearEntity();
	this->rectDrawNode->clear();
	this->buttonModifierNode->leftButtons.back()->setEnabled(true);
	this->buttonModifierNode->rightButtons.back()->setEnabled(true);
	std::queue<cocos2d::Size> empty;
	std::swap(this->randomSizes, empty);

	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Waiting");
}

void RectPackingScene::clearEntity()
{
	if (this->root != nullptr)
	{
		delete this->root;
		this->root = nullptr;
	}
}


void RectPackingScene::onButtonPressed(cocos2d::Ref * sender)
{
	auto button = dynamic_cast<cocos2d::ui::Button*>(sender);
	auto tag = static_cast<ACTION_TAG>(button->getActionTag());
	switch (tag)
	{
	case ACTION_TAG::LEFT:
		if (this->finished)
		{
			this->padding -= 1.0f;

			if (this->padding < 0)
			{
				this->padding = 0;
			}
		}
		this->buttonModifierNode->updateValue(0, this->padding);
		break;
	case ACTION_TAG::RIGHT:
		if (this->finished)
		{
			this->padding += 1.0f;

			if (this->padding > 5.0f)
			{
				this->padding = 5.0f;
			}
		}
		this->buttonModifierNode->updateValue(0, this->padding);
		break;
	default:
		break;
	}
}

void RectPackingScene::onSliderClick(cocos2d::Ref* sender)
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

void RectPackingScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(RectPackingScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(RectPackingScene::onMouseDown, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(RectPackingScene::onKeyPressed, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void RectPackingScene::onMouseMove(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	auto point = cocos2d::Vec2(x, y);

	this->labelsNode->updateMouseHover(point);
}

void RectPackingScene::onMouseDown(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	int mouseButton = mouseEvent->getMouseButton();
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	auto point = cocos2d::Vec2(x, y);

	bool ret = this->labelsNode->updateMouseDown(point);
	if (ret)
	{
		return;
	}
}

void RectPackingScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE)
	{
		// Terminate 
		cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_SPACE)
	{
		this->pause = !this->pause;
		if (this->pause)
		{
			this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::PAUSE), cocos2d::Color3B::GREEN);
			this->labelsNode->updateTimeTakenLabel("0");
		}
		else
		{
			this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::PAUSE), cocos2d::Color3B::WHITE);
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_L)
	{
		this->drawDivisionLine = !this->drawDivisionLine;
		if (this->finished == false)
		{
			this->rectDrawNode->clear();
			this->drawRects(this->root);
		}

		if (this->drawDivisionLine)
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
		this->restart();
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_C)
	{
		this->clear();
	}
}

void RectPackingScene::releaseInputListeners()
{
	if(this->mouseInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->mouseInputListener);
	if(this->keyInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->keyInputListener);
}

void RectPackingScene::onExit()
{
	cocos2d::CCScene::onExit();
	releaseInputListeners(); 

	if (this->root != nullptr)
	{
		delete this->root;
	}
}