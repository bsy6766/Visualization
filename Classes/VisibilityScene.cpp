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

	//temp
	this->raycastDrawNode = cocos2d::DrawNode::create();
	this->addChild(this->raycastDrawNode, Z_ORDER::RAYCAST);
	this->triangleDrawNode = cocos2d::DrawNode::create();
	this->addChild(this->triangleDrawNode, Z_ORDER::TRIANGLE);

	this->mousePos = cocos2d::Vec2::ZERO;

	return true;
}

void VisibilityScene::onEnter()
{
	cocos2d::Scene::onEnter();

	initECS();

	initInputListeners();

	this->newBoxOrigin = cocos2d::Vec2(100, 100);
	this->newBoxDest = cocos2d::Vec2(200, 200);
	createNewBox();
}

void VisibilityScene::initECS()
{
	auto m = ECS::Manager::getInstance();
	m->createEntityPool("WALL", 32);
	m->createEntityPool("LIGHT", 16);
}

void VisibilityScene::createNewBox()
{
	auto m = ECS::Manager::getInstance();
	auto newBox = m->createEntity("WALL");

	auto spriteComp = m->createComponent<ECS::Sprite>();
	spriteComp->sprite = cocos2d::Sprite::createWithSpriteFrameName("square_100.png");
	cocos2d::Vec2 boxSize = this->newBoxDest - this->newBoxOrigin;
	cocos2d::Vec2 boxPos = this->newBoxOrigin + (boxSize * 0.5f);
	spriteComp->sprite->setPosition(boxPos);
	float scaleX = boxSize.x * 0.01f;
	float scaleY = boxSize.y * 0.01f;
	spriteComp->sprite->setScaleX(scaleX);
	spriteComp->sprite->setScaleY(scaleY);
	spriteComp->sprite->setOpacity(50);
	this->addChild(spriteComp->sprite, Z_ORDER::WALL);

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

	this->lightPositions.push_back(position);
}

void VisibilityScene::loadMap()
{
	// clear all
	this->wallUniquePoints.clear();
	this->boundaryUniquePoints.clear();

	for (auto segment : this->wallSegments)
	{
		if (segment) delete segment;
	}

	this->wallSegments.clear();

	for (auto segment : this->boundarySegments)
	{
		if (segment) delete segment;
	}

	this->boundarySegments.clear();

	// Add boundary
	this->boundaryUniquePoints.push_back(cocos2d::Vec2(this->displayBoundary.getMinX(), this->displayBoundary.getMaxY()));  //top left
	this->boundaryUniquePoints.push_back(cocos2d::Vec2(this->displayBoundary.getMaxX(), this->displayBoundary.getMaxY()));  //top right
	this->boundaryUniquePoints.push_back(cocos2d::Vec2(this->displayBoundary.getMinX(), this->displayBoundary.getMinY()));  //bottom left
	this->boundaryUniquePoints.push_back(cocos2d::Vec2(this->displayBoundary.getMaxX(), this->displayBoundary.getMinY()));  //bottom right

	this->loadRect(this->displayBoundary, this->boundarySegments);

	// Add walls
	std::vector<ECS::Entity*> walls;
	ECS::Manager::getInstance()->getAllEntitiesInPool(walls, "WALL");

	for (auto box : walls)
	{
		auto comp = box->getComponent<ECS::Sprite>();
		auto bb = comp->sprite->getBoundingBox();

		this->wallUniquePoints.push_back(cocos2d::Vec2(bb.getMinX(), bb.getMaxY()));  //top left
		this->wallUniquePoints.push_back(cocos2d::Vec2(bb.getMaxX(), bb.getMaxY()));  //top right
		this->wallUniquePoints.push_back(cocos2d::Vec2(bb.getMinX(), bb.getMinY()));  //bottom left
		this->wallUniquePoints.push_back(cocos2d::Vec2(bb.getMaxX(), bb.getMinY()));  //bottom right

		this->loadRect(bb, this->wallSegments);
	}

}

