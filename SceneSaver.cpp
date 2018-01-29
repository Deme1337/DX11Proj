#include "stdafx.h"
#include "SceneSaver.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <iostream>


using namespace rapidjson;

SceneSaver::SceneSaver()
{
}


SceneSaver::~SceneSaver()
{
}

/* EXAMPLE rapidjson.org*/
/*

int main() {
// 1. Parse a JSON string into DOM.
const char* json = "{\"project\":\"rapidjson\",\"stars\":10}";
Document d;
d.Parse(json);
// 2. Modify it by DOM.
Value& s = d["stars"];
s.SetInt(s.GetInt() + 1);
// 3. Stringify the DOM
StringBuffer buffer;
Writer<StringBuffer> writer(buffer);
d.Accept(writer);
// Output {"project":"rapidjson","stars":11}
std::cout << buffer.GetString() << std::endl;
return 0;
}

*/

//in progress
bool SceneSaver::SaveScene(SceneClass * scene, std::string savefile)
{

	std::string allSceneDataJson = "";

	for (size_t i = 0; i < scene->m_Actors.size(); i++)
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
		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);

		doc.Parse(jsonstring.c_str());

		Value& path = doc["actorpath"];
		path.SetString(scene->m_Actors[i]->ActorPath.c_str(), doc.GetAllocator());

		Value& locx = doc["location.x"]; locx.SetFloat(scene->m_Actors[i]->actorMatrix.position.x);
		Value& locy = doc["location.y"]; locy.SetFloat(scene->m_Actors[i]->actorMatrix.position.y);
		Value& locz = doc["location.z"]; locy.SetFloat(scene->m_Actors[i]->actorMatrix.position.y);

		Value& rotx = doc["rotation.x"]; rotx.SetFloat(scene->m_Actors[i]->actorMatrix.rotation.x);
		Value& roty = doc["rotation.y"]; roty.SetFloat(scene->m_Actors[i]->actorMatrix.rotation.y);
		Value& rotz = doc["rotation.z"]; rotz.SetFloat(scene->m_Actors[i]->actorMatrix.rotation.z);

		Value& scax = doc["scale.x"]; scax.SetFloat(scene->m_Actors[i]->actorMatrix.size.x);
		Value& scay = doc["scale.y"]; scay.SetFloat(scene->m_Actors[i]->actorMatrix.size.y);
		Value& scaz = doc["scale.z"]; scaz.SetFloat(scene->m_Actors[i]->actorMatrix.size.z);

		Value& texx = doc["texoffset.x"]; texx.SetFloat(scene->m_Actors[i]->actorMatrix.texOffsetx);
		Value& texy = doc["texoffset.y"]; texy.SetFloat(scene->m_Actors[i]->actorMatrix.texOffsety);

		Value& roug = doc["roughness"]; roug.SetFloat(scene->m_Actors[i]->actorMatrix.roughness);
		Value& meta = doc["metallic"];  meta.SetFloat(scene->m_Actors[i]->actorMatrix.metallic);

		Value& colx = doc["objcolor.x"]; colx.SetFloat(scene->m_Actors[i]->actorMatrix.objColor.x);
		Value& coly = doc["objcolor.y"]; coly.SetFloat(scene->m_Actors[i]->actorMatrix.objColor.y);
		Value& colz = doc["objcolor.z"]; colz.SetFloat(scene->m_Actors[i]->actorMatrix.objColor.z);
		Value& colw = doc["objcolor.w"]; colw.SetFloat(scene->m_Actors[i]->actorMatrix.objColor.w);

		Value& spra = doc["spriteanimationinterval"]; spra.SetInt(scene->m_Actors[i]->SpriteAnimationInterval);
		Value& usem = doc["usematerial"]; usem.SetInt(scene->m_Actors[i]->useMaterial);
		Value& uset = doc["usetex"]; uset.SetInt(scene->m_Actors[i]->UseTextures);
		Value& hasa = doc["hasalpha"]; hasa.SetInt(scene->m_Actors[i]->HasAlpha);
		Value& hass = doc["hasshadow"]; hass.SetInt(scene->m_Actors[i]->HasShadow);

		Value& mata = doc["matAlbedo"]; mata.SetString("nan", doc.GetAllocator());
		Value& matm = doc["matmetallic"]; matm.SetString("nan", doc.GetAllocator());
		Value& matr = doc["matrough"]; matr.SetString("nan", doc.GetAllocator());
		Value& matn = doc["matnormal"]; matn.SetString("nan", doc.GetAllocator());

		if (scene->m_Actors[i]->objectMaterial->GetTexture("albedo")->textureName.size() > 0)
		{
			mata.SetString(scene->m_Actors[i]->objectMaterial->GetTexture("albedo")->textureName.c_str(), doc.GetAllocator());
		}
		if (scene->m_Actors[i]->objectMaterial->GetTexture("normal")->textureName.size() > 0)
		{
			matn.SetString(scene->m_Actors[i]->objectMaterial->GetTexture("normal")->textureName.c_str(), doc.GetAllocator());
		}
		if (scene->m_Actors[i]->objectMaterial->GetTexture("specular")->textureName.size() > 0)
		{
			matm.SetString(scene->m_Actors[i]->objectMaterial->GetTexture("specular")->textureName.c_str(), doc.GetAllocator());
		}
		if (scene->m_Actors[i]->objectMaterial->GetTexture("roughness")->textureName.size() > 0)
		{
			matr.SetString(scene->m_Actors[i]->objectMaterial->GetTexture("roughness")->textureName.c_str(), doc.GetAllocator());
		}

		Value& uspr = doc["usespritesheet"]; uspr.SetInt(scene->m_Actors[i]->UseAnimatedSpriteSheet);

		doc.Accept(writer);

		allSceneDataJson.append(buffer.GetString());
		allSceneDataJson.append("////");
	}


	ofstream saveFile;

	saveFile.open("Models\\scenedata.txt");

	if (!saveFile.is_open())
	{
		int e = saveFile.exceptions();
		MessageBox(NULL, std::to_wstring(e).c_str(), L"ERROR", MB_OK);
		return false;
	}
	saveFile << allSceneDataJson;

	saveFile.close();

	return true;
}

bool SceneSaver::LoadScene(SceneClass * scene, std::string savefile)
{
	return false;
}
