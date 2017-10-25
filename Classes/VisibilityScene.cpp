#include "VisibilityScene.h"
#include "MainScene.h"
#include "Utility.h"

USING_NS_CC;

int VisibilityScene::wallIDCounter = 0;

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

	// create shader
	//cocos2d::ShaderCache::getInstance()->purgeSharedShaderCache();
	this->floorShader = cocos2d::GLProgram::createWithFilenames("shaders/light_vert.glsl", "shaders/light_frag.glsl");

	this->scheduleUpdate();

	// Use for freeform walls(polygon)
	earClipping = EarClippingScene::createScene();
	earClipping->retain();

	ECS::Manager::getInstance();

	// init display boundary box node which draws outer line of simulation display box
	this->displayBoundaryBoxNode = DisplayBoundaryBoxNode::createNode();
	this->displayBoundaryBoxNode->setPosition(cocos2d::Vec2::ZERO);
	this->displayBoundaryBoxNode->displayBoundary = cocos2d::Rect(0, 0, 650, 650);
	this->displayBoundaryBoxNode->drawDisplayBoundaryBox();
	this->displayBoundary = this->displayBoundaryBoxNode->displayBoundary;
	this->displayBoundaryBoxNode->retain();
	this->displayBoundaryBoxNode->drawNode->setLocalZOrder(static_cast<int>(Z_ORDER::BOX));
	this->addChild(this->displayBoundaryBoxNode, Z_ORDER::WALL);

	// Instead of creating 650 x 650 sprite (waste of spritesheet), create black square texture sized 650.
	auto rt = cocos2d::RenderTexture::create(650, 650);
	rt->beginWithClear(0, 0, 0, 0);
	rt->end();

	// Create sprite with above texture and attach shader.
	this->floorSprite = cocos2d::Sprite::createWithTexture(rt->getSprite()->getTexture());
	this->floorSprite->setPosition(cocos2d::Vec2(this->displayBoundary.getMidX(), this->displayBoundary.getMidY()));
	this->addChild(this->floorSprite, Z_ORDER::FLOOR);

	this->floorShaderState = cocos2d::GLProgramState::getOrCreateWithGLProgram(this->floorShader);
	this->floorShaderState->setUniformInt("lightSize", 0);

	//this->floorSprite->setGLProgram(this->floorShader);

	this->floorSprite->setGLProgramState(this->floorShaderState);

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
	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Walls: 0 / 20", customLabelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::CUSTOM, "Lights: 0 / 15", customLabelSize);

	// Calculate next label block y
	const float customLastY = this->labelsNode->customLabels.back()->getBoundingBox().getMinY();
	const float blockGap = 22.0f;

	this->labelsNode->keyboardUsageLabelStartPos = cocos2d::Vec2(labelX, customLastY - blockGap);

	const int headerSize = 25;
	const int labelSize = 20;

	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Keys (Green = enabled)", headerSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "L: Cursor light", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "D (While cursor light enabled): Debug draw", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "Y (for debug): Toggle raycast draw", labelSize);
	this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::TOGGLE_RAYCAST), cocos2d::Color3B::GREEN);
	this->labelsNode->addLabel(LabelsNode::TYPE::KEYBOARD, "T (for debug): Toggle raycast draw", labelSize);
	this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::TOGGLE_TRIANGLE), cocos2d::Color3B::GREEN);


	const float keysLastY = this->labelsNode->keyboardUsageLabels.back()->getBoundingBox().getMinY();
	this->labelsNode->mouseUsageLabelStartPos = cocos2d::Vec2(labelX, keysLastY - blockGap);

	this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Mouse", headerSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Left Click Drag (In box): Draw rectangle wall", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Left Click (In box): Draw freeform wall", labelSize);
	this->labelsNode->addLabel(LabelsNode::TYPE::MOUSE, "Right Click (In box)): Add light", labelSize);

	this->currentMode = MODE::IDLE;
	this->draggingBox = false;
	this->newBoxOrigin = cocos2d::Vec2::ZERO;
	this->newBoxDest = cocos2d::Vec2::ZERO;
	this->dragBoxDrawNode = cocos2d::DrawNode::create();
	this->addChild(this->dragBoxDrawNode, Z_ORDER::DRAG);

	this->raycastDrawNode = cocos2d::DrawNode::create();
	this->addChild(this->raycastDrawNode, Z_ORDER::RAYCAST);
	this->triangleDrawNode = cocos2d::DrawNode::create();
	this->addChild(this->triangleDrawNode, Z_ORDER::TRIANGLE);
	this->freeformWallDrawNode = cocos2d::DrawNode::create();
	this->addChild(this->freeformWallDrawNode, Z_ORDER::FREEFORM);
	this->wallDrawNode = cocos2d::DrawNode::create();
	this->addChild(this->wallDrawNode, Z_ORDER::WALL);

	this->mousePos = cocos2d::Vec2::ZERO;

	this->hoveringWallIndex = -1;

	this->viewRaycast = true;
	this->viewVisibleArea = true;
	this->cursorLight = false;
	this->debugMode = false;
	this->viewLightMap = false;
	this->needToUpdateUniform = false;

	this->mousePosDirty = false;

	VisibilityScene::wallIDCounter = 0;

	this->lightMapTexture = nullptr;

	for (int i = 0; i < maxLightCount; i++)
	{
		lightPositions.push_back(cocos2d::Vec2::ZERO);
		lightColors.push_back(cocos2d::Vec3::ONE);
		lightIntensities.push_back(0);
	}

	this->uniformData = std::vector<float>(16 * 6, 0);
	this->activeLightSize = 0;

	int w = static_cast<int>(this->displayBoundary.size.width);
	int h = static_cast<int>(this->displayBoundary.size.height);
	this->lightMapRenderTexture = cocos2d::RenderTexture::create(w, h);
	this->lightMapRenderTexture->retain();

	return true;
}

void VisibilityScene::onEnter()
{
	cocos2d::Scene::onEnter();

	initECS();

	initInputListeners();

	this->initMap();
}

void VisibilityScene::initECS()
{
	auto m = ECS::Manager::getInstance();
	m->createEntityPool("LIGHT", maxLightCount);

	this->cursorLightEntity = createNewLight(this->mousePos);
	auto comp = this->cursorLightEntity->getComponent<ECS::LightData>();
	comp->active = false;
	comp->intensity = 200.0f;
	comp->color = cocos2d::Vec3(1, 1, 1);
}

bool VisibilityScene::createNewRectWall()
{
	cocos2d::Vec2 newWallSize = this->newBoxDest - this->newBoxOrigin;
	if (fabsf(newWallSize.x) < minRectSize || fabsf(newWallSize.y) < minRectSize)
	{
		this->clearDrag();
		return false;
	}

	auto maxX = this->newBoxOrigin.x > this->newBoxDest.x ? this->newBoxOrigin.x : this->newBoxDest.x;
	auto minX = this->newBoxOrigin.x < this->newBoxDest.x ? this->newBoxOrigin.x : this->newBoxDest.x;
	auto maxY = this->newBoxOrigin.y > this->newBoxDest.y ? this->newBoxOrigin.y : this->newBoxDest.y;
	auto minY = this->newBoxOrigin.y < this->newBoxDest.y ? this->newBoxOrigin.y : this->newBoxDest.y;

	cocos2d::Rect rect = cocos2d::Rect(cocos2d::Vec2(minX, minY), cocos2d::Size(maxX - minX, maxY - minY));

	Wall wall;
	wall.angle = 0;
	wall.center = cocos2d::Vec2(rect.getMidX(), rect.getMidY());
	wall.points.push_back(cocos2d::Vec2(rect.getMinX(), rect.getMaxY()));  //top left
	wall.points.push_back(cocos2d::Vec2(rect.getMaxX(), rect.getMaxY()));  //top right
	wall.points.push_back(cocos2d::Vec2(rect.getMaxX(), rect.getMinY()));  //bottom right
	wall.points.push_back(cocos2d::Vec2(rect.getMinX(), rect.getMinY()));  //bottom left
	wall.wallID = VisibilityScene::wallIDCounter++;
	wall.bb = rect;
	wall.rectangle = true;

	this->walls.push_back(wall);

	this->drawWalls();

	return true;
}

