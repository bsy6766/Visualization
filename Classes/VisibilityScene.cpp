#include "VisibilityScene.h"
#include "MainScene.h"

USING_NS_CC;

VisibilityScene* VisibilityScene::createScene()
{
	VisibilityScene* newVisibilityScene = VisibilityScene::create();
	return newVisibilityScene;
}

bool VisibilityScene::init()
{
	if (!cocos2d::Scene::init())
	{
		return false;
	}

	this->scheduleUpdate();

	ECS::Manager::getInstance();

	// init display boundary box node which draws outer line of simulation display box
	this->displayBoundaryBoxNode = DisplayBoundaryBoxNode::createNode();
	this->displayBoundaryBoxNode->setPosition(cocos2d::Vec2::ZERO);
	this->displayBoundaryBoxNode->displayBoundary = cocos2d::Rect(0, 0, 650, 650);
	this->displayBoundaryBoxNode->drawDisplayBoundaryBox();
	this->displayBoundary = this->displayBoundaryBoxNode->displayBoundary;
	this->displayBoundaryBoxNode->retain();
	this->displayBoundaryBoxNode->drawNode->setLocalZOrder(static_cast<int>(Z_ORDER::BOX));
	this->addChild(this->displayBoundaryBoxNode);

	// Init labels node
	this->labelsNode = LabelsNode::createNode();
	this->labelsNode->setSharedLabelPosition(LabelsNode::SHARED_LABEL_POS_TYPE::QUADTREE_SCENE);
	this->addChild(this->labelsNode);

	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	// Starting pos
	float labelX = winSize.height - 10.0f;
	float labelY = winSize.height - 45.0f;

	// Set title
	this->labelsNode->initTitleStr("Visibility", cocos2d::Vec2(labelX, labelY));

	labelY -= 50.0f;

	// Init custom labels
	this->labelsNode->customLabelStartPos = cocos2d::Vec2(labelX, labelY);

	// Set size
	const int customLabelSize = 25;

	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Status: Idle", customLabelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Mode: Idle", customLabelSize);

	this->currentMode = MODE::IDLE;
	this->draggingBox = false;
	this->newBoxOrigin = cocos2d::Vec2::ZERO;
	this->newBoxDest = cocos2d::Vec2::ZERO;
	this->newBoxDrawNode = cocos2d::DrawNode::create();
	this->addChild(this->newBoxDrawNode);

	this->visiableAreaDrawNode = cocos2d::DrawNode::create();
	this->addChild(this->visiableAreaDrawNode);

	return true;
}

void VisibilityScene::onEnter()
{
	cocos2d::Scene::onEnter();

	initECS();

	initInputListeners();
}

void VisibilityScene::initECS()
{
	auto m = ECS::Manager::getInstance();
	m->createEntityPool("BOX", 32);
	m->createEntityPool("LIGHT", 16);
}

void VisibilityScene::createNewBox()
{
	auto m = ECS::Manager::getInstance();
	auto newBox = m->createEntity("BOX");

	auto spriteComp = m->createComponent<ECS::Sprite>();
	spriteComp->sprite = cocos2d::Sprite::createWithSpriteFrameName("square_100.png");
	cocos2d::Vec2 boxSize = this->newBoxDest - this->newBoxOrigin;
	cocos2d::Vec2 boxPos = this->newBoxOrigin + (boxSize * 0.5f);
	spriteComp->sprite->setPosition(boxPos);
	float scaleX = boxSize.x * 0.01f;
	float scaleY = boxSize.y * 0.01f;
	spriteComp->sprite->setScaleX(scaleX);
	spriteComp->sprite->setScaleY(scaleY);
	this->addChild(spriteComp->sprite);

	newBox->addComponent<ECS::Sprite>(spriteComp);
}

void VisibilityScene::createNewLight(const cocos2d::Vec2& position)
{
	auto m = ECS::Manager::getInstance();
	auto newLight = m->createEntity("LIGHT");

	auto spriteComp = m->createComponent<ECS::Sprite>();
	spriteComp->sprite = cocos2d::Sprite::createWithSpriteFrameName("circle.png");
	spriteComp->sprite->setPosition(position);
	this->addChild(spriteComp->sprite);

	newLight->addComponent<ECS::Sprite>(spriteComp);
}

