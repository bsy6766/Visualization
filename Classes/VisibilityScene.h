#ifndef VISIBILITYSCENE_H
#define VISIBILITYSCENE_H

#include <cocos2d.h>
#include "CustomNode.h"
#include "ECS.h"
#include "Component.h"
#include "System.h"

struct Segment
{
	cocos2d::Vec2 p1;
	cocos2d::Vec2 p2;
	int wallID;						// ID for wall. Uses ECS's entity id.
};

struct Vertex
{
	cocos2d::Vec2 vertex;			// Default vertex. Usually unique point.
	bool boundaryVisible;			// True if this vertex can be extended to boundary
	bool otherWallVisible;			// True if this vertex can be extended to other wall
	bool isBounday;					// True if this vertex is endpoint of boundary segments
	cocos2d::Vec2 extendedVertex;	// Only if wall is visible
	float angle;					// Angle between light position. Used for sorting.
	int wallID;
	int extendedWallID;
};

struct VertexComparator
{
	bool operator()(const Vertex& a, const Vertex& b)
	{
		return a.angle < b.angle;
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
		WALL,
		RAYCAST,
		TRIANGLE,
		BOX,
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

	LabelsNode* labelsNode;
	DisplayBoundaryBoxNode* displayBoundaryBoxNode;
	cocos2d::Rect displayBoundary;

	cocos2d::DrawNode* raycastDrawNode;
	cocos2d::DrawNode* triangleDrawNode;
	cocos2d::Vec2 mousePos;

	std::vector<Vertex> intersects;

	std::vector<cocos2d::Vec2> lightPositions;

	std::vector<cocos2d::Vec2> wallUniquePoints;
	std::vector<cocos2d::Vec2> boundaryUniquePoints;

	std::vector<Segment*> wallSegments;
	std::vector<Segment*> boundarySegments;

	// init ECS
	void initECS();
	// Create new box entity
	void createNewBox();
	// Create new light entity
	void createNewLight(const cocos2d::Vec2& position);
	// load map
	void loadMap();
	// load rect
	void loadRect(const cocos2d::Rect& rect, std::vector<Segment*>& segments, const int wallID);
	// load segments
	void addSegment(const cocos2d::Vec2& p1, const cocos2d::Vec2& p2, std::vector<Segment*>& segments, const int wallID);
	// Get intersecting point where raycast hits
	float getIntersectingPoint(const cocos2d::Vec2& rayStart, const cocos2d::Vec2& rayEnd, const Segment* segment, cocos2d::Vec2& intersection);
	// cast rays
	void findIntersectsWithRaycasts();
	// draw triangles
	void drawTriangles();
	// Check if intersecting point is on bounday
	bool isPointOnBoundary(const cocos2d::Vec2& point);
public:
	//simple creator func
	static VisibilityScene* createScene();

	//default destructor
	~VisibilityScene() {};

	//Cocos2d Macro
	CREATE_FUNC(VisibilityScene);
};

#endif