bool VisibilityScene::createNewFreeformWall()
{
	if (this->freeformWallPoints.size() < 2)
	{
		return false;
	}

	Wall wall;
	wall.angle = 0;

	// 5000 is big enough to detect min values
	float maxX = 0;
	float minX = 5000.0f;
	float maxY = 0;
	float minY = 5000.0f;

	for (auto point : this->freeformWallPoints)
	{
		wall.points.push_back(point);

		if (point.x > maxX)
		{
			maxX = point.x;
		}
		else if (point.x < minX)
		{
			minX = point.x;
		}

		if (point.y > maxY)
		{
			maxY = point.y;
		}
		else if (point.y < minY)
		{
			minY = point.y;
		}
	}

	cocos2d::Rect rect = cocos2d::Rect(cocos2d::Vec2(minX, minY), cocos2d::Size(maxX - minX, maxY - minY));

	wall.bb = rect;
	wall.center = cocos2d::Vec2(rect.getMidX(), rect.getMidY());
	wall.rectangle = false;

	wall.wallID = VisibilityScene::wallIDCounter++;

	this->walls.push_back(wall);

	this->drawWalls();

	return true;
}

ECS::Entity* VisibilityScene::createNewLight(const cocos2d::Vec2& position)
{
	auto m = ECS::Manager::getInstance();
	auto newLight = m->createEntity("LIGHT");

	if (newLight)
	{
		auto lightComp = m->createComponent<ECS::LightData>();
		lightComp->position = position;

		cocos2d::Vec3 randColor = cocos2d::Vec3(
			Utility::Random::randomReal<float>(0.5f, 1.0f),
			Utility::Random::randomReal<float>(0.5f, 1.0f),
			Utility::Random::randomReal<float>(0.5f, 1.0f));
		lightComp->color = randColor;

		int w = static_cast<int>(this->displayBoundary.size.width);
		int h = static_cast<int>(this->displayBoundary.size.height);

		lightComp->renderTexture = cocos2d::RenderTexture::create(w, h);
		lightComp->renderTexture->retain();
		
		newLight->addComponent<ECS::LightData>(lightComp);
	}
	else
	{
		newLight = nullptr;
	}

	return newLight;
}

void VisibilityScene::initMap()
{
	// initialize boundary unique points and segment because boundary doesnt change.
	this->wallUniquePoints.clear();

	//// Add boundary
	this->wallUniquePoints.push_back({ BOUNDARY_WALL_ID, cocos2d::Vec2(this->displayBoundary.getMinX(), this->displayBoundary.getMaxY()) });  //top left
	this->wallUniquePoints.push_back({ BOUNDARY_WALL_ID, cocos2d::Vec2(this->displayBoundary.getMaxX(), this->displayBoundary.getMaxY()) });  //top right
	this->wallUniquePoints.push_back({ BOUNDARY_WALL_ID, cocos2d::Vec2(this->displayBoundary.getMinX(), this->displayBoundary.getMinY()) });  //bottom left
	this->wallUniquePoints.push_back({ BOUNDARY_WALL_ID, cocos2d::Vec2(this->displayBoundary.getMaxX(), this->displayBoundary.getMinY()) });  //bottom right

	this->loadRect(this->displayBoundary, this->wallSegments, BOUNDARY_WALL_ID);
}

void VisibilityScene::reloadMap()
{
	// clear all
	this->wallUniquePoints.resize(4);

	if (this->wallSegments.size() > 4)
	{
		auto it = this->wallSegments.begin();
		std::advance(it, 4);
		for (; it != this->wallSegments.end(); it++)
		{
			if (*it) delete *it;
		}

		this->wallSegments.resize(4);
	}

	// Add walls
	for (auto wall : walls)
	{
		this->addWall(wall);
	}
}

void VisibilityScene::addWall(Wall & wall)
{
	if (wall.rectangle)
	{
		auto& bb = wall.bb;

		this->wallUniquePoints.push_back({ wall.wallID, cocos2d::Vec2(bb.getMinX(), bb.getMaxY()) });  //top left
		this->wallUniquePoints.push_back({ wall.wallID, cocos2d::Vec2(bb.getMaxX(), bb.getMaxY()) });  //top right
		this->wallUniquePoints.push_back({ wall.wallID, cocos2d::Vec2(bb.getMinX(), bb.getMinY()) });  //bottom left
		this->wallUniquePoints.push_back({ wall.wallID, cocos2d::Vec2(bb.getMaxX(), bb.getMinY()) });  //bottom right

		this->loadRect(bb, this->wallSegments, wall.wallID);
	}
	else
	{
		auto size = wall.points.size();
		for (unsigned int i = 0; i < size; i++)
		{
			this->wallUniquePoints.push_back({ wall.wallID, wall.points.at(i) });
		}

		this->loadFreeform(wall.points, this->wallSegments, wall.wallID);
	}
}

void VisibilityScene::loadRect(const cocos2d::Rect& rect, std::vector<Segment*>& segments, const int wallID)
{
	// Going clock wise from top left point, first point is begin and second point is not

	cocos2d::Vec2 topLeft = cocos2d::Vec2(rect.getMinX(), rect.getMaxY());
	cocos2d::Vec2 topRight = cocos2d::Vec2(rect.getMaxX(), rect.getMaxY());
	cocos2d::Vec2 bottomLeft = cocos2d::Vec2(rect.getMinX(), rect.getMinY());
	cocos2d::Vec2 bottomRight = cocos2d::Vec2(rect.getMaxX(), rect.getMinY());

	// Top segment
	this->addSegment(topLeft, topRight, segments, wallID);

	// right segment
	this->addSegment(topRight, bottomRight, segments, wallID);

	// bottom segment
	this->addSegment(bottomRight, bottomLeft, segments, wallID);

	// Left segment
	this->addSegment(bottomLeft, topLeft, segments, wallID);
}

void VisibilityScene::loadFreeform(const std::vector<cocos2d::Vec2>& points, std::vector<Segment*>& segments, const int wallID)
{
	auto size = points.size();
	for (unsigned int i = 0; i < size; i++)
	{
		if (i < size - 1)
		{
			this->addSegment(points.at(i), points.at(i + 1), segments, wallID);
		}
	}

	if (size > 1)
	{
		this->addSegment(points.at(0), points.at(size - 1), segments, wallID);
	}
}

void VisibilityScene::addSegment(const cocos2d::Vec2& p1, const cocos2d::Vec2& p2, std::vector<Segment*>& segments, const int wallID)
{
	Segment* segment = new Segment();

	segment->p1 = p1;
	segment->p2 = p2;

	segment->wallID = wallID;

	segments.push_back(segment);
}

