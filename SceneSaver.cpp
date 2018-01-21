#include "stdafx.h"
#include "SceneSaver.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


using namespace rapidjson;

SceneSaver::SceneSaver()
{
}


SceneSaver::~SceneSaver()
{
}

//in progress
bool SceneSaver::SaveScene(SceneClass * scene, std::string savefile)
{
	std::string jsonstring = "{\"actorpath\":\"0\",\"location.x\":10,\"location.y\":10,\"location.z\":10,"
							 "\"rotation.x\":0,\"rotation.y\":0,\"rotation.z\":0"
							 ",\"scale.x\":1,\"scale.y\":1,\"scale.z\":1"
							 ",\"texoffset.x\":1, \"texoffset.y\":1, \"roughness\":1,\"metallic\":0.003"
							 ",\"objcolor.x\":1,\"objcolor.y\":1,\"objcolor.z\":1,\"objcolor.w\":1"
							 ",\"spriteanimationinterval\":5,\"usematerial\":0,\"usetex\":1,\"hasalpha\":1,\"hasshadow\":1"
							 ",\"matAlbedo\":0,\"matmetallic\":0,\"matrough\":0,\"matnormal\":0"
							 ",\"usespritesheet\":0}";

	Document doc;

	doc.Parse(jsonstring.c_str());

	Value& path = doc["actorpath"];

	Value& locx = doc["location.x"];
	Value& locy = doc["location.y"];
	Value& locz = doc["location.z"];

	Value& rotx = doc["rotation.x"];
	Value& roty = doc["rotation.y"];
	Value& rotz = doc["rotation.z"];

	Value& scax = doc["scale.x"];
	Value& scay = doc["scale.y"];
	Value& scaz = doc["scale.z"];

	Value& texx = doc["texoffset.x"];
	Value& texy = doc["texoffset.y"];

	Value& roug = doc["roughness"];
	Value& meta = doc["metallic"];

	Value& colx = doc["objcolor.x"];
	Value& coly = doc["objcolor.y"];
	Value& colz = doc["objcolor.z"];
	Value& colw = doc["objcolor.w"];

	Value& spra = doc["spriteanimationinterval"];
	Value& usem = doc["usematerial"];
	Value& uset = doc["usetex"];
	Value& hasa = doc["hasalpha"];
	Value& hass = doc["hasshadow"];

	Value& mata = doc["matAlbedo"];
	Value& matm = doc["matmetallic"];
	Value& matr = doc["matrough"];
	Value& matn = doc["matnormal"];
	Value& uspr = doc["usespritesheet"];

	

	return true;
}

bool SceneSaver::LoadScene(SceneClass * scene, std::string savefile)
{
	return false;
}
