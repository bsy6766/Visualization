#ifndef FLOCKINGSCENE_H
#define FLOCKINGSCENE_H

#include "cocos2d.h"
#include "QTree.h"
#include "Entity.h"
#include "QTreeLineNode.h"

class FlockingScene : public cocos2d::CCScene
{
private:
	//default constructor
	FlockingScene() {};

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
	cocos2d::Rect displayBoundary;
	cocos2d::Node* areaNode;
	cocos2d::Node* qTreeLineNode;

	// Quadtree to optimize collision check
	QTree* quadTree;

	// Entities
	std::list<ECS::Entity*> entities;

	// Initialize entities and quad tree
	void initEntitiesAndQTree();
	// Creates new entity with required components
	ECS::Entity* createNewEntity();
public:
	//simple creator func
	static FlockingScene* createScene();

	//default destructor
	~FlockingScene() {};

	//Cocos2d Macro
	CREATE_FUNC(FlockingScene);
};

#endif