void VisibilityScene::loadRect(const cocos2d::Rect& rect, std::vector<Segment*>& segments)
{
	// Going clock wise from top left point, first point is begin and second point is not

	cocos2d::Vec2 topLeft = cocos2d::Vec2(rect.getMinX(), rect.getMaxY());
	cocos2d::Vec2 topRight = cocos2d::Vec2(rect.getMaxX(), rect.getMaxY());
	cocos2d::Vec2 bottomLeft = cocos2d::Vec2(rect.getMinX(), rect.getMinY());
	cocos2d::Vec2 bottomRight = cocos2d::Vec2(rect.getMaxX(), rect.getMinY());

	// Top segment
	this->addSegment(topLeft, topRight, segments);

	// right segment
	this->addSegment(topRight, bottomRight, segments);

	// bottom segment
	this->addSegment(bottomRight, bottomLeft, segments);

	// Left segment
	this->addSegment(bottomLeft, topLeft, segments);
}

void VisibilityScene::addSegment(const cocos2d::Vec2& p1, const cocos2d::Vec2& p2, std::vector<Segment*>& segments)
{
	Segment* segment = new Segment();

	segment->p1 = p1;
	segment->p2 = p2;

	segments.push_back(segment);

	//cocos2d::log("Added segment p1: (%f, %f), p2: (%f, %f)", p1.x, p1.y, p2.x, p2.y);
}

float VisibilityScene::getIntersectingPoint(const cocos2d::Vec2 & rayStart, const cocos2d::Vec2 & rayEnd, const Segment * segment, cocos2d::Vec2 & intersection)
{
	auto& rs = rayStart;
	auto& re = rayEnd;

	auto& sp1 = segment->p1;
	auto& sp2 = segment->p2;

	auto rd = re - rs;
	auto sd = sp2 - sp1;

	auto rsd = sp1 - rs;

	auto rdxsd = rd.cross(sd);
	auto rsdxrd = rsd.cross(rd);

	float t = rsd.cross(sd) / rdxsd;
	float u = rsd.cross(rd) / rdxsd;

	//cocos2d::log("u = %f", u);
	//cocos2d::log("t = %f", t);

	if (rdxsd == 0)
	{
		/*
		if (rsdxrd == 0)
		{
			cocos2d::log("colinear");
		}
		else
		{
			cocos2d::log("Parallel and not intersecting");
		}
		*/

		intersection = cocos2d::Vec2::ZERO;
		return -1;
	}
	else
	{
		if (0 <= t &&  0 <= u && u <= 1.0f)
		{
			//cocos2d::log("Intersecting");
			intersection.x = rayStart.x + (t * rd.x);
			intersection.y = rayStart.y + (t * rd.y);
			return t;
		}
	}

	//cocos2d::log("Not colinear nor parallel, but doesn't intersect");
	return 0;
}

