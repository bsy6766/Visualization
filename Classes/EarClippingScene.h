#ifndef EARCLIPPINGSCENE_H
#define EARCLIPPINGSCENE_H

#include "cocos2d.h"
#include "ui\CocosGUI.h"
#include "Component.h"
#include "System.h"
#include "ECS.h"
#include "CustomNode.h"
#include <list>

class EarClippingScene : public cocos2d::Scene
{
private:
	//default constructor
	EarClippingScene() {};

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
    
    enum class CUSTOM_LABEL_INDEX
    {
        STATUS,
		OUTER_VERTEX,
		INNER_VERTEX,
		TOTAL_TRIANGLE,
        MAX_CUSTOM_LABEL,
    };

	enum class USAGE_KEY
	{
		NONE,
		ENTER,	// Start, next
		CANCEL,
		CLEAR,
		RESTART,
	};

	enum class USAGE_MOUSE
	{
		NONE,
		ADD_OUTER_VERTEX,
		ADD_INNER_VERTEX,
		REMOVE_VERTEX,
	};
    
    enum SCENE_STATE
    {
        IDLE,
        DRAWING_OUTER_STATE,
        DRAWING_INNER_STATE,
        FINALIZE_STATE,
        ALGORITHM_STATE,
		FINISHED,
    };
    
    enum Z_ORDER
    {
        LINE,
        DOT,
        BOX,
		INSTRUCTION
    };
    
    struct Vertex
    {
        cocos2d::Vec2 point;
        Vertex* prev;
        Vertex* next;
    };

	void initInputListeners();
	void releaseInputListeners();
    const bool isCounterClockWise(const std::list<cocos2d::Vec2>& verticies);
    void reverseVerticiesOrder(std::list<cocos2d::Vec2>& verticies, std::list<cocos2d::Label*>& labels);
    // Returns positive if vec b is ccw from vec a, returns negative if cw from vec a. 0 if parrellel
    const float ccw(const cocos2d::Vec2& a, const cocos2d::Vec2& b);
    // From point p, if vec v is ccw from vec a, return positive. if cw, return negative. 0 if parrellel
    const float ccw(const cocos2d::Vec2& p, const cocos2d::Vec2& a, const cocos2d::Vec2& b);
    const bool doesPointIntersectLines(const std::list<cocos2d::Vec2>& verticies, const cocos2d::Vec2 start, const cocos2d::Vec2 end);
    const bool doesSegmentIntersects(cocos2d::Vec2 a, cocos2d::Vec2 b, cocos2d::Vec2 c, cocos2d::Vec2 d);
    void reassignLabelNumber(std::list<cocos2d::Label*>& labels);
    
    void initECS();
    
    void changeState(SCENE_STATE state);
    const float determinant(const cocos2d::Vec2& a, const cocos2d::Vec2& b);
    void drawLinesBetweenDots(std::list<cocos2d::Vec2>& verticies, cocos2d::DrawNode& drawNode, const bool drawEnd);
    void drawTriangles();
    void scaleDotSizeAndColor(const float scale, const cocos2d::Vec2& frontPoint, const cocos2d::Color3B& color, const std::string& entityPoolName);
    const bool addVertex(const cocos2d::Vec2& point, std::list<cocos2d::Vec2>& verticies, std::list<cocos2d::Label*>& verticiesLabels, const std::string& entityPoolName);
    const bool finishAddingVertex(std::list<cocos2d::Vec2>& verticies);
    const bool isPointInPolygon(std::list<cocos2d::Vec2>& verticies, const cocos2d::Vec2& point);
    const bool isPointInOrOnTriangle(const cocos2d::Vec2& a, const cocos2d::Vec2& b, const cocos2d::Vec2& c, const cocos2d::Vec2& p);
    const bool removeVertex(const std::string& entityPoolName, std::list<cocos2d::Vec2>& verticies, std::list<cocos2d::Label*>& verticiesLabels, const cocos2d::Vec2& point);
    void clearVerticies(std::list<cocos2d::Vec2>& verticies, std::list<cocos2d::Label*>& verticiesLabels, const std::string& entityPoolName);
    void finalizeVerticies();
    void toggleVertexVisibility(std::list<cocos2d::Label*>& verticiesLabels, const std::string& entityPoolName, const bool mode);
    void runEarClipping();
    const bool isEar(const cocos2d::Vec2 prevP, const cocos2d::Vec2 p, const cocos2d::Vec2 nextP);
    const bool isConvex(const cocos2d::Vec2 prevP, const cocos2d::Vec2 p, const cocos2d::Vec2 nextP);
	void makeVerticies();

    SCENE_STATE currentSceneState;
    
    std::list<cocos2d::Vec2> outerVerticies;
    std::list<cocos2d::Label*> outerVerticiesLabels;
    std::list<cocos2d::Vec2> innerVerticies;
    std::list<cocos2d::Label*> innerVerticiesLabels;
    std::list<cocos2d::Vec2> finalVerticies;
    std::list<cocos2d::Label*> finalVerticiesLabels;
	std::list<Vertex*> verticies;
	Vertex* head;
	int actualVerticiesSize;
	std::vector<cocos2d::Vec2> triangles;
    
    LabelsNode* labelsNode;
    cocos2d::Node* dotNode;
    cocos2d::DrawNode* outerLineNode;
    cocos2d::DrawNode* innerLineNode;
    cocos2d::DrawNode* finalLineNode;
    cocos2d::DrawNode* earClippingLineNode;
    DisplayBoundaryBoxNode* displayBoundaryBoxNode;
    cocos2d::Rect displayBoundary;
	cocos2d::ui::Button* infoButton;
	bool viewingInstruction;
	cocos2d::Label* instructionLabel;
	SliderLabelNode* sliderLabelNode;
	float simulationSpeedModifier;
	float elapsedTime;
	float stepDuration;
	bool finished;

	void onInfoButtonClick(cocos2d::Ref* sender);
	// On slider finishes click on slider
	void onSliderClick(cocos2d::Ref* sender);
public:
	//simple creator func
	static EarClippingScene* createScene();

	//default destructor
	~EarClippingScene() {};

	//Cocos2d Macro
	CREATE_FUNC(EarClippingScene);
};

#endif