float VisibilityScene::getIntersectingPoint(const cocos2d::Vec2 & rayStart, const cocos2d::Vec2 & rayEnd, const Segment * segment, cocos2d::Vec2 & intersection)
{
	auto& rs = rayStart;
	auto& re = rayEnd;

	auto& sp1 = segment->p1;
	auto& sp2 = segment->p2;

	auto rd = re - rs;
	auto sd = sp2 - sp1;

	auto rdDotsd = rd.dot(sd);

	auto rsd = sp1 - rs;

	auto rdxsd = rd.cross(sd);		//denom
	auto rsdxrd = rsd.cross(rd);

	float t = rsd.cross(sd) / rdxsd;
	float u = rsd.cross(rd) / rdxsd;

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
		if (u > 1.0f && u - 1.0f < 0.00001f)
		{
			u = 1.0f;
		}
		if (0 <= t &&  0 <= u && u <= 1.0f)
		{
			intersection.x = rayStart.x + (t * rd.x);
			intersection.y = rayStart.y + (t * rd.y);
			intersection.x = sp1.x + (u * sd.x);
			intersection.y = sp1.y + (u * sd.y);
			return t;
		}
	}
	return 0;
}

cocos2d::Vec2 VisibilityScene::getIntersectingPoint(const cocos2d::Vec2 & p1, const cocos2d::Vec2 & p2, const cocos2d::Vec2 & p3, const cocos2d::Vec2 & p4)
{
	float s = ((p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x))
		/ ((p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y));
	return cocos2d::Vec2(p1.x + s * (p2.x - p1.x), p1.y + s * (p2.y - p1.y));
}

bool VisibilityScene::getIntersectingPoint(const cocos2d::Vec2 & rayStart, const cocos2d::Vec2 & rayEnd, const Segment * segment, Hit & hit)
{
	auto& rs = rayStart;
	auto& re = rayEnd;

	auto& sp1 = segment->p1;
	auto& sp2 = segment->p2;

	auto rd = re - rs;
	auto sd = sp2 - sp1;

	auto rdDotsd = rd.dot(sd);

	auto rsd = sp1 - rs;

	auto rdxsd = rd.cross(sd);		//denom
	auto rsdxrd = rsd.cross(rd);

	float t = rsd.cross(sd) / rdxsd;
	float u = rsd.cross(rd) / rdxsd;

	hit.t = 0;
	hit.u = 0;
	hit.hitPoint = cocos2d::Vec2::ZERO;
	hit.parallel = false;
	hit.perpendicular = (rdDotsd == 0);

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
		hit.parallel = true;
		return false;
	}
	else
	{
		if (u > 1.0f && u - 1.0f < 0.00001f)
		{
			u = 1.0f;
		}
		if (0 <= t && 0 <= u && u <= 1.0f)
		{
			hit.t = t;
			hit.u = u;
			hit.hitPoint.x = sp1.x + (u * sd.x);
			hit.hitPoint.y = sp1.y + (u * sd.y);
			return true;
		}
	}

	return false;
}