void VisibilityScene::findIntersectsWithRaycasts()
{
	if (this->lightPositions.empty())
	{
		return;
	}

	//auto lightPos = this->lightPositions.at(0);
	auto lightPos = mousePos;

	if (this->displayBoundary.containsPoint(lightPos))
	{
		// load the map and generate segments
		this->loadMap();

		// Raycast once for boundary and once for walls
		// First, check with walls.
		// key point is that we skip segment that has unique point, because we just want to check if ray hits other segments.

		// clear intersecting points
		intersects.clear();

		// Iterate through wall unique point and calculate angles
		std::vector<float> wallUniqueAngles;
		for (auto uniquePoint : this->wallUniquePoints)
		{
			float angle = atan2(uniquePoint.y - lightPos.y, uniquePoint.x - lightPos.x);
			wallUniqueAngles.push_back(angle);
			//wallUniqueAngles.push_back(angle - 0.0001f);
			//wallUniqueAngles.push_back(angle + 0.0001f);
		}

		// Iterate through boundary unique point and calculate angles
		std::vector<float> boundaryUniqueAngles;
		for (auto uniquePoint : this->boundaryUniquePoints)
		{
			float angle = atan2(uniquePoint.y - lightPos.y, uniquePoint.x - lightPos.x);
			boundaryUniqueAngles.push_back(angle);
			//boundaryUniqueAngles.push_back(angle - 0.0001f);
			//boundaryUniqueAngles.push_back(angle + 0.0001f);
		}

		// Now iterate the angle we calculated above, create ray and see if hits any segments
		std::vector<Segment*> allSegments;
		for (auto s : this->boundarySegments)
		{
			allSegments.push_back(s);
		}

		for (auto s : this->wallSegments)
		{
			allSegments.push_back(s);
		}

		int wallIndex = 0;	// for unique point
		int indexCounter = 0;
		for (auto angle : wallUniqueAngles)
		{
			int hitCount = 0;	// increment everytime ray hits segment

			float degree = angle * 180.0f / M_PI;
			//cocos2d::log("angle = %f", degree);
			if (45.0f == fabsf(degree))
			{
				//cocos2d::log("45 degree");
			}
			if (135.0f == fabsf(degree))
			{
				//cocos2d::log("135 degree");
			}
			float dx = cosf(angle);
			float dy = sinf(angle);

			auto rayStart = lightPos;
			auto rayEnd = cocos2d::Vec2(rayStart.x + dx, rayStart.y + dy);

			// Zero will mean no hit
			cocos2d::Vec2 closestIntersection = cocos2d::Vec2::ZERO;
			cocos2d::Vec2 farthestIntersection = cocos2d::Vec2::ZERO;

			bool parallel = false;

			for (auto segment : allSegments)
			{
				auto p1 = cocos2d::Vec2(segment->p1.x, segment->p1.y);
				auto p2 = cocos2d::Vec2(segment->p2.x, segment->p2.y);

				if (p1 == this->wallUniquePoints.at(wallIndex) || p2 == this->wallUniquePoints.at(wallIndex))
				{
					// Don't check segment that includes the unique point we are raycasting.
					continue;
				}

				cocos2d::Vec2 intersectingPoint;
				auto dist = this->getIntersectingPoint(rayStart, rayEnd, segment, intersectingPoint);

				if (dist > 0)
				{
					// count
					hitCount++;

					if (closestIntersection == cocos2d::Vec2::ZERO)
					{
						// Haven't find any intersection yet. Set as closest
						closestIntersection = intersectingPoint;
						farthestIntersection = intersectingPoint;
					}
					else
					{
						// Check if new intersecting point we found is closer to light position(where ray starts)
						if (intersectingPoint.distance(rayStart) < closestIntersection.distance(rayStart))
						{
							// If so, set as closest
							closestIntersection = intersectingPoint;
						}

						if (intersectingPoint.distance(rayStart) > farthestIntersection.distance(rayStart))
						{
							farthestIntersection = intersectingPoint;
						}
					}
				}
				// else if dist == 0, didn't intersect
				else if (dist < 0)
				{
					// it's parallel
					parallel = true;
					//cocos2d::log("Parallel");
				}
				// else, didn't hit this segment
			}
			// end of segment interation

			if (hitCount == 1)
			{
				// Only hit once, which must be boundary.
				// This also means that unique point is visiable from light.
				// We are using intersection with boundary instead of unique point because
				// Light doesn't stop where it intersects with wall. It goes until it hits the end (boundary).
				Vertex v;
				v.boundaryVisible = true;
				v.isBounday = false;
				v.vertex = this->wallUniquePoints.at(wallIndex);
				v.boundaryVertex = closestIntersection;
				v.angle = angle;
				intersects.push_back(v);
			}
			else
			{
				if (hitCount > 1)
				{
					// Check if closest intersecting point is closer than unique point
					auto dist = closestIntersection.distance(rayStart);
					auto maxDist = wallUniquePoints.at(wallIndex).distance(rayStart);
					if (dist > maxDist)
					{
						// Closest intersecting point is further than unique point from light position. 
						// Add unique point as intersecting point.
						Vertex v;
						// in this case, it's not boundary
						v.boundaryVisible = false;
						v.isBounday = false;
						v.vertex = wallUniquePoints.at(wallIndex);
						if (degree == 45.0f || degree == 135.0f || degree == -45.0f || degree == -135.0f)
						{
							//cocos2d::log("Degree 45, adding (%f, %f)", wallUniquePoints.at(wallIndex).x, wallUniquePoints.at(wallIndex).y);
						}
						//v.vertex = closestIntersection;
						if (parallel)
						{
							v.boundaryVisible = true;
							v.boundaryVertex = farthestIntersection;
						}
						else
						{
							if (hitCount % 2 == 0)
							{
								v.boundaryVisible = false;
								v.boundaryVertex = cocos2d::Vec2::ZERO;
							}
							else
							{
								if (degree == 45.0f || degree == 135.0f || degree == -45.0f || degree == -135.0f)
								{
									// Corner cases. Unlike other ray, if angle between segment and ray is 45, 135, -45, -135, treat as single hit
									v.boundaryVisible = false;
									v.boundaryVertex = cocos2d::Vec2::ZERO;
								}
								else
								{
									v.boundaryVisible = true;
									v.boundaryVertex = closestIntersection;
								}
							}
						}
						v.angle = angle;
						intersects.push_back(v);
					}
				}
				// If it's 0, it's bug.
			}

			wallIndex++;
		}

		int boundaryIndex = 0;
		for (auto angle : boundaryUniqueAngles)
		{
			float dx = cosf(angle);
			float dy = sinf(angle);

			auto rayStart = lightPos;
			auto rayEnd = cocos2d::Vec2(rayStart.x + dx, rayStart.y + dy);

			// Zero will mean no hit
			cocos2d::Vec2 closestIntersection = cocos2d::Vec2::ZERO;

			for (auto segment : allSegments)
			{
				auto p1 = cocos2d::Vec2(segment->p1.x, segment->p1.y);
				auto p2 = cocos2d::Vec2(segment->p2.x, segment->p2.y);

				if (p1 == this->boundaryUniquePoints.at(boundaryIndex) || p2 == this->boundaryUniquePoints.at(boundaryIndex))
				{
					// Don't check segment that includes the unique point we are raycasting.
					continue;
				}

				cocos2d::Vec2 intersectingPoint;
				auto dist = this->getIntersectingPoint(rayStart, rayEnd, segment, intersectingPoint);

				if (dist != 0)
				{
					if (closestIntersection == cocos2d::Vec2::ZERO)
					{
						// Haven't find any intersection yet. Set as closest
						closestIntersection = intersectingPoint;
					}
					else
					{
						// Check if new intersecting point we found is closer to light position(where ray starts)
						if (intersectingPoint.distance(rayStart) < closestIntersection.distance(rayStart))
						{
							// If so, set as closest
							closestIntersection = intersectingPoint;
						}
					}
				}
			}

			if (closestIntersection == cocos2d::Vec2::ZERO)
			{
				// Ray didn't hit any segment. Closest intersecting point will be the unique point
				Vertex v;
				v.boundaryVisible = false;
				v.isBounday = true;
				v.vertex = boundaryUniquePoints.at(boundaryIndex);
				v.boundaryVertex = cocos2d::Vec2::ZERO;
				v.angle = angle;
				intersects.push_back(v);
			}
			// Else, ray hit segment. We ignore

			boundaryIndex++;
		}

		this->raycastDrawNode->clear();

		for (auto intersect : intersects)
		{
			float degree = intersect.angle * 180.0f / M_PI;
			auto color = cocos2d::Color4F::RED;
			color.a = 0.5f;
			if (intersect.boundaryVisible)
			{
				this->raycastDrawNode->drawLine(lightPos, intersect.boundaryVertex, color);
			}
			else
			{
				this->raycastDrawNode->drawLine(lightPos, intersect.vertex, color);
			}
		}
	}
}

