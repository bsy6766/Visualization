#ifndef QTREESCENE_H
#define QTREESCENE_H

#include "cocos2d.h"
#include "QuadTree.h"
#include "Component.h"
#include "ECS.h"
#include "Component.h"
#include "CustomNode.h"
#include <vector>
#include <list>

class QuadTreeScene : public cocos2d::Scene
{
private:
	//default constructor
	QuadTreeScene() {};

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

	// Separating each usage labels for animation
	std::vector<cocos2d::Label*> usageLabels;

    enum class CUSTOM_LABEL_INDEX
    {
        ENTITIES,
        COLLISION,
        COLLISION_WO_DUP_CHECK,
        BRUTE_FORCE,
        QUAD_TREE_MAX_LEVEL,
    };
    
	enum class USAGE_KEY
	{
        NONE = 0,
		SPACE,
		CLEAR,
		ADD_TEN,
		REMOVE_TEN,
		GRID,
		DUPL_CHECK,
		COL_RESOLVE,
		INC_QTREE_LEVEL,
		DEC_QTREE_LEVEL,
		MAX_KEYBOARD_USAGE,
	};
    
    enum class USAGE_MOUSE
    {
        NONE = 0,
        ADD_ONE,
        TRACK,
        REMOVE_ONE,
        MAX_MOUSE_USAGE
    };

	enum Z_ORDER
	{
		LINE,
		ENTITY,
		BOX
	};

	// flags
	bool pause;
	bool showGrid;
	bool duplicationCheck;
	bool collisionResolve;

	// Tracking entity
	int lastTrackingEntityID;

	// Quadtree
	QuadTree* quadTree;

	// Node
	cocos2d::Node* areaNode;
	QuadTreeLineNode* quadTreeLineNode;
    DisplayBoundaryBoxNode* displayBoundaryBoxNode;
    LabelsNode* labelsNode;

	// Boundary holder
	cocos2d::Rect displayBoundary;

	// Track all entities
	std::list<ECS::Entity*> entities;

	// Initialize entities and quad tree
	void initEntitiesAndQTree();
	// Creates new entity with required components
	ECS::Entity* createNewEntity();
	// Update each entity's position and reassign to quadtree
	void resetQTreeAndUpdatePosition(float delta);
	// Checks collision between entities, count number, resolve if enabled
	void checkCollision();
	// Checks if box is out of boundary. Sets bool to true if box need to flip direction
	void checkBoundary(ECS::Sprite& spriteComp, bool& flipX, bool& flipY);
	// Flips direction
	void flipDirVec(const bool flipX, const bool flipY, cocos2d::Vec2& dirVec);
	// Resolves collision between two entities.
	void resolveCollisions(ECS::Sprite& entitySpriteComp, ECS::Sprite& nearEntitySpriteComp, ECS::DirectionVector& entityDirVecComp, ECS::DirectionVector& nearEntityDirVecComp);
	// Reassigns entitiy id to keep id less than 1000 (because of duplication check).
	void reassignEntityIds();
    // Toggle color
    void toggleColor(const bool enabled, LabelsNode::TYPE type, const int index, const bool playAnimation = true);
public:
	//simple creator func
	static QuadTreeScene* createScene();

	//default destructor
	~QuadTreeScene() {};

	//Cocos2d Macro
	CREATE_FUNC(QuadTreeScene);
};

#endif
