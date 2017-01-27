#ifndef QUADTREE_LINE_NODE_H
#define QUADTREE_LINE_NODE_H

#include "cocos2d.h"

/**
*	@class LineNode
*	@brief Inherits cocos2d Node.
*	
*	This class derives from Cocos2d's Node class.
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
	cocos2d::DrawNode* dispalyBoundaryDrawNode;
	cocos2d::DrawNode* quadTreeSubDivisionDrawNode;
public:
	//simple creator func
	static QuadTreeLineNode* createNode();

	//Default destructor
	~QuadTreeLineNode() = default;

	//Cocos2d Macro
	CREATE_FUNC(QuadTreeLineNode);

	// Draw box
	void drawDisplayBoundaryBox(const cocos2d::Rect& displayBoundary);
};

#endif