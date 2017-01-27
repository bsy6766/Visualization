#ifndef QUADTREE_H
#define QUADTREE_H

#ifndef VISUALIZATION
#define VISUALIZATION
#endif

#include "cocos2d.h"
#include <list>
#include "ECS.h"

using namespace ECS;

//The maximum/minimum level of tree it can sub divide.
#define MAX_LEVEL 10
#define MIN_LEVEL 5
#define DEFAULT_LEVEL 6
//If set to 1, clear() will delete sub quadtree instances. If set to 0, clear() will not delete sub quadtree and reuse whenever it needs
#define CLEAN_UP_SUB_DIV 1

/**
*	@class QTree
*	@brief Simple implementation of QuadTree with cocos2d-x. 
*	@note This quad tree manages cocos2d-x bounding boxes instaead of entity.
*/

class QuadTree
{
private:
	static int CurrentMaxLevelSet;
	//The capacity of leaf tree can have before it sub divides
	static const int MAX_ENTITY_TO_SUBDIVIDE = 3;

	//4 divided areas
	QuadTree* nw;	//North West
	QuadTree* ne;	//North East
	QuadTree* sw;	//South West
	QuadTree* se;	//South East

	//The tree's level. Starts from 0
	int level;

	//True if this tree is leaf, which means has no sub division
	bool leaf;

	//True if this tree never modified (= fresh)
	bool clean;

	//The boundary of this quad tree.
	cocos2d::Rect boundary;

	//Boxes
	std::list<Entity*> datas;

	//sub divide current quad tree
	void subDivide();
public:
	//Contructor
	QuadTree(const cocos2d::Rect& boundary, int level);

	//Destructor. Calls clear()
	~QuadTree();

	//Insert bounding box to tree
	bool insert(Entity* entity);

	//Set boundary
	void setBoundary(const cocos2d::Rect& boundary);

	//Set level
	void setLevel(const int level);

	//Clears current tree. Deletes all sub division and bounding boxes.
	void clear();
	
	// These are for Visualization project.
#ifdef VISUALIZATION
	void showLines();
	void drawLines();
	static cocos2d::DrawNode* lineDrawNode;
#endif

	// Increase quad tree level
	void increaseLevel();
	// Decrease quad tree level
	void decreaseLevel();
	// Get current level setting
	const int getCurrentLevelSetting();

	/**
	*	@brief Query all bounding boxes that intersects querying area.
	*
	*	@param queryingArea An area to query.
	*	@param nearEntities Container to retreive queried bounding boxes
	*/
	void queryAllEntities(const cocos2d::Rect& queryingArea, std::list<Entity*>& nearEntities);
};

#endif