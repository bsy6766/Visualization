#include "SortScene.h"
#include <algorithm>
#include "MainScene.h"

USING_NS_CC;

SortScene* SortScene::createScene()
{
	SortScene* newSortScene = SortScene::create();
	return newSortScene;
}

bool SortScene::init()
{
	if (!cocos2d::Scene::init())
	{
		return false;
	}

	// Uncomment this to activate update(float) function
	this->scheduleUpdate();

	paused = false;

	// init display boundary box node which draws outer line of simulation display box
	this->displayBoundaryBoxNode = DisplayBoundaryBoxNode::createNode();
	this->displayBoundaryBoxNode->setPosition(cocos2d::Vec2::ZERO);
	this->displayBoundaryBoxNode->displayBoundary = cocos2d::Rect(0, 0, 650, 650);
	this->displayBoundaryBoxNode->drawDisplayBoundaryBox();
	this->displayBoundaryBoxNode->retain();
	this->displayBoundaryBoxNode->drawNode->setLocalZOrder(static_cast<int>(Z_ORDER::BOUNDARY_WALL));
	this->addChild(this->displayBoundaryBoxNode, Z_ORDER::BOUNDARY_WALL);

	for (int i = 0; i < MAX_VALUE_SIZE; i++)
	{
		bars.push_back(cocos2d::Sprite::createWithSpriteFrameName("dot.png"));
		bars.back()->setScaleX(10.0f);
		bars.back()->setAnchorPoint(cocos2d::Vec2(0, 0));
		bars.back()->setPosition(cocos2d::Vec2(35.0f + (static_cast<float>(i) * 10.0f), 36.0f));
		bars.back()->setLocalZOrder(Z_ORDER::BAR);
		this->addChild(bars.back());
	}

	resetValues();

	sortMode = SORT_MODE::NONE;

	checkSpeed = 0.07f;
	checkIndex = 0;

	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	// Init labels node
	this->labelsNode = LabelsNode::createNode();
	this->labelsNode->setSharedLabelPosition(LabelsNode::SHARED_LABEL_POS_TYPE::QUADTREE_SCENE);
	this->addChild(this->labelsNode);

	// Starting pos
	float labelX = winSize.height - 10.0f;
	float labelY = winSize.height - 45.0f;

	// Set title
	this->labelsNode->initTitleStr("Sort", cocos2d::Vec2(labelX, labelY));

	labelY -= 50.0f;

	// Init custom labels
	this->labelsNode->customLabelStartPos = cocos2d::Vec2(labelX, labelY);

	// Set size
	const int customLabelSize = 25;

	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Status: Idle", customLabelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Current sort: None", customLabelSize);

	// Calculate next label block y
	const float customLastY = this->labelsNode->customLabels.back()->getBoundingBox().getMinY();
	const float blockGap = 22.0f;

	this->labelsNode->keyboardUsageLabelStartPos = cocos2d::Vec2(labelX, customLastY - blockGap);

	const int headerSize = 25;
	const int labelSize = 20;

	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Keys (Green = enabled)", headerSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Space: Toggle update", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Enter: Step", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "R: Reset", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "1: Selection sort", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "2: Insertion sort", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "3: Merge", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "4: Bubble", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "5: Quick", labelSize);

	this->simulationSpeedModifier = 1.0f;

	this->sliderLabelNode = SliderLabelNode::createNode();
	const float mouseLastY = this->labelsNode->keyboardUsageLabels.back()->getBoundingBox().getMinY();
	this->sliderLabelNode->sliderStartPos = cocos2d::Vec2(labelX, mouseLastY - blockGap);
	this->sliderLabelNode->addSlider("Simulation Speed", "Slider", 50, CC_CALLBACK_1(SortScene::onSliderClick, this));
	this->addChild(this->sliderLabelNode);

	return true;
}

void SortScene::onEnter()
{
	cocos2d::Scene::onEnter();
	// Uncomment this to enable mouse and keyboard event listeners
	initInputListeners();
}