void VisibilityScene::drawTriangles()
{
	this->triangleDrawNode->clear();
	std::vector<cocos2d::Vec2> verticies;

	auto centerPos = this->mousePos;

	// Sort intersecting points by angle.
	std::sort(this->intersects.begin(), this->intersects.end(), VertexComparator());

	// Generate vertex for triangle. 
	auto size = this->intersects.size();
	bool prevEndedWithBoundayVisible = false;
	for (unsigned int i = 0; i < size; i++)
	{
		// First, add center point
		verticies.push_back(centerPos);		
		
		int index = i + 1;
		if (index == size)
		{
			index = 0;
		}

		auto& v2 = intersects.at(i);
		auto& v3 = intersects.at(index);
		verticies.push_back(v2.vertex);
		verticies.push_back(v3.vertex);
		/*
		if (v2.isBounday)
		{
			if (v3.isBounday)
			{
				verticies.push_back(v2.vertex);
				verticies.push_back(v3.vertex);
			}
			else
			{
				if (v3.boundaryVisible)
				{
					verticies.push_back(intersects.at(i).vertex);
					verticies.push_back(intersects.at(index).boundaryVertex);
				}
				else
				{
					// can't reach?
					verticies.push_back(v2.vertex);
					verticies.push_back(v3.vertex);
				}
			}
		}
		else
		{
			if(v3.isBounday)
			{
				verticies.push_back(v2.boundaryVertex);
				verticies.push_back(v3.vertex);
			}
			else
			{
				if (v3.boundaryVisible)
				{
					if (v2.boundaryVisible)
					{
						if (prevEndedWithBoundayVisible)
						{
							verticies.push_back(v2.boundaryVertex);
							verticies.push_back(v3.boundaryVertex);
						}
						else
						{
							verticies.push_back(v2.vertex);
							verticies.push_back(v3.vertex);
						}
					}
					else
					{
						verticies.push_back(v2.vertex);
						verticies.push_back(v3.vertex);
					}
				}
				else
				{
					verticies.push_back(v2.vertex);
					verticies.push_back(v3.vertex);
				}
			}
		}

		if (v3.boundaryVisible)
		{
			//prevEndedWithBoundayVisible = true;
		}
		else
		{
			//prevEndedWithBoundayVisible = false;
		}
		*/
	}

	size = verticies.size();
	auto color = cocos2d::Color4F::GREEN;
	color.a = 0.2f;
	for (unsigned int i = 0; i < size; i+=3)
	{
		this->triangleDrawNode->drawTriangle(verticies.at(i), verticies.at(i + 1), verticies.at(i + 2), color);
	}
}

