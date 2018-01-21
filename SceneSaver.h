#pragma once

#ifndef SCENESAVER_H
#define SCENESAVER_H

#include "SceneClass.h"
#include "Actor.h"
#include "Model.h"
#include "Material.h"

#include "rapidjson\rapidjson.h"
#include <iostream>

class SceneSaver
{
public:
	SceneSaver();
	~SceneSaver();


	bool SaveScene(SceneClass* scene, std::string savefile);

	bool LoadScene(SceneClass* scene, std::string savefile);
};


#endif // !SCENESAVER_H