void VisibilityScene::sweep()
{
	cocos2d::log("Sweeping...");
	std::vector<cocos2d::Vec2> output;

	std::list<Segment*> open;
	float beginAngle = 0;

	float maxAngle = 999.0f;

	std::vector<ECS::Entity*> lights;
	ECS::Manager::getInstance()->getAllEntitiesInPool(lights, "LIGHT");

	auto spriteComp = lights.at(0)->getComponent<ECS::Sprite>();
	auto pos = spriteComp->sprite->getPosition();

	cocos2d::log("Sort end points");
	std::sort(this->endPoints.begin(), this->endPoints.end(), EndPointComparator());

	for (auto ep : this->endPoints)
	{
		float degreeAngle = ep->angle * 180.0f / M_PI;
		cocos2d::log("Angle: %f (%f), pos: (%f, %f), v: %d, b: %d", ep->angle, degreeAngle, ep->x, ep->y, ep->visualize, ep->begin);
	}

	cocos2d::log("\nSweeping end points...");
	for (int i = 0; i < 2; i++)
	{
		if (i == 0)
		{
			cocos2d::log("Pre-sweeping...");
		}
		else
		{
			cocos2d::log("Post-sweeping...");
		}
		for (auto endPoint : this->endPoints)
		{
			cocos2d::log("");
			float degreeAngle = endPoint->angle * 180.0f / M_PI;
			cocos2d::log("EndPoint Angle: %f (%f), pos: (%f, %f), v: %d, b: %d", endPoint->angle, degreeAngle, endPoint->x, endPoint->y, endPoint->visualize, endPoint->begin);
			if (i == 1 && endPoint->angle > maxAngle)
			{
				break;
			}

			Segment* oldSegment = open.empty() ? nullptr : open.front();

			if (oldSegment == nullptr)
			{
				cocos2d::log("oldSegment is nullptr.");
			}
			else
			{
				cocos2d::log("oldSegment p1: (%f, %f), p2: (%f, %f)", oldSegment->p1->x, oldSegment->p1->y, oldSegment->p2->x, oldSegment->p2->y);
			}

			if (endPoint->begin)
			{
				auto it = open.begin();
				while (it != open.end() && (*it) != nullptr && this->checkSegmentInFrontOf(endPoint->segment, (*it), pos))
				{
					it++;
				}

				if (it == open.end())
				{
					open.push_back(endPoint->segment);
					cocos2d::log("Adding segment to open!");
					cocos2d::log("segment p1: (%f, %f), p2: (%f, %f)", endPoint->segment->p1->x, endPoint->segment->p1->y, endPoint->segment->p2->x, endPoint->segment->p2->y);
				}
				else
				{
					open.insert(it, endPoint->segment);
					cocos2d::log("Adding segment in to open!");
					cocos2d::log("segment p1: (%f, %f), p2: (%f, %f)", endPoint->segment->p1->x, endPoint->segment->p1->y, endPoint->segment->p2->x, endPoint->segment->p2->y);
				}
			}
			else
			{
				auto find_it = open.begin();
				for (; find_it != open.end();)
				{
					if ((*find_it)->p1 == endPoint->segment->p1)
					{
						cocos2d::log("Removing segment from open!");
						cocos2d::log("segment p1: (%f, %f), p2: (%f, %f)", (*find_it)->p1->x, (*find_it)->p1->y, (*find_it)->p2->x, (*find_it)->p2->y);
						//delete *find_it;
						open.erase(find_it);
						break;
					}

					find_it++;
				}
			}

			Segment* newSegment = open.empty() ? nullptr : open.front();
			if (newSegment == nullptr)
			{
				cocos2d::log("newSegment is nullptr.");
			}
			else
			{
				cocos2d::log("newSegment p1: (%f, %f), p2: (%f, %f)", newSegment->p1->x, newSegment->p1->y, newSegment->p2->x, newSegment->p2->y);
			}
			if (oldSegment != nullptr && newSegment != nullptr)
			{
				bool same = newSegment->p1 == oldSegment->p1;
				if (!same)
				{
					if (i == 1)
					{
						// add triangle
						this->addTriangle(beginAngle, endPoint->angle, oldSegment, pos);
					}
					beginAngle = endPoint->angle;
				}
			}
		}
	}
}

void VisibilityScene::loadMap()
{
	// clear vectors
	this->endPoints.clear();
	this->segments.clear();

	// Load edge of map (boundary)
	cocos2d::log("Loading boundary segments...");
	this->loadBoundary();

	// Load blocks(boxes)
	std::vector<ECS::Entity*> boxes;
	ECS::Manager::getInstance()->getAllEntitiesInPool(boxes, "BOX");

	cocos2d::log("Loading boxes...");
	for (auto box : boxes)
	{
		auto comp = box->getComponent<ECS::Sprite>();
		auto bb = comp->sprite->getBoundingBox();

		this->loadRect(bb);
	}

	// Load walls
}