void VisibilityScene::findIntersectsWithRaycasts(const cocos2d::Vec2& lightPos)
{
	// Don't need to find if there is no light
	if (this->displayBoundary.containsPoint(lightPos))
	{
		// clear intersecting points.
		intersects.clear();

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

		// Eventhough we use both boudary and wall segment for raycasting, we need separate 
		// list of unique points to determine if we really need the intersecting point.
		// More details on below comments

		// Iterate through walls first
		for (auto uniquePoint : this->wallUniquePoints)
		{
			//int hitCount = 0;	// increment everytime ray hits segment

			// Get angle from light position and unique point so we can create ray to that direction
			float angle = atan2(uniquePoint.point.y - lightPos.y, uniquePoint.point.x - lightPos.x);

			// Get direction of ray
			float dx = cosf(angle);
			float dy = sinf(angle);

			// Generate raycast vec2 points
			auto rayStart = lightPos;
			auto rayEnd = cocos2d::Vec2(rayStart.x + dx, rayStart.y + dy);

			// Zero will mean no hit
			cocos2d::Vec2 closestPoint = cocos2d::Vec2::ZERO;
			cocos2d::Vec2 secondClosestIntersection = cocos2d::Vec2::ZERO;

			// Keep tracks the wallID of segment that raycast intersected.
			int wallID = -1;
			int uniquePointWallID = uniquePoint.wallID;
			float shortestDist = 5000.0f;	// 5000 is enough distance to be max
			bool skip = false;
			bool perpendicular = false;

			std::unordered_set<int> wallIDSet;

			// Iterate through all segments
			for (auto segment : allSegments)
			{
				auto p1 = cocos2d::Vec2(segment->p1.x, segment->p1.y);
				auto p2 = cocos2d::Vec2(segment->p2.x, segment->p2.y);

				if (p1 == uniquePoint.point || p2 == uniquePoint.point)
				{
					// If either one of the end points of segments is unique point, we don't have to check raycast
					// because it's 100% hit.
					continue;
				}

				// Get intersecting point
				Hit hit;
				bool result = this->getIntersectingPoint(rayStart, rayEnd, segment, hit);

				if (result)
				{
					// if we are raycasting to boundary wall's unique point, any hit means unique point isn't visible
					if (uniquePointWallID == BOUNDARY_WALL_ID)
					{
						skip = true;
						break;
					}
					// Else, unique point is not boundary. keep go on.

					// Ray hit something
					// Check if it hit the edge
					bool edge = hit.u == 0 || hit.u == 1.0f;
					// We don't consider edge point as a closest point. 
					if (edge)
					{
						if (segment->wallID == BOUNDARY_WALL_ID)
						{
							continue;
						}
						else
						{
							if (uniquePointWallID != segment->wallID)
							{
								auto& center = this->walls.at(segment->wallID).center;
								bool segDir = isOnLeft(rayStart, rayEnd, center);	// left = true, right = false
								bool uniquepointSegDir = isOnLeft(rayStart, rayEnd, this->walls.at(uniquePoint.wallID).center);
								if (segDir == uniquepointSegDir)
								{
									continue;
								}
								// Else, record if it's closest.
							}
							// Else, intersecting edge on same wall means hit.
							else
							{
								// Check if it was perpendicular
								if (hit.perpendicular)
								{
									perpendicular = true;
									auto dist = hit.hitPoint.distance(rayStart);
									auto maxDist = uniquePoint.point.distance(rayStart);
									if (dist > maxDist)
									{
										continue;
									}
								}
							}
						}
					}

					// Store wall id of segment that wasn't neither edge nor perpendicular, 
					wallIDSet.insert(segment->wallID);

					// Not the edge.
					float dist = hit.hitPoint.distance(rayStart);
					if (dist < shortestDist)
					{
						shortestDist = dist;
						closestPoint = hit.hitPoint;
						wallID = segment->wallID;
					}
				}
				else
				{
					// Ray didn't hit the segment.
					continue;
				}
			}
			// end of segment interation

			// Check for case where unique point is boundary wall
			if (uniquePoint.wallID == BOUNDARY_WALL_ID)
			{
				// We are raycasting to boundary wall. 
				if (skip)
				{
					// Ray hit other segment before it reached boundary wall. Ignore this ray.
					continue;
				}
				else
				{
					// Ray reached boundary wall without hitting any other segments
					Vertex v;
					v.point = uniquePoint.point;
					v.uniquePoint = uniquePoint.point;
					v.type = Vertex::TYPE::ON_UNIQUE_POINT;	// It's unique point not boundary
					v.angle = angle;
					v.pointWallID = BOUNDARY_WALL_ID;
					v.uniquePointWallID = BOUNDARY_WALL_ID;
					this->intersects.push_back(v);
					continue;
				}
			}
			// Else, unique point is not boundary wall
			
			// Check if ray only hit boundary wall. 
			// Only comparing wall ID to -1 is enough because if wall ID is -1,
			// it might have hit other segments than boundary, but boundary wall segments are 
			// raycasted first than wall segments, which means if wall ID remains -1 
			// then it only hit boundary wall
			if (wallID == BOUNDARY_WALL_ID)
			{
				// Ray only hit boundary wall.
				Vertex v;
				v.point = closestPoint;
				v.uniquePoint = uniquePoint.point;
				v.type = Vertex::TYPE::ON_BOUNDARY;
				v.angle = angle;
				v.pointWallID = BOUNDARY_WALL_ID;
				v.uniquePointWallID = uniquePoint.wallID;
				this->intersects.push_back(v);
			}
			else
			{
				// Ray always hits boundary wall, but hit other segment. 
				// Check if ray hit the segment that has end point from same wall.
				auto find_it = wallIDSet.find(uniquePointWallID);
				if (find_it == wallIDSet.end())
				{
					// Ray didn't hit segment from same wall.
					// This case can be single line wall, or passed the edge (corner case)
					auto dist = closestPoint.distance(rayStart);
					auto maxDist = uniquePoint.point.distance(rayStart);

					if (dist < maxDist)
					{
						// If ray hit the segment before it reached unique point. Ignore
						continue;
					}
					else
					{
						// Ray hit the segment after it reached the unique point. 
						// There are 2 cases.
						// Ray only hit boundary wall. 
						Vertex v;
						v.point = closestPoint;
						v.uniquePoint = uniquePoint.point;
						bool hitOtherWall = this->didRayHitOtherWall(wallIDSet, uniquePointWallID);
						if (hitOtherWall)
						{
							v.type = Vertex::TYPE::ON_WALL;
							v.pointWallID = wallID;
						}
						else
						{
							v.type = Vertex::TYPE::ON_BOUNDARY;
							v.pointWallID = BOUNDARY_WALL_ID;
						}
						v.angle = angle;
						v.uniquePointWallID = uniquePoint.wallID;
						this->intersects.push_back(v);
					}
				}
				else
				{
					// ray hit the segment that has end point from same wall.
					// There is 2 cases here.
					// case 1) ray hit unique point and then hit the segment from same wall.
					//		   In this case, unique point is the closest point
					// case 2) Ray hit segment from same wall and then the unique point.
					//		   In this case, we ignore.
					// We can determine the case by comparing the distance.
					auto dist = closestPoint.distance(rayStart);
					auto maxDist = uniquePoint.point.distance(rayStart);

					if (dist < maxDist)
					{
						// case 2. Ignore
						continue;
					}
					else
					{
						// case 1. 
						// However, there might be an additional the case where ray hit other wall before it reached unique point.
						bool hitOtherWall = this->didRayHitOtherWall(wallIDSet, uniquePointWallID);

						if (hitOtherWall)
						{
							// Even though ray hit other wall, we need to check if wall segment was behind(farther) from unique point or not(Closer)
							if (dist > maxDist)
							{
								// Ray hit other wall's segment after reaching the unique point. 
								// Unique point is the closest point
								Vertex v;
								v.point = uniquePoint.point;
								v.uniquePoint = uniquePoint.point;
								v.type = Vertex::TYPE::ON_UNIQUE_POINT;
								v.angle = angle;
								v.pointWallID = uniquePoint.wallID;
								v.uniquePointWallID = uniquePoint.wallID;
								this->intersects.push_back(v);
							}
							else
							{
								// Ray hit other wall's segment before it reached the unique point. We can ignore this.
								continue;
							}
						}
						else
						{
							// Ray didn't hit any other wall before it reached unique point.
							// After case 1 (ray hit segment from same wall) and didn't hit any other wall segment except boundary wall.
							// So unique point is closest(case 2 is already handled at first)
							Vertex v;

							// However, if the segment was perpendicular (parallel case actually)
							if (perpendicular)
							{
								v.point = uniquePoint.point;
							}
							else
							{
								v.point = uniquePoint.point;
							}
							v.uniquePoint = uniquePoint.point;
							v.type = Vertex::TYPE::ON_UNIQUE_POINT;
							v.angle = angle;
							v.pointWallID = uniquePoint.wallID;
							v.uniquePointWallID = uniquePoint.wallID;
							this->intersects.push_back(v);
						}
					}
				}
			}
		}
	}
}

