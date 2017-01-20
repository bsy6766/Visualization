#ifndef QTREESCENE_H
#define QTREESCENE_H

#include "cocos2d.h"
#include "QTree.h"
#include "Component.h"
#include "ECS.h"
#include "Component.h"
#include "QTreeLineNode.h"
#include <vector>
#include <list>

class QTreeScene : public cocos2d::Scene
{
private:
	//default constructor
	QTreeScene() {};

	//Input Listeners
	cocos2d::EventListenerMouse* mouseInputListener;
	cocos2d::EventListenerKeyboard* keyInputListener;

	//Mouse events
	void onMouseMove(cocos2d::Event* event);
	void onMouseDown(cocos2d::Event* event);

	//keyboard events
	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

	//cocos2d virtuals
	virtual bool init() override;
	virtual void onEnter() override;
	virtual void update(float delta) override;
	virtual void onExit() override;

	// Initialize and release input listeners.
	void initInputListeners();
	void releaseInputListeners();

	// Exit(Back) label
	cocos2d::Label* backLabel;
	
	// Label numbers
	int entityCount;
	int collisionsCount;
	int collisionChecksCount;
	int collisionCheckWithOutRepeatCount;
	int bruteforceChecksCount;
	int fps;
	float fpsElapsedTime;

	// Cocos2d labels
	cocos2d::Label* entityCountLabel;
	cocos2d::Label* collisionChecksCountLabel;
	cocos2d::Label* bruteforceChecksCountLabel;
	cocos2d::Label* collisionCheckWithOutRepeatCountLabel;
	cocos2d::Label* fpsLabel;
	cocos2d::Label* quadtreeLevelLabel;

	// Separating each usage labels for animation
	std::vector<cocos2d::Label*> usageLabels;

	enum USAGE_KEY
	{
		SPACE = 1,
		CLEAR,
		ADD_TEN,
		REMOVE_TEN,
		GRID,
		DUPL_CHECK,
		COL_RESOLVE,
		INC_QTREE_LEVEL,
		DEC_QTREE_LEVEL,
		MAX_KEYBOARD_USAGE,
		ADD_ONE,
		TRACK,
		REMOVE_ONE,
		MAX_MOUSE_USAGE
	};

	// flags
	bool pause;
	bool showGrid;
	bool duplicationCheck;
	bool collisionResolve;

	// Tracking entity
	int lastTrackingEntityID;
	
	// UI animation
	cocos2d::ActionInterval* clickAnimation;

	// Quadtree
	QTree* quadTree;

	// Node
	cocos2d::Node* areaNode;
	QTreeLineNode* qTreeLineNode;

	// Boundary
	cocos2d::Rect displayBoundary;

	// Track all entities
	std::list<ECS::Entity*> entities;

	// Initialize entities and quad tree
	void initEntitiesAndQTree();
	// Creates new entity with required components
	ECS::Entity* createNewEntity();
	// Updates FPS instead of using built in fps label
	void updateFPS(float delta);
	// Update each entity's position and reassign to quadtree
	void resetQTreeAndUpdatePosition(float delta);
	// Checks collision between entities, count number, resolve if enabled
	void checkCollision();
	// Checks if box is out of boundary. Sets bool to true if box need to flip direction
	void checkBoundary(ECS::Sprite& spriteComp, bool& flipX, bool& flipY);
	// Flips direction
	void flipDirVec(const bool flipX, const bool flipY, cocos2d::Vec2& dirVec);
	// Updates labels with number
	void updateLabels();
	// Resolves collision between two entities.
	void resolveCollisions(ECS::Sprite& entitySpriteComp, ECS::Sprite& nearEntitySpriteComp, ECS::DirectionVector& entityDirVecComp, ECS::DirectionVector& nearEntityDirVecComp);
	// Reassigns entitiy id to keep id less than 1000 (because of duplication check).
	void reassignEntityIds();
	// Play UI animation
	void playUIAnimation(const USAGE_KEY usageKey);
public:
	//simple creator func
	static QTreeScene* createScene();

	//default destructor
	~QTreeScene() {};

	//Cocos2d Macro
	CREATE_FUNC(QTreeScene);
};

#endif