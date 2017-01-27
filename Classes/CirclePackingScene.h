#ifndef CIRCLEPACKINGSCENE_H
#define CIRCLEPACKINGSCENE_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "QuadTree.h"
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

    // Labels
	cocos2d::Label* backLabel;
	int fps;
	float fpsElapsedTime;
	cocos2d::Label* fpsLabel;
	cocos2d::DrawNode* growingDrawNode;
	cocos2d::DrawNode* allGrownCircleDrawNode;
	cocos2d::Label* imageNameLabel;
	cocos2d::Label* imageTestPurposeLabel;
	cocos2d::Label* imageSelectInstructionLabel;
	cocos2d::Sprite* leftArrow;
	cocos2d::Node* imageSelectNode;
	cocos2d::Sprite* imageSelectPanelBg;
	bool viewingImageSelectPanel;
	cocos2d::Label* statsLabel;
	cocos2d::Label* runStatus;
	cocos2d::Label* possibleCircleSpawnPointLabel;
	cocos2d::Label* spawnedCircleCountLabel;

    // Images and sprites
	std::vector<cocos2d::Image*> images;
	std::vector<cocos2d::Sprite*> imageSprites;
	std::vector<cocos2d::ui::Button*> imageSpritesIconButtons;

	enum BUTTON_TAG
	{
		CPP,
		CAT,
		THE_SCREAM,
		GRADIENT
	};

	// new circles are added on front. Growing circles are placed on front block. 
	// All grown circles are moved to end of list
	// [[----growing circles-----][-----all grown circles------]]
	std::list<ECS::Entity*> activeCircles;
	// Fresh circles are deactived. Front element moves to activeCircles list when activated
	// [---------------deactivated circles----------------------]
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
    
    // Maximum attempt that circle spawn function can take
    const int MAXIMUM_SPAWN_ATTEMPT = 30;

	// Pause simulation
	bool pause;

	// Simulate speed multiplier. 1.0 by default. 0 = stops simulation
	float simulateSpeedMultiplier;

	// Keep track growing circle number
	int growingCircleCount;

	// Quadtree to optimize collision check
	QuadTree* quadTree;

	enum class IMAGE_INDEX
	{
		CPP = 0,	//C++
		CAT,		//Color test
		THE_SCREAM,	//Color test
		GRADIENT,	//Alpha test
		MAX_SIZE,
		NONE,
	};

	IMAGE_INDEX currentImageIndex;

	enum class SPRITE_Z_ORDER
	{
		BEHIND_CIRCLES = 0,
		CIRCLES,
		ABOVE_CIRCLES
	};

    // Maximum circle that can spawn
	int maxCircles;
    
    // Search offsets. @see findCircleSpawnPoint(const IMAGE_INDEX)
	int searchSpawnPointWidthOffset;
	int searchSpawnPointHeightOffset;

    // Initialize all images
	void initImages();
    
    // Initialize image and sprite
	void initImageAndSprite(const std::string& imageName, const BUTTON_TAG buttonTag);
    
    // Find possible circle's spawn location on the screen for image
	void findCircleSpawnPoint(const IMAGE_INDEX imageIndex);
    
    // Initialize circles.
	void initCircles();
    
	// Move all grown circles to back of list. Returns true if new all grown up circles are found
	const bool moveAllGrownCircles();
    
    /**
     *  Spawn circles.
     *  @return The actual number of circles spawned on each frame
     */
	const int spawnCircles();
    
    // Deactivates active circles and merge to fresh circels list.
	void resetCircles();
    
    // Convert pixel point to screen point
	cocos2d::Vec2 pixelToPoint(const int x, const int y, const int height, const cocos2d::Vec2& spritePos);
    
    // Run circle packing algorithm with specific image
	void runCirclePacking(const IMAGE_INDEX imageIndex);
    
    // Set image name label corresponding to image name
	void setImageNameLabel();
    
    // Update FPS.
	void updateFPS(const float delta);
    
    // Update active circle's radius(growth)
	void updateCircleGrowth(const float delta);
    
    // Check each circle's collision and resolve the collision
	void updateCircleCollisionResolution();
    
    // Draw circle with cocos2d-x draw node. Disabled
	void updateDrawNodes(const bool clearAllGrownDrawNode);
    
    // Create new circle entity
	ECS::Entity* createNewEntity();
    
	// Initialize entities and quad tree
	void initQuadTree();
    
	// Insert entities to quad tree
	void insertEntitiesToQuadTree();
    
	// Reset QuadTree and remove inactive entities
	void releaseQuadTree();
    
	// Button press call back
	void onButtonPressed(cocos2d::Ref* sender);

public:
	//simple creator func
	static CirclePackingScene* createScene();

	//default destructor
	~CirclePackingScene() {};

	//Cocos2d Macro
	CREATE_FUNC(CirclePackingScene);
};

#endif