void VisibilityScene::generateTriangles(const cocos2d::Vec2& lightPos)
{
	auto v1 = lightPos;

	// Generate vertex for triangle. 
	auto size = this->intersects.size();

	this->triangles.clear();

	for (unsigned int i = 0; i < size; i++)
	{
		// First, add center position, which is position of light
		triangles.push_back(v1);

		int index = i + 1;
		if (index == size)
		{
			index = 0;
		}

		auto& v2 = intersects.at(i);
		auto& v3 = intersects.at(index);

		// Intersects are sorted by angle. The order is counter clockwise and starts from third quadrant.
		/*
		*	q1						q0
		*		   4	|    3
		*				|
		*		--------+---------  CCW
		*				|
		*		   1	|    2
		*	q3						q4
		*/

		// The triangle depends on how ray hit the segments and all.

		// Case: v2 is unique point type
		if (v2.type == Vertex::TYPE::ON_UNIQUE_POINT)
		{
			// Case: v2 is boundary wall unique point
			if (v2.uniquePointWallID == BOUNDARY_WALL_ID)
			{
				// Case 1) If v2 is boundary wall unique point, v3 must be closest point
				triangles.push_back(v2.uniquePoint);
				triangles.push_back(v3.point);
			}
			else
			{
				// Case: v3 is boundary type
				if (v3.type == Vertex::TYPE::ON_BOUNDARY)
				{
					// Case 6) 
					triangles.push_back(v2.uniquePoint);
					triangles.push_back(v3.uniquePoint);
				}
				// Case: v3 is unique point type or wall type
				else if (v3.type == Vertex::TYPE::ON_UNIQUE_POINT)
				{
					// Case 7) both are unique point
					triangles.push_back(v2.uniquePoint);
					triangles.push_back(v3.uniquePoint);
				}
				else if (v3.type == Vertex::TYPE::ON_WALL)
				{
					// Case: id is same
					if (v2.uniquePointWallID == v3.uniquePointWallID)
					{
						// Case 8)
						triangles.push_back(v2.uniquePoint);
						triangles.push_back(v3.uniquePoint);
					}
					else
					{
						// Case 14)
						triangles.push_back(v2.uniquePoint);
						triangles.push_back(v3.point);
					}
				}
			}
		}
		// Case: v2 is boundary type
		else if (v2.type == Vertex::TYPE::ON_BOUNDARY)
		{
			// Case: v3 is uniquepoint type
			if (v3.type == Vertex::TYPE::ON_UNIQUE_POINT)
			{
				// Case: v3 is boundary wall unique point
				if (v3.uniquePointWallID == BOUNDARY_WALL_ID)
				{
					// Case 2) if v2 is boundary type and v3 is boundary unique wall, v2 is closest point
					triangles.push_back(v2.point);
					triangles.push_back(v3.uniquePoint);
				}
				// Case: v3 is not boundary wall unique point
				else
				{
					// Case 3) both are unique point
					triangles.push_back(v2.uniquePoint);
					triangles.push_back(v3.uniquePoint);
				}
			}
			// Case: v3 is boundary type
			else if (v3.type == Vertex::TYPE::ON_BOUNDARY)
			{
				// Case: both are boundary type and on same wall
				if (v2.uniquePointWallID == v3.uniquePointWallID)
				{
					// Case 4) both unique point
					triangles.push_back(v2.uniquePoint);
					triangles.push_back(v3.uniquePoint);
				}
				// Case: v2 and v3 from different wall
				else
				{
					// Case 5) between wall.
					triangles.push_back(v2.point);
					triangles.push_back(v3.point);
				}
			}
			// Case: v3 is wall type
			else if (v3.type == Vertex::TYPE::ON_WALL)
			{
				// Case: same wall
				if (v2.uniquePointWallID == v3.uniquePointWallID)
				{
					// Case 15)
					triangles.push_back(v2.uniquePoint);
					triangles.push_back(v3.uniquePoint);
				}
				else
				{
					// Case 10) 
					triangles.push_back(v2.uniquePoint);
					triangles.push_back(v3.point);
				}
			}
		}
		// Case: v2 is wall type
		else if (v2.type == Vertex::TYPE::ON_WALL)
		{
			// Case: v3 is boundary type
			if (v3.type == Vertex::TYPE::ON_BOUNDARY)
			{
				// Case: v2 and v3 is on same wall
				if (v2.uniquePointWallID == v3.uniquePointWallID)
				{
					// Case 9)
					triangles.push_back(v2.uniquePoint);
					triangles.push_back(v3.uniquePoint);
				}
				// Case: v2 and v3 is on different wall
				else
				{
					// Case 13)
					triangles.push_back(v2.point);
					triangles.push_back(v3.uniquePoint);
				}
			}
			// Case: v3 is unique point type
			else if (v3.type == Vertex::TYPE::ON_UNIQUE_POINT)
			{
				if (v2.uniquePointWallID == v3.uniquePointWallID)
				{
					// Case 12)
					triangles.push_back(v2.uniquePoint);
					triangles.push_back(v3.uniquePoint);
				}
				else
				{
					// Case 16)
					triangles.push_back(v2.point);
					triangles.push_back(v3.uniquePoint);
				}
			}
			// Case: v3 is wall type
			else if (v3.type == Vertex::TYPE::ON_WALL)
			{
				if (v2.uniquePointWallID == v3.uniquePointWallID)
				{
					// Case 11) v2 and v3 must be on same wall
					triangles.push_back(v2.uniquePoint);
					triangles.push_back(v3.uniquePoint);
				}
				else
				{
					if (v2.pointWallID == v3.pointWallID)
					{
						// Case 17) v2 and v3 is not on same wall but ended up on same wall
						triangles.push_back(v2.point);
						triangles.push_back(v3.point);
					}
					else
					{
						if (v2.pointWallID == v3.uniquePointWallID)
						{
							// Case 18)
							triangles.push_back(v2.point);
							triangles.push_back(v3.uniquePoint);
						}
						else
						{
							// Case 19)
							triangles.push_back(v2.uniquePoint);
							triangles.push_back(v3.point);
						}
					}
				}
			}
		}
	}
}

void VisibilityScene::drawTriangles()
{
	if (this->viewVisibleArea == false) return;

	this->triangleDrawNode->clear();

	auto size = this->triangles.size();
	auto color = cocos2d::Color4F::WHITE;
	color.a = 0.8f;
	for (unsigned int i = 0; i < size; i+=3)
	{
		try
		{
			this->triangleDrawNode->drawTriangle(this->triangles.at(i), 
												this->triangles.at(i + 1), 
												this->triangles.at(i + 2), 
												color);
		}
		catch (...)
		{
			cocos2d::log("Error: There wasn't enough verticies to form triangle. generateTriangles() must be bugged.");
		}
	}
}

bool VisibilityScene::isPointInWall(const cocos2d::Vec2 & point)
{
	for (auto wall : this->walls)
	{
		if (wall.rectangle)
		{
			if (wall.bb.containsPoint(point))
			{
				return true;
			}
		}
		else
		{
			auto size = wall.points.size();
			if (size >= 3)
			{
				std::list<cocos2d::Vec2> pointList(wall.points.begin(), wall.points.end());

				bool inPolygon = Utility::Polygon::isPointInPolygon(pointList, point);

				if (inPolygon)
				{
					return true;
				}
			}
			else
			{
				// it's a line
				continue;
			}
		}
	}

	return false;
}

void VisibilityScene::generateLightTexture(ECS::Entity& light)
{
	auto lightComp = light.getComponent<ECS::LightData>();

	if (lightComp->active == false) return;

	// Find intersection
	this->findIntersectsWithRaycasts(lightComp->position);

	// sort
	this->sortIntersects();

	// genereate triangle verticies
	this->generateTriangles(lightComp->position);

	// Create temporary draw node
	auto drawNode = cocos2d::DrawNode::create();

	auto size = this->triangles.size();
	auto shift = this->displayBoundary.origin;
	auto color = cocos2d::Color4F::WHITE;
	// Draw triangles to draw node
	for (unsigned int i = 0; i < size; i += 3)
	{
		drawNode->drawTriangle(this->triangles.at(i) - shift,
			this->triangles.at(i + 1) - shift,
			this->triangles.at(i + 2) - shift,
			color);
	}

	// render to render texture
	//auto start4 = Utility::Time::now();
	// create render texture size of display boundary
	//int w = static_cast<int>(this->displayBoundary.size.width);
	//int h = static_cast<int>(this->displayBoundary.size.height);

	////auto renderTexture = cocos2d::RenderTexture::create(w, h);
	//auto renderTexture = cocos2d::RenderTexture::create(w, h, cocos2d::Texture2D::PixelFormat::RGB565);
	lightComp->renderTexture->beginWithClear(0, 0, 0, 0);
	drawNode->visit();
	lightComp->renderTexture->end();
	//auto end4 = Utility::Time::now();
	//cocos2d::log("render texture %s", Utility::Time::toMicroSecondString(start4, end4).c_str());

	// Create sprite to remember 
	if (lightComp->lightMapSprite != nullptr)
	{
		// Release if already had one
		lightComp->lightMapSprite->release();
	}

	// Create new sprite.
	lightComp->lightMapSprite = cocos2d::Sprite::createWithTexture(lightComp->renderTexture->getSprite()->getTexture());
	auto pos = cocos2d::Vec2(this->displayBoundary.getMidX(), this->displayBoundary.getMidY());
	lightComp->lightMapSprite->setPosition(pos - this->displayBoundary.origin);
	lightComp->lightMapSprite->setOpacity(70);
	lightComp->lightMapSprite->retain();
}

