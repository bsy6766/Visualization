#ifndef CIRCLEPACKINGSCENE_H
#define CIRCLEPACKINGSCENE_H

#include "cocos2d.h"
#include "QTree.h"
#include "ECS.h"
#include <memory>

class CirclePackingScene : public cocos2d::Scene
{
private:
	//default constructor
	CirclePackingScene() {};

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

	cocos2d::Label* backLabel;
	int fps;
	float fpsElapsedTime;
	cocos2d::Label* fpsLabel;
	cocos2d::DrawNode* drawNode;

	std::vector<cocos2d::Image*> images;
	std::vector<cocos2d::Sprite*> imageSprites;

	// new circles are added on front. Growing circles are placed on front block. 
	// All grown circles are moved to end of list
	// [[----growing circles-----][-----all grown circles------]]
	//std::list<std::unique_ptr<Circle>> activeCircles;
	std::list<ECS::Entity*> activeCircles;
	// Fresh circles are deactived. Front element moves to activeCircles list when activated
	// [---------------deactivated circles----------------------]
	//std::list<std::unique_ptr<Circle>> freshCircles;
	std::list<ECS::Entity*> freshCircles;

	struct SpawnPoint
	{
		cocos2d::Vec2 point;
		cocos2d::Color4F color;
	};

	// Possible spawn point where circle can spawn. Pops from front.
	std::queue<SpawnPoint> circleSpawnPointsWithColor;

	// Initial number of circles that spawn on start
	int initialCircleCount;

	// Number of circles that spawn every tick
	int circleSpawnRate;

	// Pause simulation
	bool pause;

	// Simulate speed multiplier. 1.0 by default. 0 = stops simulation
	float simulateSpeedMultiplier;

	// Quadtree to optimize collision check
	QTree* quadTree;

	enum IMAGE_INDEX
	{
		CPP = 0,	//C++
		CAT,
		THE_SCREAM,
		MAX_SIZE,
		NONE,
	};

	IMAGE_INDEX currentImageIndex;

	enum SPRITE_Z_ORDER
	{
		BEHIND_CIRCLES = 0,
		CIRCLES,
		ABOVE_CIRCLES
	};

	int maxCircles;
	int searchSpawnPointWidthOffset;
	int searchSpawnPointHeightOffset;

	void initImages();
	void initImageAndSprite(const std::string& imageName);
	void findCircleSpawnPoint(const IMAGE_INDEX imageIndex);
	void initCircles();
	// Move all grown circles to back of list
	void moveAllGrownCircles();
	void spawnCircles(const int spawnRate);
	void resetCircles();
	cocos2d::Vec2 pixelToPoint(const int x, const int y, const int height, const cocos2d::Vec2& spritePos);
	void runCirclePacking(const IMAGE_INDEX imageIndex);
	void updateFPS(const float delta);
	ECS::Entity* createNewEntity();
	// Initialize entities and quad tree
	void initQuadTree();
	// Insert entities to quad tree
	void insertEntitiesToQuadTree();
	// Reset QuadTree and remove inactive entities
	void releaseQuadTree();

public:
	//simple creator func
	static CirclePackingScene* createScene();

	//default destructor
	~CirclePackingScene() {};

	//Cocos2d Macro
	CREATE_FUNC(CirclePackingScene);
};

#endif