void VisibilityScene::loadBoundary()
{
	cocos2d::log("Boundary origin: (%f, %f), size: (%f, %f)", this->displayBoundary.origin.x, this->displayBoundary.origin.y, this->displayBoundary.size.width, this->displayBoundary.size.height);
	this->loadRect(this->displayBoundary);
}

void VisibilityScene::loadRect(const cocos2d::Rect& rect)
{
	// Going clock wise from top left point, first point is begin and second point is not
	{
		// Top segment
		EndPoint* topLeft = new EndPoint();
		topLeft->x = rect.getMinX();
		topLeft->y = rect.getMaxY();

		EndPoint* topRight = new EndPoint();
		topRight->x = rect.getMaxX();
		topRight->y = rect.getMaxY();

		this->addSegment(*topLeft, *topRight);
	}

	{
		// right segment
		EndPoint* topRight = new EndPoint();
		topRight->x = rect.getMaxX();
		topRight->y = rect.getMaxY();

		EndPoint* bottomRight = new EndPoint();
		bottomRight->x = rect.getMaxX();
		bottomRight->y = rect.getMinY();

		this->addSegment(*topRight, *bottomRight);
	}

	{
		// bottom segment
		EndPoint* bottomRight = new EndPoint();
		bottomRight->x = rect.getMaxX();
		bottomRight->y = rect.getMinY();

		EndPoint* bottomLeft = new EndPoint();
		bottomLeft->x = rect.getMinX();
		bottomLeft->y = rect.getMinY();

		this->addSegment(*bottomRight, *bottomLeft);
	}

	{
		// Left segment
		EndPoint* topLeft = new EndPoint();
		topLeft->x = rect.getMinX();
		topLeft->y = rect.getMaxY();

		EndPoint* bottomLeft = new EndPoint();
		bottomLeft->x = rect.getMinX();
		bottomLeft->y = rect.getMinY();

		this->addSegment(*bottomLeft, *topLeft);
	}
}

void VisibilityScene::addSegment(EndPoint& p1, EndPoint& p2)
{
	Segment* segment = new Segment();
	p1.segment = segment;
	p2.segment = segment;

	p1.visualize = true;
	p2.visualize = false;

	segment->p1 = &p1;
	segment->p2 = &p2;
	
	segment->d = 0;

	this->segments.push_back(segment);

	this->endPoints.push_back(&p1);
	this->endPoints.push_back(&p2);

	cocos2d::log("Added segment p1: (%f, %f), p2: (%f, %f)", p1.x, p1.y, p2.x, p2.y);
}

void VisibilityScene::setLightLocation()
{
	std::vector<ECS::Entity*> lights;
	ECS::Manager::getInstance()->getAllEntitiesInPool(lights, "LIGHT");
	
	auto spriteComp = lights.at(0)->getComponent<ECS::Sprite>();
	auto pos = spriteComp->sprite->getPosition();

	cocos2d::log("Setting light location");

	for (auto segment : this->segments)
	{
		// The original article use distance sqaured for optimization, but it says sqrt in real practice is fast enough to use.
		// I'm going to use cocos2d-x's built-in distance function, which uses sqrt.
		//float dx = ((segment->p1->x + segment->p2->x) * 0.5f) - pos.x;
		//float dy = ((segment->p1->y + segment->p2->y) * 0.5f) - pos.y;

		cocos2d::Vec2 half = cocos2d::Vec2(segment->p1->x, segment->p1->y) + cocos2d::Vec2(segment->p2->x, segment->p2->y);
		half *= 0.5f;
		segment->d = half.distance(pos);

		// Get angle from light position
		segment->p1->angle = atan2(segment->p1->y - pos.y, segment->p1->x - pos.x);
		segment->p2->angle = atan2(segment->p2->y - pos.y, segment->p2->x - pos.x);

		float dAngle = segment->p2->angle - segment->p1->angle;
		if (dAngle <= -M_PI)
		{
			dAngle += (M_PI * 2.0f);
		}
		if (dAngle > M_PI)
		{
			dAngle -= (M_PI * 2.0f);
		}

		segment->p1->begin = (dAngle > 0.0f);
		segment->p2->begin = !segment->p2->begin;

		cocos2d::log("Segment p1: (%f, %f), p2: (%f, %f)", segment->p1->x, segment->p1->y, segment->p2->x, segment->p2->y);
		cocos2d::log("p1 angle: %f, p2 angle: %f", segment->p1->angle, segment->p2->angle);
		cocos2d::log("d: %f", segment->d);
	}
}