void SortScene::update(float delta)
{
	float dt = delta * simulationSpeedModifier;

	switch (sortMode)
	{
	case SortScene::SORT_MODE::SELECTION:
	{
		updateSelectionSort(delta);
	}
		break;
	case SortScene::SORT_MODE::INSERTION:
	{
		updateInsertionSort(delta);
	}
		break;
	case SortScene::SORT_MODE::MERGE:
		break;
	case SortScene::SORT_MODE::BUBBLE:
		break;
	case SortScene::SORT_MODE::QUICK:
		break;
	case SortScene::SORT_MODE::NONE:
	default:
		break;
	}
}

void SortScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(SortScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(SortScene::onMouseDown, this);
	this->mouseInputListener->onMouseUp = CC_CALLBACK_1(SortScene::onMouseUp, this);
	this->mouseInputListener->onMouseScroll = CC_CALLBACK_1(SortScene::onMouseScroll, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(SortScene::onKeyPressed, this);
	this->keyInputListener->onKeyReleased = CC_CALLBACK_2(SortScene::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void SortScene::onMouseMove(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void SortScene::onMouseDown(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	//int mouseButton = mouseEvent->getMouseButton();
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void SortScene::onMouseUp(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	//int mouseButton = mouseEvent->getMouseButton();
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void SortScene::onMouseScroll(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//float x = mouseEvent->getScrollX();
	//float y = mouseEvent->getScrollY();
}

void SortScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE)
	{
		cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));

	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_SPACE)
	{
		paused = !paused;
		this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::PAUSE), paused ? cocos2d::Color3B::GREEN : cocos2d::Color3B::WHITE);
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ENTER)
	{
		if (paused)
		{
			switch (sortMode)
			{
			case SortScene::SORT_MODE::SELECTION:
				searchElapsedTime = 0;
				if (selectionSortState == SELECTION_SORT_STATE::SEARCHING_MIN_VALUE)
				{
					stepSelectionSort();
				}
				break;
			case SortScene::SORT_MODE::INSERTION:
				if (insertionSortState == INSERTION_SORT_STATE::SELECTING_NEXT)
				{
					stepInsertionSort();
				}
				break;
			case SortScene::SORT_MODE::MERGE:
				break;
			case SortScene::SORT_MODE::BUBBLE:
				break;
			case SortScene::SORT_MODE::QUICK:
				break;
			default:
			case SortScene::SORT_MODE::NONE:
				break;
			}
			this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::STEP), cocos2d::Color3B::WHITE);
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_R)
	{
		clearPrevSortModeLabelColor();
		reset();
		this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::RESET), cocos2d::Color3B::WHITE);
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_1)
	{
		// selection sort
		clearPrevSortModeLabelColor();
		initSelectionSort();
		this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::KEY_1),cocos2d::Color3B::GREEN);
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_2)
	{
		// selection sort
		clearPrevSortModeLabelColor();
		initInsertionSort();
		this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::KEY_2), cocos2d::Color3B::GREEN);
	}
}

void SortScene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{

}

void SortScene::releaseInputListeners()
{
	if(this->mouseInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->mouseInputListener);
	if(this->keyInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->keyInputListener);
}

void SortScene::resetValues()
{
	values.clear();

	for (int i = 1; i <= MAX_VALUE_SIZE; i++)
	{
		values.push_back(i);
	}

	for (int i = 0; i < MAX_VALUE_SIZE; i++)
	{
		float percentage = static_cast<float>(values.at(i)) / 65.0f * 100.0f;
		bars.at(i)->setScaleY(percentage * 6.5f);
	}
}

void SortScene::randomizeValues()
{
	values.clear();

	for (int i = 1; i <= MAX_VALUE_SIZE; i++)
	{
		values.push_back(i);
	}

	std::random_shuffle(values.begin(), values.end());
	
	for (int i = 0; i < 65; i++)
	{
		bars.at(i)->stopAllActions();
		float percentage = static_cast<float>(values.at(i)) / 65.0f * 100.0f;
		bars.at(i)->setScaleY(percentage * 6.5f);
	}
}

void SortScene::reset()
{
	sortMode = SORT_MODE::NONE;
	resetValues();

	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Idle");
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::CURRENT_SORT), "Current sort: None");

	resetBar();
}

void SortScene::resetBar()
{
	for (auto bar : bars)
	{
		bar->setColor(cocos2d::Color3B::WHITE);
		bar->stopAllActions();
	}
}