void VisibilityScene::generateLightMap()
{
	//auto start = Utility::Time::now();
	std::vector<ECS::Entity*> lights;
	ECS::Manager::getInstance()->getAllEntitiesInPool(lights, "LIGHT");
	//auto end = Utility::Time::now();
	//cocos2d::log("query lights %s", Utility::Time::toMicroSecondString(start, end).c_str());

	//auto start1 = Utility::Time::now();
	//int w = this->displayBoundary.size.width;
	//int h = this->displayBoundary.size.height;
	//auto rt = cocos2d::RenderTexture::create(w, h, cocos2d::Texture2D::PixelFormat::RGB565);
	//auto rt = cocos2d::RenderTexture::create(w, h);
	//auto end1 = Utility::Time::now();
	//cocos2d::log("create rt %s", Utility::Time::toMicroSecondString(start1, end1).c_str());

	//auto start2 = Utility::Time::now();
	this->lightMapRenderTexture->beginWithClear(0, 0, 0, 0);

	for (auto light : lights)
	{
		auto comp = light->getComponent<ECS::LightData>();
		if (comp->active)
		{
			if (comp->lightMapSprite != nullptr)
			{
				comp->lightMapSprite->visit();
			}
			else
			{
				cocos2d::log("Light map sprite is invalid. Possible bug?");
			}
		}
	}

	this->lightMapRenderTexture->end();
	//auto end2 = Utility::Time::now();
	//cocos2d::log("render texture %s", Utility::Time::toMicroSecondString(start2, end2).c_str());

	//auto start3 = Utility::Time::now();
	if (this->lightMapTexture != nullptr)
	{
		this->lightMapTexture->release();
	}

	this->lightMapTexture = this->lightMapRenderTexture->getSprite()->getTexture();
	this->lightMapTexture->retain();
	//auto end3 = Utility::Time::now();
	//cocos2d::log("create sprite with texture %s", Utility::Time::toMicroSecondString(start3, end3).c_str());
}

void VisibilityScene::updateCursorLight()
{
	if (this->cursorLight)
	{
		this->generateLightTexture(*this->cursorLightEntity);
	}
}

void VisibilityScene::drawDragBox()
{
	this->dragBoxDrawNode->clear();
	this->dragBoxDrawNode->drawSolidRect(this->newBoxOrigin, this->newBoxDest, cocos2d::Color4F::ORANGE);
	this->dragBoxDrawNode->drawRect(this->newBoxOrigin, this->newBoxDest, cocos2d::Color4F::YELLOW);
}

void VisibilityScene::clearDrag()
{
	this->currentMode = MODE::IDLE;
	this->newBoxDest = cocos2d::Vec2::ZERO;
	this->newBoxOrigin = cocos2d::Vec2::ZERO;
	this->dragBoxDrawNode->clear();
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Idle");
	this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::DRAW_RECT_WALL), cocos2d::Color3B::WHITE, false);
}

void VisibilityScene::drawFreeformWall()
{
	this->freeformWallDrawNode->clear();

	auto size = this->freeformWallPoints.size();
	if (size < 1) return;

	for (unsigned int i = 0; i < size; i++)
	{
		this->freeformWallDrawNode->drawSolidCircle(this->freeformWallPoints.at(i), 7.5f, 360.0f, 20, cocos2d::Color4F::BLUE);

		if (i < size - 1)
		{
			this->freeformWallDrawNode->drawLine(this->freeformWallPoints.at(i), this->freeformWallPoints.at(i + 1), cocos2d::Color4F::YELLOW);
		}
	}

	if (size > 1)
	{
		this->freeformWallDrawNode->drawLine(this->freeformWallPoints.at(0), this->freeformWallPoints.at(size - 1), cocos2d::Color4F::YELLOW);
	}
}

void VisibilityScene::clearFreeform()
{
	this->freeformWallPoints.clear();
	this->freeformWallDrawNode->clear();
	this->currentMode = MODE::IDLE;
	this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Idle");
	this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::DRAW_FREEFORM_WALL), cocos2d::Color3B::WHITE, false);
}

void VisibilityScene::drawWalls()
{
	this->wallDrawNode->clear();

	int index = 0;
	for (auto wall : this->walls)
	{
		if(wall.rectangle)
		{
			this->wallDrawNode->drawRect(wall.points.at(3), wall.points.at(1), cocos2d::Color4F::YELLOW);
		}
		else
		{
			auto size = wall.points.size();
			cocos2d::Color4F color = cocos2d::Color4F::YELLOW;
			if (wall.wallID == this->hoveringWallIndex)
			{
				color = cocos2d::Color4F::RED;
			}
			for (unsigned int i = 0; i < size; i++)
			{
				if (i < size - 1)
				{
					this->wallDrawNode->drawLine(wall.points.at(i), wall.points.at(i + 1), color);
				}
			}

			if (size > 1)
			{
				this->wallDrawNode->drawLine(wall.points.at(0), wall.points.at(size - 1), color);
			}
		}

		index++;
	}
}

void VisibilityScene::drawRaycast()
{
	this->raycastDrawNode->clear();

	auto color = cocos2d::Color4F::WHITE;
	//color.a = 0.1f;
	if (this->viewRaycast)
	{
		for (auto intersect : intersects)
		{
			this->raycastDrawNode->drawLine(this->mousePos, intersect.point, color);
		}
	}
}

void VisibilityScene::sortIntersects()
{
	if (this->intersects.empty()) return;

	// Sort intersecting points by angle.
	std::sort(this->intersects.begin(), this->intersects.end(), VertexComparator());
}

bool VisibilityScene::isOnLeft(const cocos2d::Vec2 & p1, const cocos2d::Vec2 & p2, const cocos2d::Vec2 & target)
{
	// 0 = on the line, positive = on left side, negative = on right side
	float value = (p2.x - p1.x) * (target.y - p1.y) - (target.x - p1.x) * (p2.y - p1.y);
	return value > 0;
}

bool VisibilityScene::didRayHitOtherWall(const std::unordered_set<int>& wallIDSet, const int uniquePointWallID)
{
	// Check if ray hit other walls.
	for (auto id : wallIDSet)
	{
		if (id == BOUNDARY_WALL_ID || id == uniquePointWallID)
		{
			// Boundary wall and unique point's wall doesn't count, keep go on
			continue;
		}
		else
		{
			// Ray hit other wall.
			return true;
		}
	}

	return false;
}

void VisibilityScene::updateUniformData()
{
	std::vector<ECS::Entity*> lights;
	ECS::Manager::getInstance()->getAllEntitiesInPool(lights, "LIGHT");

	int index = 0;
	int activeLightsCount = 0;
	for (auto light : lights)
	{
		auto lightComp = light->getComponent<ECS::LightData>();

		if (lightComp->active)
		{
			this->uniformData.at(index * 6) = lightComp->position.x;
			this->uniformData.at(index * 6 + 1) = lightComp->position.y;
			this->uniformData.at(index * 6 + 2) = lightComp->color.x;
			this->uniformData.at(index * 6 + 3) = lightComp->color.y;
			this->uniformData.at(index * 6 + 4) = lightComp->color.z;
			this->uniformData.at(index * 6 + 5) = lightComp->intensity;

			activeLightsCount++;
			index++;
		}
	}

	this->activeLightSize = activeLightsCount;
}

void VisibilityScene::setLightUniforms()
{
	this->floorShaderState->setUniformInt("lightSize", this->activeLightSize);

	this->floorShaderState->setUniformFloatv("lightSources", maxLightCount * 6, &this->uniformData.at(0));

	this->floorShaderState->setUniformTexture("lightMap", this->lightMapTexture);
}

