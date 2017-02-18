#ifndef VISIBILITYSCENE_H
#define VISIBILITYSCENE_H

#include <cocos2d.h>
#include "CustomNode.h"
#include "ECS.h"
#include "Component.h"
#include "System.h"

struct EndPoint;
struct Segment;

struct EndPoint : public cocos2d::Vec2
{
	bool begin;
	float angle;
	bool visualize;
	Segment* segment;

	EndPoint::EndPoint() : begin(false), angle(0), visualize(0), segment(nullptr) {}
};

struct EndPointComparator
{
	int operator()(EndPoint* a, EndPoint* b)
	{
		// smaller angle goes first, begin goes first
		if (a->angle == b->angle)
		{
			if (!a->begin && b->begin)
			{
				return false;
			}
			if (a->begin && !b->begin)
			{
				return true;
			}
		}
		else
		{
			if (a->angle > b->angle)
			{
				return false;
			}
			else
			{
				return true;
			}
			//return a->angle < b->angle;
		}
		return false;
	}
};

struct Segment
{
	EndPoint* p1;
	EndPoint* p2;
	float d;

	Segment::Segment() : p1(nullptr), p2(nullptr), d(0) {}
	Segment::~Segment()
	{
		if (p1) delete p1;
		if (p2) delete p2;
	}

	bool operator==(const Segment& other) const
	{
		return (this->p1->x == other.p1->x) && (this->p1->y == other.p1->y)
			&& (this->p2->x == other.p2->x) && (this->p2->y == other.p2->y);
	}
};

class VisibilityScene : public cocos2d::Scene
{
private:
	//default constructor
	VisibilityScene() {};

	//Input Listeners
	cocos2d::EventListenerMouse* mouseInputListener;
	cocos2d::EventListenerKeyboard* keyInputListener;

	//Mouse events
	void onMouseMove(cocos2d::Event* event);
	void onMouseDown(cocos2d::Event* event);
	void onMouseUp(cocos2d::Event* event);
	void onMouseScroll(cocos2d::Event* event);

	//keyboard events
	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
	void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	//cocos2d virtuals
	virtual bool init() override;
	virtual void onEnter() override;
	virtual void update(float delta) override;
	virtual void onExit() override;

	void initInputListeners();
	void releaseInputListeners();

	enum Z_ORDER
	{
		BOX
	};

	enum class CUSTOM_LABEL_INDEX
	{
		STATUS,
		MODE
	};

	enum class MODE
	{
		IDLE,
		BOX,
		BOX_EDIT,
		LIGHT
	};
	
	MODE currentMode;

	bool draggingBox;
	cocos2d::Vec2 newBoxOrigin;
	cocos2d::Vec2 newBoxDest;
	cocos2d::DrawNode* newBoxDrawNode;
	cocos2d::DrawNode* visiableAreaDrawNode;
	std::vector<cocos2d::Vec2> verticies;

	LabelsNode* labelsNode;
	DisplayBoundaryBoxNode* displayBoundaryBoxNode;
	cocos2d::Rect displayBoundary;

	std::vector<EndPoint*> endPoints;
	std::vector<Segment*> segments;

	// init ECS
	void initECS();
	// Create new box entity
	void createNewBox();
	// Create new light entity
	void createNewLight(const cocos2d::Vec2& position);
	// Sweep area
	void sweep();
	// load map
	void loadMap();
	// load boundary
	void loadBoundary();
	// load rect
	void loadRect(const cocos2d::Rect& rect);
	// Add segment
	void addSegment(EndPoint& p1, EndPoint& p2);
	// Set light location
	void setLightLocation();
	// Check if segment a is front of b
	bool checkSegmentInFrontOf(Segment* a, Segment* b, const cocos2d::Vec2& relativeOf/*light position*/);
	// Check if point is left of segment
	bool isLeftOf(Segment* segment, const cocos2d::Vec2& point);
	// Get slightly short version of point
	const cocos2d::Vec2 interpolate(const cocos2d::Vec2& p, const cocos2d::Vec2& q, const float ratio);
	// add triangle
	void addTriangle(float angle1, float angle2, Segment* segment, const cocos2d::Vec2& lightPos);
	// Get point where line intersects
	const cocos2d::Vec2 getIntersectingPoint(const cocos2d::Vec2& p1, const cocos2d::Vec2& p2, const cocos2d::Vec2& p3, const cocos2d::Vec2& p4);
public:
	//simple creator func
	static VisibilityScene* createScene();

	//default destructor
	~VisibilityScene() {};

	//Cocos2d Macro
	CREATE_FUNC(VisibilityScene);
};

#endif