void SortScene::clearPrevSortModeLabelColor()
{
	switch (sortMode)
	{
	case SortScene::SORT_MODE::SELECTION:
		this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::KEY_1), cocos2d::Color3B::WHITE, false);
		break;
	case SortScene::SORT_MODE::INSERTION:
		this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::KEY_2), cocos2d::Color3B::WHITE, false);
		break;
	case SortScene::SORT_MODE::MERGE:
		this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::KEY_3), cocos2d::Color3B::WHITE, false);
		break;
	case SortScene::SORT_MODE::BUBBLE:
		this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::KEY_4), cocos2d::Color3B::WHITE, false);
		break;
	case SortScene::SORT_MODE::QUICK:
		this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::KEY_5), cocos2d::Color3B::WHITE, false);
		break;
	case SortScene::SORT_MODE::NONE:
	default:
		break;
	}
}

void SortScene::initSelectionSort()
{
	randomizeValues();
	resetBar();
	sortMode = SORT_MODE::SELECTION;

	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Sorting");
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::CURRENT_SORT), "Current sort: Selection Sort");

	minSearchIndex = 0;
	minSelectedIndex = -1;
	sortedIndex = -1;
	searchSpeed = 0.04f;
	searchElapsedTime = 0;
	selectionSortState = SELECTION_SORT_STATE::NONE;

	checkIndex = 0;
	checkElapsedTime = 0.0f;
}

void SortScene::updateSelectionSort(const float delta)
{
	if (selectionSortState == SELECTION_SORT_STATE::NONE)
	{
		selectionSortState = SELECTION_SORT_STATE::SEARCHING_MIN_VALUE;
		//bars.at(0)->setColor(cocos2d::Color3B::RED);
	}
	else if (selectionSortState == SELECTION_SORT_STATE::SEARCHING_MIN_VALUE)
	{
		if (paused) return;

		searchElapsedTime += (delta * simulationSpeedModifier);
		if (searchElapsedTime >= searchSpeed)
		{
			//bars.at(minSearchIndex)
			searchElapsedTime -= searchSpeed;
			stepSelectionSort();
		}
	}
	else if (selectionSortState == SELECTION_SORT_STATE::SWAP)
	{
		searchElapsedTime += delta;
		if (searchElapsedTime >= searchSpeed * (2.0f - simulationSpeedModifier) * 2.0f)
		{
			searchElapsedTime = 0;
			selectionSortState = SELECTION_SORT_STATE::SEARCHING_MIN_VALUE;

			int siValue = values.at(sortedIndex);
			int minValue = values.at(minSelectedIndex);

			values.at(sortedIndex) = minValue;
			values.at(minSelectedIndex) = siValue;

			bars.at(sortedIndex)->stopAllActions();
			bars.at(minSelectedIndex)->stopAllActions();

			bars.at(minSelectedIndex)->setScaleY((static_cast<float>(siValue) / 65.0f * 100.0f * 6.5f));
			bars.at(minSelectedIndex)->setColor(cocos2d::Color3B::WHITE);
			bars.at(sortedIndex)->setScaleY((static_cast<float>(minValue) / 65.0f * 100.0f * 6.5f));
			bars.at(sortedIndex)->setColor(cocos2d::Color3B::YELLOW);

			minSearchIndex = 0;
			minSelectedIndex = -1;

			minSearchIndex = sortedIndex + 1;

			searchElapsedTime = 0;
			
		}
	}
	else if (selectionSortState == SELECTION_SORT_STATE::CHECK)
	{
		checkSort(delta);
	}
}

