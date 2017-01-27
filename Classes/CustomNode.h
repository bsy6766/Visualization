#ifndef CUSTOM_NODE_H
#define CUSTOM_NODE_H

#include "cocos2d.h"

/**
*	@class QuadTreeLineNode
*	@brief Inherits cocos2d Node.
*
*	Purpose of this class is to create node that has all the quadtree line
*/

class QuadTreeLineNode : public cocos2d::Node
{
private:
	friend class QuadTreeScene;

	//Default contructor
	QuadTreeLineNode() = default;

	//cocos2d virtual
	virtual bool init() override;

	// Draw node
	cocos2d::DrawNode* drawNode;
public:
	//simple creator func
	static QuadTreeLineNode* createNode();

	//Default destructor
	~QuadTreeLineNode() = default;

	//Cocos2d Macro
	CREATE_FUNC(QuadTreeLineNode);
};

/**
 *	@class DisplayBoundaryBoxNode
 *	@brief Inherits cocos2d Node.
 *
 *	Purpose of this class is to create node that draws display boundary box
 */

class DisplayBoundaryBoxNode : public cocos2d::Node
{
private:
    friend class QuadTreeScene;
    friend class FlockingScene;
    
    //Default contructor
    DisplayBoundaryBoxNode() = default;
    
    //cocos2d virtual
    virtual bool init() override;
    
    // Draw node
    cocos2d::DrawNode* drawNode;
public:
    //simple creator func
    static DisplayBoundaryBoxNode* createNode();
    
    //Default destructor
    ~DisplayBoundaryBoxNode() = default;
    
    //Cocos2d Macro
    CREATE_FUNC(DisplayBoundaryBoxNode);
    
    // Draw box and modify display boundary to fit center left of screen
    void drawDisplayBoundaryBox(cocos2d::Rect& displayBoundary);
};

#endif
