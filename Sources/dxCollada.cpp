#include "dxCollada.h"
#include ".\Collada\common\colladaInc.h"
#include ".\Collada\common\ce.h"
#include ".\Collada\FS\FSfileLib.h"
#include ".\Collada\FS\FSdir.h"

dxCollada::dxCollada(void)
{

}

dxCollada::~dxCollada(void)
{

}

bool dxCollada::initLoader()
{	 
	DAE dae;
	daeDatabase *daeDB;
	FSdir dir;
	FSfile* file;
	const std::string &filename = "box.DAE";
	dir.setPath(FSdir::getProgramWD());

	file = &(FSfileLib::getByFilename(filename));
	file->getDir()->navigate();
	daeElement* root = dae.open(file->getDir()->getPath()+'\\' + file->getFilenameFull());
	daeDB = dae.getDatabase();

	if (!root) 
	{
		return 0;
	}

	daeElement* node = root->getDescendant("node");
	if (!node)
		return 0;
	else
		std::string s = "node id: " + node->getAttribute("id");
	return true;
}


void dxCollada::processCollada()
{

}