void SortScene::stepSelectionSort()
{
	if (minSearchIndex < MAX_VALUE_SIZE)
	{
		// change previous searching bar to white. if it was min bar, mark red
		if (minSearchIndex >(sortedIndex + 1))
		{
			if ((minSearchIndex - 1) != minSelectedIndex)
			{
				bars.at(minSearchIndex - 1)->setColor(cocos2d::Color3B::WHITE);
			}
			else
			{
				bars.at(minSearchIndex - 1)->setColor(cocos2d::Color3B::RED);
			}
		}

		// set current bar to blue
		bars.at(minSearchIndex)->setColor(cocos2d::Color3B::BLUE);

		// check if minimum value was found this iteration
		if (minSelectedIndex == -1)
		{
			// not found. set as after sorted Index
			minSelectedIndex = sortedIndex + 1;
			// mark as red only if it's not blue
			if (minSelectedIndex != minSearchIndex)
			{
				bars.at(minSelectedIndex)->setColor(cocos2d::Color3B::RED);
			}
		}
		else
		{
			// there is min value. check with new value
			if (values.at(minSelectedIndex) > values.at(minSearchIndex))
			{
				bars.at(minSelectedIndex)->setColor(cocos2d::Color3B::WHITE);
				minSelectedIndex = minSearchIndex;
				bars.at(minSelectedIndex)->setColor(cocos2d::Color3B::RED);
			}
		}

		minSearchIndex++;
	}
	else
	{
		sortedIndex++;
		if (sortedIndex == 65)
		{
			selectionSortState = SELECTION_SORT_STATE::CHECK;
			this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Checking");
		}
		else
		{
			int siValue = values.at(sortedIndex);
			int minValue = values.at(minSelectedIndex);

			float ssmFlip = (2.0f - simulationSpeedModifier) * 2.0f;
			bars.at(sortedIndex)->runAction(cocos2d::ScaleTo::create(searchSpeed * ssmFlip, 10.0f, (static_cast<float>(minValue) / 65.0f * 100.0f * 6.5f), 1.0f));
			bars.at(sortedIndex)->runAction(cocos2d::TintTo::create(searchSpeed * ssmFlip, cocos2d::Color3B::RED));
			bars.at(minSelectedIndex)->runAction(cocos2d::ScaleTo::create(searchSpeed * ssmFlip, 10.0f, (static_cast<float>(siValue) / 65.0f * 100.0f * 6.5f), 1.0f));
			bars.at(minSelectedIndex)->runAction(cocos2d::TintTo::create(searchSpeed * ssmFlip, cocos2d::Color3B::WHITE));

			bars.at(minSearchIndex - 1)->setColor(cocos2d::Color3B::WHITE);
			selectionSortState = SELECTION_SORT_STATE::SWAP;

			searchElapsedTime = 0;
		}
	}
}

void SortScene::initInsertionSort()
{
	randomizeValues();
	resetBar();
	sortMode = SORT_MODE::INSERTION;

	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Sorting");
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::CURRENT_SORT), "Current sort: Insertion Sort");

	curIndex = -1;
	swappingIndex = 0;
	animSpeed = 0.06f;
	insertionSortState = INSERTION_SORT_STATE::NONE;

	checkIndex = 0;
	checkElapsedTime = 0.0f;
}

