#ifndef FLOCKINGSCENE_H
#define FLOCKINGSCENE_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "QTree.h"
#include "ECS.h"
#include "Component.h"
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

	// cocos2d
	cocos2d::Label* backLabel;
	cocos2d::Rect displayBoundary;
	cocos2d::Node* areaNode;
	cocos2d::Node* qTreeLineNode;
	cocos2d::Node* blackArea;

	// Labels
	cocos2d::Label* weightLabel;

	cocos2d::Label* alignmentLabel;
	cocos2d::Label* alignmentWeightLabel;
	cocos2d::ui::Button* leftAlignmentButton;
	cocos2d::ui::Button* rightAlignmentButton;

	cocos2d::Label* cohesionLabel;
	cocos2d::Label* cohesionWeightLabel;
	cocos2d::ui::Button* leftCohesionButton;
	cocos2d::ui::Button* rightCohesionButton;

	cocos2d::Label* separationLabel;
	cocos2d::Label* separationWeightLabel;
	cocos2d::ui::Button* leftSeparationButton;
	cocos2d::ui::Button* rightSeparationButton;

	cocos2d::Label* avoidLabel;
	cocos2d::Label* avoidWeightLabel;
	cocos2d::ui::Button* leftAvoidButton;
	cocos2d::ui::Button* rightAvoidButton;

	cocos2d::Label* entityCountLabel;
	cocos2d::Label* fpsLabel;
	std::vector<cocos2d::Label*> usageLabels;	
	
	enum USAGE_KEY
	{
		SPACE = 1,
		CLEAR,
		ADD_TEN,
		REMOVE_TEN,
		MAX_KEYBOARD_USAGE,
		ADD_ONE,
		TRACK,
		REMOVE_ONE,
		ADD_OBSTACLE,
		REMOVE_OBSTACLE,
		MAX_MOUSE_USAGE
	};

	// UI animation
	cocos2d::ActionInterval* clickAnimation;

	enum ACTION_TAG
	{
		ALIGNMENT_LEFT,
		ALIGNMENT_RIGHT,
		COHESION_LEFT,
		COHESION_RIGHT,
		SEPARATION_LEFT,
		SEPARATION_RIGHT,
		AVOID_LEFT,
		AVOID_RIGHT
	};

	// Quadtree to optimize collision check
	QTree* quadTree;

	// Flags
	bool pause;

	// FPS
	float fpsElapsedTime;
	int fps;

	// Entities
	std::list<ECS::Entity*> entities;

	// RangeChecker
	cocos2d::Sprite* rangeChecker;

	// Tracking
	int lastTrackingBoidId;

	// Initialize entities and quad tree
	void initEntitiesAndQTree();
	// Creates new entity with required components
	ECS::Entity* createNewEntity();
	ECS::Entity* createNewEntity(const cocos2d::Vec2& pos);
	ECS::Entity* createNewObstacleEntity(const cocos2d::Vec2& pos);

	// Reset QuadTree and remove inactive entities
	void resetQTreeAndPurge();
	// Update flocking algorithm
	void updateFlockingAlgorithm(const float delta);
	// Update FPS label
	void updateFPS(const float delta);

	// Flocking algorithm
	/**
	*	Get Alignment vector
	*	Iterate through near boids and checks distance.
	*	If distance between boid and near boids are close enough, 
	*	sum near boids' direction vector and get average then normalize
	*/
	const cocos2d::Vec2 getAlignment(Entity* boid, std::list<Entity*>& nearBoids);
	/**
	*	Get Cohesion
	*	Iterate thorugh near boids and checks distance.
	*	If distance between boid and near boids are close enough,
	*	sum near boids' position vector and get average then nomarlize.
	*	In this case, we want direction vector not position, so compute
	*	direction vector based on boid's position
	*/
	const cocos2d::Vec2 getCohesion(Entity* boid, std::list<Entity*>& nearBoids);
	/**
	*	Get Separation
	*	Iterate through near boids and checks distance
	*	If distance between boid and near boids are close enough,
	*	sum the distance between boid and near boids and get average, 
	*	negate the direction, then normalize. 
	*/
	const cocos2d::Vec2 getSeparation(Entity* boid, std::list<Entity*>& nearBoids);
	/**
	*	Get Avoid
	*	This isn't core flocking algorithm. Alignment, Cohesion and
	*	Separation are the core but this is a simple addition to
	*	algorithm to make boid avoid obstacles.
	*	It's similar to Separation, but weighted by distance
	*/
	const cocos2d::Vec2 getAvoid(Entity* boid, std::list<Entity*>& nearAvoids);

	// UI callbacks
	void onButtonPressed(cocos2d::Ref* sender);
	// Play UI animation
	void playUIAnimation(const USAGE_KEY usageKey);
public:
	//simple creator func
	static FlockingScene* createScene();

	//default destructor
	~FlockingScene() {};

	//Cocos2d Macro
	CREATE_FUNC(FlockingScene);
};

#endif