bool VisibilityScene::checkSegmentInFrontOf(Segment * a, Segment * b, const cocos2d::Vec2 & relativeOf)
{
	bool A1 = isLeftOf(a, interpolate(*b->p1, *b->p2, 0.01f));
	bool A2 = isLeftOf(a, interpolate(*b->p2, *b->p1, 0.01f));
	bool A3 = isLeftOf(a, relativeOf);

	bool B1 = isLeftOf(b, interpolate(*a->p1, *a->p2, 0.01f));
	bool B2 = isLeftOf(b, interpolate(*b->p2, *b->p1, 0.01f));
	bool B3 = isLeftOf(b, relativeOf);

	if (B1 == B2 && B2 != B3) return true;
	if (A1 == A2 && A2 == A3) return true;
	if (A1 == A2 && A2 != A3) return false;
	if (B1 == B2 && B2 == B3) return false;

	return false;
}

bool VisibilityScene::isLeftOf(Segment * segment, const cocos2d::Vec2 & point)
{
	// Cross product. If result is negative, point is on left from segment. 0 == parallel
	float crossProduct = (segment->p2->x - segment->p1->x) * (point.y - segment->p1->y)
						- (segment->p2->y - segment->p1->y) * (point.x - segment->p1->x);

	return crossProduct < 0;
}

const cocos2d::Vec2 VisibilityScene::interpolate(const cocos2d::Vec2 & p, const cocos2d::Vec2 & q, const float ratio)
{
	return cocos2d::Vec2((p.x * (1.0f - ratio)) + (q.x * ratio), (p.y * (1.0f - ratio)) + (q.y * ratio));
}

void VisibilityScene::addTriangle(float angle1, float angle2, Segment * segment, const cocos2d::Vec2 & lightPos)
{
	auto p1 = lightPos;
	auto p2 = cocos2d::Vec2(lightPos.x + cosf(angle1), lightPos.y + sinf(angle1));
	auto p3 = cocos2d::Vec2::ZERO;
	auto p4 = cocos2d::Vec2::ZERO;

	if (segment != nullptr)
	{
		p3.x = segment->p1->x;
		p3.y = segment->p1->y;
		p4.x = segment->p2->x;
		p4.y = segment->p2->y;
	}
	else
	{
		p3.x = lightPos.x + cosf(angle1) * 500.0f;
		p3.y = lightPos.y + sinf(angle1) * 500.0f;
		p4.x = lightPos.x + cosf(angle2) * 500.0f;
		p4.y = lightPos.y + sinf(angle2) * 500.0f;
	}

	auto pointBegin = getIntersectingPoint(p3, p4, p1, p2);

	p2.x = lightPos.x + cosf(angle2);
	p2.y = lightPos.y + sinf(angle2);

	auto pointEnd = getIntersectingPoint(p3, p4, p1, p2);

	verticies.push_back(pointBegin);
	verticies.push_back(lightPos);
	verticies.push_back(pointEnd);
}

const cocos2d::Vec2 VisibilityScene::getIntersectingPoint(const cocos2d::Vec2 & p1, const cocos2d::Vec2 & p2, const cocos2d::Vec2 & p3, const cocos2d::Vec2 & p4)
{
	float s = ((p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x))
		/ ((p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y));
	return cocos2d::Vec2(p1.x + s * (p2.x - p1.x), p1.y + s * (p2.y - p1.y));
}

void VisibilityScene::update(float delta)
{
	this->labelsNode->updateFPSLabel(delta);
}

void VisibilityScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(VisibilityScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(VisibilityScene::onMouseDown, this);
	this->mouseInputListener->onMouseUp = CC_CALLBACK_1(VisibilityScene::onMouseUp, this);
	this->mouseInputListener->onMouseScroll = CC_CALLBACK_1(VisibilityScene::onMouseScroll, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(VisibilityScene::onKeyPressed, this);
	this->keyInputListener->onKeyReleased = CC_CALLBACK_2(VisibilityScene::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void VisibilityScene::onMouseMove(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	int mouseButton = mouseEvent->getMouseButton();
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	cocos2d::Vec2 point = cocos2d::Vec2(x, y);

	if (this->displayBoundary.containsPoint(point))
	{
		if (this->currentMode == MODE::BOX)
		{
			if (this->draggingBox)
			{
				cocos2d::Vec2 size = point - this->newBoxOrigin;
				if (size.x > 100.0f)
				{
					point.x = this->newBoxOrigin.x + 100.0f;
				}
				else if (size.x < -100.0f)
				{
					point.x = this->newBoxOrigin.x - 100.0f;
				}

				if (size.y > 100.0f)
				{
					point.y = this->newBoxOrigin.y + 100.0f;
				}
				else if (size.y < -100.0f)
				{
					point.y = this->newBoxOrigin.y - 100.0f;
				}

				float fx = floorf(point.x);
				float fy = floorf(point.y);
				this->newBoxDest = cocos2d::Vec2(fx, fy);

				this->newBoxDrawNode->clear();
				this->newBoxDrawNode->drawSolidRect(this->newBoxOrigin, this->newBoxDest, cocos2d::Color4F::ORANGE);
				this->newBoxDrawNode->drawRect(this->newBoxOrigin, this->newBoxDest, cocos2d::Color4F::YELLOW);
			}
		}
	}

}

void VisibilityScene::onMouseDown(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	int mouseButton = mouseEvent->getMouseButton();
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	cocos2d::Vec2 point = cocos2d::Vec2(x, y);

	if (this->displayBoundary.containsPoint(point))
	{	
		if (this->currentMode == MODE::BOX)
		{
			// Box mode. draw new box
			auto m = ECS::Manager::getInstance();
			auto boxCount = m->getAliveEntityCountInEntityPool("BOX");
			if (boxCount < 32)
			{
				// still can make another box
				this->draggingBox = true;
				float fx = floorf(point.x);
				float fy = floorf(point.y);
				this->newBoxOrigin = cocos2d::Vec2(fx, fy);
			}
			else
			{
				// Can't make more boxes. return
				return;
			}
		}
		else if (this->currentMode == MODE::LIGHT)
		{
			this->createNewLight(point);
		}
	}


}

void VisibilityScene::onMouseUp(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	int mouseButton = mouseEvent->getMouseButton();
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	cocos2d::Vec2 point = cocos2d::Vec2(x, y);

	if (this->displayBoundary.containsPoint(point))
	{
		if (this->currentMode == MODE::BOX)
		{
			if (this->draggingBox)
			{
				this->draggingBox = false;

				// Make new box
				this->createNewBox();

				this->newBoxDrawNode->clear();
			}
		}
	}
	else
	{
		if (this->currentMode == MODE::BOX)
		{
			if (this->draggingBox)
			{
				this->draggingBox = false;
				this->newBoxDest = cocos2d::Vec2::ZERO;
				this->newBoxOrigin = cocos2d::Vec2::ZERO;
				this->newBoxDrawNode->clear();
			}
		}
	}
}

void VisibilityScene::onMouseScroll(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//float x = mouseEvent->getScrollX();
	//float y = mouseEvent->getScrollY();
}

void VisibilityScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE)
	{
		// Terminate
		cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_B)
	{
		// Box mode
		this->currentMode = MODE::BOX;
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_L)
	{
		// light mode
		this->currentMode = MODE::LIGHT;
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ENTER)
	{
		this->currentMode = MODE::IDLE;
		this->loadMap();
		this->setLightLocation();
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_SPACE)
	{
		this->sweep();
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_1)
	{
		this->visiableAreaDrawNode->clear();

		auto size = this->verticies.size();
		for (unsigned int i = 0; i < size; i += 3)
		{
			this->visiableAreaDrawNode->drawTriangle(this->verticies.at(i), this->verticies.at(i + 1), this->verticies.at(i + 2), cocos2d::Color4F::GREEN);
			this->visiableAreaDrawNode->drawLine(this->verticies.at(i), this->verticies.at(i + 1), cocos2d::Color4F::ORANGE);
			this->visiableAreaDrawNode->drawLine(this->verticies.at(i+1), this->verticies.at(i + 2), cocos2d::Color4F::ORANGE);
			this->visiableAreaDrawNode->drawLine(this->verticies.at(i + 2), this->verticies.at(i), cocos2d::Color4F::ORANGE);
		}
	}
}

void VisibilityScene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{

}

void VisibilityScene::releaseInputListeners()
{
	if(this->mouseInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->mouseInputListener);
	if(this->keyInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->keyInputListener);
}

void VisibilityScene::onExit()
{
	cocos2d::Scene::onExit();

	releaseInputListeners(); 

	ECS::Manager::deleteInstance();

	for (auto segment : this->segments)
	{
		delete segment;
	}
}