#ifndef VISIBILITYSCENE_H
#define VISIBILITYSCENE_H

#include <cocos2d.h>
#include "CustomNode.h"
#include "ECS.h"
#include "Component.h"
#include "System.h"
#include "EarClippingScene.h"

struct UniquePoint
{
	int wallID;
	cocos2d::Vec2 point;
};

struct Segment
{
	cocos2d::Vec2 p1;
	cocos2d::Vec2 p2;
	int wallID;						// ID for wall. Uses ECS's entity id.
};

struct Vertex
{
	enum class TYPE
	{
		ON_BOUNDARY,
		ON_WALL,
		ON_UNIQUE_POINT
	};
	cocos2d::Vec2 point;
	cocos2d::Vec2 uniquePoint;
	TYPE type;
	float angle;
	int uniquePointWallID;
	int pointWallID;
};

struct VertexComparator
{
	bool operator()(const Vertex& a, const Vertex& b)
	{
		return a.angle < b.angle;
	}
};

struct Wall
{
	std::vector<cocos2d::Vec2> points;
	cocos2d::Rect bb;
	cocos2d::Vec2 center;
	int wallID;
	float angle;
	bool rectangle;
};

struct Hit
{
	float t;
	float u;
	cocos2d::Vec2 hitPoint;
	bool parallel;
	bool perpendicular;
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
		DEBUG,
		FLOOR,
		WALL,
		TRIANGLE,
		RAYCAST,
		BOX,
		DRAG,
		FREEFORM,
		LIGHT_MAP,
		LIGHT_ICON,
	};

	enum class CUSTOM_LABEL_INDEX
	{
		STATUS,
		WALL_COUNT,
		LIGHT_COUNT
	};

	enum class USAGE_KEY
	{
		NONE,
		CURSOR_LIGHT,
		DEBUG_MODE,
		TOGGLE_RAYCAST,
		TOGGLE_TRIANGLE,
	};

	enum class USAGE_MOUSE
	{
		NONE,
		DRAW_RECT_WALL,
		DRAW_FREEFORM_WALL,
		ADD_LIGHT
	};

	enum class MODE
	{
		IDLE,
		DRAW_WALL_READY,
		DRAW_WALL_DRAG,
		DRAW_WALL_POINT,
	};
	
	MODE currentMode;

	bool draggingBox;
	const int maxWallCount = 20;
	const int maxWallPointsPerWall = 8;
	const int maxLightCount = 16; // 16 lights
	const float maxRectWallSegmentSize = 100.0f;
	const float maxFreeformWallSegmentSize = 150.0f;
	const float minRectSize = 10.0f;
	const int BOUNDARY_WALL_ID = -1;
	const float defaultLightIntensity = 100.0f;
	static int wallIDCounter;

	EarClippingScene* earClipping;

	cocos2d::DrawNode* visiableAreaDrawNode;

	cocos2d::Vec2 newBoxOrigin;
	cocos2d::Vec2 newBoxDest;
	cocos2d::DrawNode* dragBoxDrawNode;
	cocos2d::DrawNode* wallDrawNode;

	cocos2d::DrawNode* freeformWallDrawNode;
	std::vector<cocos2d::Vec2> freeformWallPoints;

	LabelsNode* labelsNode;
	DisplayBoundaryBoxNode* displayBoundaryBoxNode;
	cocos2d::Rect displayBoundary;

	cocos2d::DrawNode* raycastDrawNode;
	cocos2d::DrawNode* triangleDrawNode;
	cocos2d::Vec2 mousePos;
	bool mousePosDirty;

	cocos2d::RenderTexture* lightMapRenderTexture;

	std::vector<Vertex> intersects;
	std::vector<cocos2d::Vec2> triangles;

	std::vector<UniquePoint> wallUniquePoints;

	std::vector<Segment*> wallSegments;
	std::vector<Segment*> boundarySegments;
	std::vector<Wall> walls;

	cocos2d::GLProgram* floorShader;
	cocos2d::GLProgramState* floorShaderState;
	cocos2d::Sprite* floorSprite;
	cocos2d::Texture2D* lightMapTexture;

	std::vector<cocos2d::Vec2> lightPositions;
	std::vector<cocos2d::Vec3> lightColors;
	std::vector<float> lightIntensities;
	// pos(vec2), color(vec3), intensity(float), repeat...
	std::vector<float> uniformData;
	int activeLightSize;

	int hoveringWallIndex;

	bool needToUpdateUniform;
	bool viewRaycast;
	bool viewVisibleArea;
	bool debugMode;
	bool viewLightMap;
	bool cursorLight;
	ECS::Entity* cursorLightEntity;

	// init ECS
	void initECS();
	// Create new box entity
	bool createNewRectWall();
	// create new freeform wall
	bool createNewFreeformWall();
	// Create new light entity
	ECS::Entity* createNewLight(const cocos2d::Vec2& position);
	// initialize map
	void initMap();
	// load map
	void reloadMap();
	// add Wall
	void addWall(Wall& wall);
	// load rect
	void loadRect(const cocos2d::Rect& rect, std::vector<Segment*>& segments, const int wallID);
	// load freeform
	void loadFreeform(const std::vector<cocos2d::Vec2>& points, std::vector<Segment*>& segments, const int wallID);
	// load segments
	void addSegment(const cocos2d::Vec2& p1, const cocos2d::Vec2& p2, std::vector<Segment*>& segments, const int wallID);
	// Get intersecting point where raycast hits
	float getIntersectingPoint(const cocos2d::Vec2& rayStart, const cocos2d::Vec2& rayEnd, const Segment* segment, cocos2d::Vec2& intersection);
	cocos2d::Vec2 getIntersectingPoint(const cocos2d::Vec2& rayStart, const cocos2d::Vec2& rayEnd, const cocos2d::Vec2& segStart, const cocos2d::Vec2& segEnd);
	bool getIntersectingPoint(const cocos2d::Vec2& rayStart, const cocos2d::Vec2& rayEnd, const Segment* segment, Hit& hit);
	// cast rays
	void findIntersectsWithRaycasts(const cocos2d::Vec2& lightPos);
	// Generate triangles based on intersecting points
	void generateTriangles(const cocos2d::Vec2& lightPos);
	// draw triangles
	void drawTriangles();
	// Check if mouse point is inside of wall
	bool isPointInWall(const cocos2d::Vec2& point);
	// generateLightTexture
	void generateLightTexture(ECS::Entity& light);
	// generate light map texture
	void generateLightMap();
	// Update cursor light
	void updateCursorLight();
	// Draw new wall preview
	void drawDragBox();
	// clear drag
	void clearDrag();
	// Draw Freeform wall
	void drawFreeformWall();
	// clear freeform
	void clearFreeform();
	// draw walls
	void drawWalls();
	// draw raycast
	void drawRaycast();
	// Sort intersects. Call this before draw triangles.
	void sortIntersects();
	// check if point is on left or right of segment
	bool isOnLeft(const cocos2d::Vec2& p1, const cocos2d::Vec2& p2, const cocos2d::Vec2& target);
	// Check if ray hit other wall
	bool didRayHitOtherWall(const std::unordered_set<int>& wallIDSet, const int uniquePointWallID);
	// update uniform data
	void updateUniformData();
	// set light uniforms
	void setLightUniforms();
	// Draw lights
	void drawLights(bool updateLightTexture);
	// Check if can finish drawing freeform wall
	bool canFinishFreeformWallDrawing();
	
public:
	//simple creator func
	static VisibilityScene* createScene();

	//default destructor
	~VisibilityScene() {};

	//Cocos2d Macro
	CREATE_FUNC(VisibilityScene);
};

#endif