void VisibilityScene::drawLights(bool updateLightTexture)
{
	if (updateLightTexture)
	{
		//auto start1 = Utility::Time::now();
		std::vector<ECS::Entity*> lights;
		ECS::Manager::getInstance()->getAllEntitiesInPool(lights, "LIGHT");

		for (auto light : lights)
		{
			// Generate light texture for each light
			this->generateLightTexture(*light);
		}
		//auto end1 = Utility::Time::now();
		//cocos2d::log("generateLightTexture() update light Texture %s", Utility::Time::toMicroSecondString(start1, end1).c_str());
	}

	// Generate light map by overlaying all light textures.
	//auto start = Utility::Time::now();
	this->generateLightMap();
	//auto end = Utility::Time::now();
	//cocos2d::log("generateLightMap() %s", Utility::Time::toMicroSecondString(start, end).c_str());

	// Update uniforms
	if (this->needToUpdateUniform)
	{
		this->updateUniformData();
		this->setLightUniforms();

		this->needToUpdateUniform = false;
	}
}

bool VisibilityScene::canFinishFreeformWallDrawing()
{
	if (this->freeformWallPoints.size() < 3)
	{
		return false;
	}
	// Check if last point and starting point segment intersects any line
	// copy points
	std::list<cocos2d::Vec2> verticies(std::begin(this->freeformWallPoints), std::end(this->freeformWallPoints));
	// remove the first and last point (touching points)
	verticies.pop_front();
	verticies.pop_back();
	// Check intersect
	//bool intersect = this->doesPointIntersectLines(points, verticies.back(), verticies.front());
	bool intersect = Utility::Polygon::doesPointIntersectPolygonSegments(verticies, verticies.front());
	if (intersect)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void VisibilityScene::update(float delta)
{
	this->labelsNode->updateFPSLabel(delta);

	if (this->currentMode == MODE::IDLE)
	{
		// Cursor is light
		if (this->cursorLight)
		{
			if (this->displayBoundary.containsPoint(this->mousePos))
			{
				if (!this->isPointInWall(this->mousePos))
				{
					if (this->mousePosDirty)
					{
						this->mousePosDirty = false;
						// Point is not in the wall
						if (this->debugMode)
						{
							// Show tirnagles and raycast in debug mode
							if (this->viewRaycast == false && this->viewVisibleArea == false)
							{
								// Nothing to render
								return;
							}

							this->findIntersectsWithRaycasts(this->mousePos);
							this->sortIntersects();

							if (this->viewRaycast)
							{
								this->drawRaycast();
							}

							if (this->viewVisibleArea)
							{
								this->generateTriangles(this->mousePos);
								this->drawTriangles();
							}
						}
						else
						{
							Utility::Time::start();

							// If it's not debug mode, cursor is light
							auto lightComp = this->cursorLightEntity->getComponent<ECS::LightData>();
							lightComp->position = this->mousePos;

							this->updateCursorLight();

							this->drawLights(false);

							Utility::Time::stop();

							std::string timeTakenStr = Utility::Time::getElaspedTime();	// Microseconds
							float timeTakenF = std::stof(timeTakenStr);	// to float
							timeTakenF *= 0.001f; // To milliseconds
							this->labelsNode->updateTimeTakenLabel(std::to_string(timeTakenF).substr(0, 5));
						}
					}
				}
			}
		}
	}
	else
	{
		this->triangleDrawNode->clear();
		this->raycastDrawNode->clear();
	}
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

	this->labelsNode->updateMouseHover(point);

	if (this->displayBoundary.containsPoint(point))
	{
		if (this->currentMode == MODE::IDLE)
		{
			/*
			int index = 0;
			bool hovered = false;
			for (auto wall : this->walls)
			{
				if (wall.rectangle)
				{
					if (wall.bb.containsPoint(point))
					{
						this->hoveringWallIndex = index;
						this->drawWalls();
						hovered = true;
						break;
					}
				}
				else
				{
					auto size = wall.points.size();
					if (size >= 3)
					{
						std::list<cocos2d::Vec2> pointList(wall.points.begin(), wall.points.end());

						bool inPolygon = earClipping->isPointInPolygon(pointList, point);

						if (inPolygon)
						{
							this->hoveringWallIndex = index;
							this->drawWalls();
							hovered = true;
						}
					}
					else
					{
						// it's a line
						continue;
					}
				}

				index++;
			}

			if (!hovered)
			{
				if (this->hoveringWallIndex != -1)
				{
					this->hoveringWallIndex = -1;
					this->drawWalls();
				}
			}
			*/
		}
		else if (this->currentMode == MODE::DRAW_WALL_READY)
		{
			// Trying to make wall with dragging
			if (static_cast<int>(this->walls.size())  < maxWallCount)
			{
				// still can make another box
				this->currentMode = MODE::DRAW_WALL_DRAG;
				this->newBoxDest = point;
				this->drawDragBox();
				this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Draw rectangle wall");
				this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::DRAW_RECT_WALL), cocos2d::Color3B::GREEN);
			}
			else
			{
				// Can't make more wall. return
				this->currentMode = MODE::IDLE;
				this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Idle");
				return;
			}
		}
		else if (this->currentMode == MODE::DRAW_WALL_DRAG)
		{
			if (!this->isPointInWall(point))
			{
				cocos2d::Vec2 size = point - this->newBoxOrigin;
				if (size.x > maxRectWallSegmentSize)
				{
					point.x = this->newBoxOrigin.x + maxRectWallSegmentSize;
				}
				else if (size.x < -maxRectWallSegmentSize)
				{
					point.x = this->newBoxOrigin.x - maxRectWallSegmentSize;
				}

				if (size.y > maxRectWallSegmentSize)
				{
					point.y = this->newBoxOrigin.y + maxRectWallSegmentSize;
				}
				else if (size.y < -maxRectWallSegmentSize)
				{
					point.y = this->newBoxOrigin.y - maxRectWallSegmentSize;
				}

				this->newBoxDest = point;

				this->drawDragBox();
			}
		}
	}

	if (this->cursorLight)
	{
		if (this->mousePos != point)
		{
			if (this->currentMode == MODE::IDLE)
			{
				this->mousePosDirty = true;
			}
			this->needToUpdateUniform = true;
		}
	}
	this->mousePos = point;
}