bool VisibilityScene::isPointOnBoundary(const cocos2d::Vec2 & point)
{
	if (point.x == this->displayBoundary.getMinX() || point.x == this->displayBoundary.getMaxX() || 
		point.y == this->displayBoundary.getMinY() || point.y == this->displayBoundary.getMinY())
	{
		return true;
	}
	else
	{
		return false;
	}
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

	this->mousePos = point;

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

				//float fx = floorf(point.x);
				//float fy = floorf(point.y);
				//this->newBoxDest = cocos2d::Vec2(fx, fy);
				this->newBoxDest = point;

				this->newBoxDrawNode->clear();
				this->newBoxDrawNode->drawSolidRect(this->newBoxOrigin, this->newBoxDest, cocos2d::Color4F::ORANGE);
				this->newBoxDrawNode->drawRect(this->newBoxOrigin, this->newBoxDest, cocos2d::Color4F::YELLOW);
			}
		}

		this->findIntersectsWithRaycasts();
		this->drawTriangles();
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
			auto boxCount = m->getAliveEntityCountInEntityPool("WALL");
			if (boxCount < 32)
			{
				// still can make another box
				this->draggingBox = true;
				//float fx = floorf(point.x);
				//float fy = floorf(point.y);
				//this->newBoxOrigin = cocos2d::Vec2(fx, fy);
				this->newBoxOrigin = point;
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
				cocos2d::log("Creating new box origin (%f, %f), size (%f, %f)", this->newBoxOrigin.x, this->newBoxOrigin.y, this->newBoxDest.x, this->newBoxDest.y);

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
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_SPACE)
	{
		this->findIntersectsWithRaycasts();
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_1)
	{
		this->visiableAreaDrawNode->clear();
		this->drawTriangles();
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

	for (auto segment : this->wallSegments)
	{
		delete segment;
	}
}