void SortScene::updateInsertionSort(const float delta)
{
	if (insertionSortState == INSERTION_SORT_STATE::NONE)
	{
		if (curIndex == -1)
		{
			curIndex = 0;
			float ssmFlip = (2.0f - simulationSpeedModifier) * 2.0f;
			bars.at(curIndex)->runAction(cocos2d::TintTo::create(animSpeed * ssmFlip, cocos2d::Color3B::BLUE));
		}

		if (bars.at(curIndex)->getNumberOfRunningActions() == 0)
		{
			insertionSortState = INSERTION_SORT_STATE::SWAP;
		}
	}
	else if (insertionSortState == INSERTION_SORT_STATE::SELECTING_NEXT)
	{
		if (paused) return;

		stepInsertionSort();
	}
	else if (insertionSortState == INSERTION_SORT_STATE::SWAP)
	{
		if (curIndex == 0)
		{
			float ssmFlip = (2.0f - simulationSpeedModifier) * 2.0f;
			bars.at(curIndex)->runAction(cocos2d::TintTo::create(animSpeed * ssmFlip, cocos2d::Color3B::YELLOW));
		}
		else
		{
			float ssmFlip = (2.0f - simulationSpeedModifier) * 2.0f;

			if (swappingIndex >= 0)
			{
				int curVal = values.at(curIndex);
				for (int i = curIndex; i >= swappingIndex; i--)
				{
					if (i == swappingIndex)
					{
						bars.at(i)->runAction(cocos2d::ScaleTo::create(animSpeed * ssmFlip, 10.0f, (static_cast<float>(curVal) / 65.0f * 100.0f * 6.5f), 1.0f));
						//bars.at(i)->runAction(cocos2d::TintTo::create(animSpeed * ssmFlip, cocos2d::Color3B::YELLOW));
						values.at(i) = curVal;
					}
					else
					{
						bars.at(i)->runAction(cocos2d::ScaleTo::create(animSpeed * ssmFlip, 10.0f, (static_cast<float>(values.at(i - 1)) / 65.0f * 100.0f * 6.5f), 1.0f));
						bars.at(i)->runAction(cocos2d::TintTo::create(animSpeed * ssmFlip, cocos2d::Color3B::YELLOW));
						values.at(i) = values.at(i - 1);
					}
				}
			}
			else
			{
				float ssmFlip = (2.0f - simulationSpeedModifier) * 2.0f;
				bars.at(curIndex)->runAction(cocos2d::TintTo::create(animSpeed * ssmFlip, cocos2d::Color3B::YELLOW));
			}
		}

		insertionSortState = INSERTION_SORT_STATE::SWAPPING;
	}
	else if (insertionSortState == INSERTION_SORT_STATE::SWAPPING)
	{
		if (bars.at(curIndex)->getNumberOfRunningActions() == 0)
		{
			insertionSortState = INSERTION_SORT_STATE::SELECTING_NEXT;
			float ssmFlip = (2.0f - simulationSpeedModifier) * 2.0f;
			if (swappingIndex != -1 && (curIndex != swappingIndex || curIndex > 0))
			{
				//bars.at(curIndex)->runAction(cocos2d::TintTo::create(animSpeed * ssmFlip, cocos2d::Color3B::WHITE));
			}
			curIndex++;

			if (curIndex >= MAX_VALUE_SIZE)
			{
				insertionSortState = INSERTION_SORT_STATE::CHECK;
				
				return;
			}

			bars.at(curIndex)->runAction(cocos2d::TintTo::create(animSpeed * ssmFlip, cocos2d::Color3B::BLUE));
		}
	}
	else if (insertionSortState == INSERTION_SORT_STATE::CHECK)
	{
		checkSort(delta);
	}
}

void SortScene::stepInsertionSort()
{
	if (bars.at(curIndex)->getNumberOfRunningActions() == 0)
	{
		insertionSortState = INSERTION_SORT_STATE::SWAP;

		swappingIndex = -1;
		int curVal = values.at(curIndex);
		for (int i = curIndex - 1; i >= 0; i--)
		{
			if (curVal < values.at(i))
			{
				swappingIndex = i;
			}
			else
			{
				break;
			}
		}
	}
}

void SortScene::checkSort(const float delta)
{
	checkElapsedTime += (delta * simulationSpeedModifier);

	if (checkElapsedTime <= checkSpeed)
	{
		checkElapsedTime -= checkSpeed;

		bars.at(checkIndex)->runAction(cocos2d::Sequence::createWithTwoActions(cocos2d::TintTo::create(checkSpeed, cocos2d::Color3B::GREEN), cocos2d::TintTo::create(checkSpeed, cocos2d::Color3B::YELLOW)));

		checkIndex++;

		if (checkIndex == MAX_VALUE_SIZE)
		{
			switch (sortMode)
			{
			case SortScene::SORT_MODE::SELECTION:
				selectionSortState = SELECTION_SORT_STATE::FINISHED;
				break;
			case SortScene::SORT_MODE::INSERTION:
				insertionSortState = INSERTION_SORT_STATE::FINISHED;
				break;
			case SortScene::SORT_MODE::MERGE:
				break;
			case SortScene::SORT_MODE::BUBBLE:
				break;
			case SortScene::SORT_MODE::QUICK:
				break;
			case SortScene::SORT_MODE::NONE:
			default:
				break;
			}
		}
	}
}

void SortScene::onSliderClick(cocos2d::Ref* sender)
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

void SortScene::onExit()
{
	cocos2d::Scene::onExit();
	// Uncomment this if you are using initInputListeners()
	releaseInputListeners(); 
}