void VisibilityScene::onMouseDown(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	int mouseButton = mouseEvent->getMouseButton();
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	cocos2d::Vec2 point = cocos2d::Vec2(x, y);

	bool ret = this->labelsNode->updateMouseDown(point);
	if (ret)
	{
		return;
	}

	if (this->displayBoundary.containsPoint(point))
	{	
		if (this->currentMode == MODE::IDLE)
		{
			if (mouseButton == 0)
			{
				// Yet, we don't know if user is going to either drag or make point. 
				// So just put it in a ready state
				this->currentMode = MODE::DRAW_WALL_READY;
				this->newBoxOrigin = point;
				this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Draw wall");
			}
			else if(mouseButton == 1)
			{
				std::vector<ECS::Entity*> lights;
				ECS::Manager::getInstance()->getAllEntitiesInPool(lights, "LIGHT");
				for (auto light : lights)
				{
					if (light->getId() != this->cursorLightEntity->getId())
					{
						auto comp = light->getComponent<ECS::LightData>();
						if (comp->position.distance(point) < 10.0f)
						{
							light->kill();
							this->needToUpdateUniform = true;
							this->drawLights(false);
							this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::LIGHT_COUNT), "Lights: " + std::to_string(lights.size() - 2) + " / 15");
							return;
						}
					}
				}
				
				// Add light
				auto newLight = this->createNewLight(point);
				if (newLight != nullptr)
				{
					this->generateLightTexture(*newLight);
					this->needToUpdateUniform = true;
					// Draw light
					this->drawLights(false);
					int lightCount = ECS::Manager::getInstance()->getAliveEntityCountInEntityPool("LIGHT") - 1;
					this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::LIGHT_COUNT), "Lights: " + std::to_string(lightCount) + " / 15");
					this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::ADD_LIGHT), cocos2d::Color3B::WHITE);
				}
			}
		}
		else if (this->currentMode == MODE::DRAW_WALL_POINT)
		{
			// And add point as first point.
			if (static_cast<int>(this->freeformWallPoints.size()) < this->maxWallPointsPerWall)
			{
				if (!this->isPointInWall(point))
				{
					std::list<cocos2d::Vec2> verticies(std::begin(this->freeformWallPoints), std::end(this->freeformWallPoints));
					bool intersect = Utility::Polygon::doesPointIntersectPolygonSegments(verticies, point);
					if (!intersect)
					{
						if (this->freeformWallPoints.back().distance(point) < maxFreeformWallSegmentSize)
						{
							this->freeformWallPoints.push_back(point);
							this->drawFreeformWall();
						}
					}
				}
			}
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
		if (this->currentMode == MODE::IDLE)
		{
			// Do nothing?
		}
		else if (this->currentMode == MODE::DRAW_WALL_READY)
		{
			// try to Draw Freeform wall 
			if (static_cast<int>(this->walls.size()) < this->maxWallCount)
			{
				this->currentMode = MODE::DRAW_WALL_POINT;
				// And add point as first point.
				this->freeformWallPoints.clear();
				this->freeformWallPoints.push_back(point);
				this->drawFreeformWall();
				this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Draw freeform wall");
				this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::DRAW_FREEFORM_WALL), cocos2d::Color3B::GREEN);
			}
			else
			{
				// Can't make more walls. return
				this->currentMode = MODE::IDLE;
				this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::STATUS), "Status: Idle");
				return;
			}
		}
		else if (this->currentMode == MODE::DRAW_WALL_DRAG)
		{
			// End drawing				
			if (this->newBoxOrigin != this->newBoxDest)
			{
				// Make new box
				bool success = this->createNewRectWall();

				if (success)
				{
					this->addWall(this->walls.back());
					this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::WALL_COUNT), "Walls: " + std::to_string(this->walls.size()) + " / 20");
				}

				this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::DRAW_RECT_WALL), cocos2d::Color3B::WHITE);

				this->clearDrag();

				if (this->activeLightSize > 0)
				{
					this->needToUpdateUniform = true;
					this->drawLights(true);
				}
			}
		}
	}
	else
	{
		if (this->currentMode == MODE::DRAW_WALL_DRAG)
		{
			// Cancle draw
			this->currentMode = MODE::IDLE;
			this->newBoxDest = cocos2d::Vec2::ZERO;
			this->newBoxOrigin = cocos2d::Vec2::ZERO;
			this->dragBoxDrawNode->clear();
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
		if (this->currentMode == MODE::DRAW_WALL_POINT)
		{
			this->clearFreeform();
		}
		else if (this->currentMode == MODE::DRAW_WALL_DRAG || this->currentMode == MODE::DRAW_WALL_READY)
		{
			this->clearDrag();
		}
		else if (this->currentMode == MODE::IDLE)
		{
			// Terminate
			cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ENTER)
	{
		if (this->currentMode == MODE::DRAW_WALL_POINT)
		{
			if (this->freeformWallPoints.size() < 1)
			{
				this->clearFreeform();
			}
			else
			{
				if (this->canFinishFreeformWallDrawing())
				{
					bool success = this->createNewFreeformWall();
					if (success)
					{
						this->addWall(this->walls.back());
						this->labelsNode->updateLabel(static_cast<int>(CUSTOM_LABEL_INDEX::WALL_COUNT), "Walls: " + std::to_string(this->walls.size()) + " / 20");
						this->labelsNode->setColor(LabelsNode::TYPE::MOUSE, static_cast<int>(USAGE_MOUSE::DRAW_FREEFORM_WALL), cocos2d::Color3B::WHITE);
					}
					this->clearFreeform();

					if (this->activeLightSize > 0)
					{
						this->needToUpdateUniform = true;
						this->drawLights(true);
					}
				}
			}
		}
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_L)
	{
		this->cursorLight = !this->cursorLight;

		this->needToUpdateUniform = true;

		auto lightComp = this->cursorLightEntity->getComponent<ECS::LightData>();
		lightComp->position = this->mousePos;
		lightComp->active = this->cursorLight;

		this->needToUpdateUniform = true;
		if (this->cursorLight)
		{
			this->updateCursorLight();
		}
		this->drawLights(false);
		this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::CURSOR_LIGHT), this->cursorLight ? cocos2d::Color3B::GREEN : cocos2d::Color3B::WHITE);
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_T)
	{
		this->viewVisibleArea = !this->viewVisibleArea;
		if (!this->viewVisibleArea)
		{
			this->triangleDrawNode->clear();
		}
		this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::TOGGLE_TRIANGLE), this->viewVisibleArea ? cocos2d::Color3B::GREEN : cocos2d::Color3B::WHITE);
		this->mousePosDirty = true;
		this->needToUpdateUniform = true;
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_Y)
	{
		this->viewRaycast = !this->viewRaycast;
		if (!this->viewRaycast)
		{
			this->raycastDrawNode->clear();
		}
		this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::TOGGLE_RAYCAST), this->viewRaycast ? cocos2d::Color3B::GREEN : cocos2d::Color3B::WHITE);
		this->mousePosDirty = true;
		this->needToUpdateUniform = true;
	}
	else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_D)
	{
		this->debugMode = !this->debugMode;
		this->floorSprite->setVisible(!this->debugMode);
		if (!this->debugMode)
		{
			this->triangleDrawNode->clear();
			this->raycastDrawNode->clear();
		}
		this->labelsNode->setColor(LabelsNode::TYPE::KEYBOARD, static_cast<int>(USAGE_KEY::DEBUG_MODE), this->debugMode ? cocos2d::Color3B::GREEN : cocos2d::Color3B::WHITE);
		this->mousePosDirty = true;
		this->needToUpdateUniform = true;
	}
	else if(keyCode == cocos2d::EventKeyboard::KeyCode::KEY_M)
	{
		this->viewLightMap = !this->viewLightMap;
		this->removeChildByName("lightMapSprite");
		if (this->viewLightMap)
		{
			auto temp = cocos2d::Sprite::createWithTexture(this->lightMapTexture);
			temp->setPosition(cocos2d::Vec2(this->displayBoundary.getMidX(), this->displayBoundary.getMidY()));
			temp->setName("lightMapSprite");
			this->addChild(temp, 10000);
			this->floorSprite->setVisible(false);
		}
		else
		{
			this->floorSprite->setVisible(true);
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

	this->cursorLightEntity->kill();

	ECS::Manager::deleteInstance();

	for (auto segment : this->wallSegments)
	{
		delete segment;
	}

	this->earClipping->release();

	this->lightMapRenderTexture->release();
}