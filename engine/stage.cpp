#include "stage.h"
#include "window.h"
#include <iostream>
#include <glm/ext/matrix_transform.hpp>

Stage::Stage(GfxHandler &gfx, std::filesystem::path file, std::function <void(glm::mat4&)> setViewFun):
gfx(&gfx),
setView(setViewFun)
{
	sol::state lua;
	GfxHandler::LoadLuaDefinitions(lua);
	auto result = lua.script_file(file.string());
	if(!result.valid()){
		sol::error err = result;
		std::cerr << "When loading " << file <<"\n";
		std::cerr << err.what() << std::endl;
		throw std::runtime_error("Lua syntax error.");
	}
	defId = gfx.LoadGfxFromLua(lua, file.parent_path());

	//try{
		sol::table stage = lua["stage"];
		width = stage["width"];
		height = stage["height"];
		globalScale = stage["scale"].get_or(1.f);
		sol::table layersT = stage["layers"];
		for(auto &layerEntry : layersT)
		{
			layer l;
			sol::table layerT = layerEntry.second;
			l.scale = layerT["scale"].get_or(1.f);
			l.xParallax = layerT["xParallax"].get_or(0.f);
			l.yParallax = layerT["yParallax"].get_or(l.xParallax);
			l.x = layerT["x"].get_or(0.f);
			l.y = layerT["y"].get_or(0.f);

			sol::table elementsT = layerT["elements"];
			for(auto &elementEntry : elementsT)
			{
				sol::table elementT = elementEntry.second;
				element e;
				e.drawId = elementT["id"];
				e.x = elementT["x"].get_or(0.f);
				e.y = elementT["y"].get_or(0.f);
				l.elements.push_back(e);
			}
			layers.push_back(std::move(l));
		}
/* 	}
	catch(std::exception e)
	{
		std::cerr << e.what() << "\n";
		std::cerr << "In "<<file<<"\n";
		throw std::runtime_error("Script file may be lacking non optional attributes.");
	} */
}

std::pair<int,int> Stage::GetDimensions()
{
	return {width, height};
}

void Stage::Draw(glm::mat4 &view, centerScale camera)
{
	//std::cout <<camera.scale<<"\n";
 	float wRange = width - internalWidth*camera.scale;
	//float hRange = height - internalHeight*camera.scale;
	//std::cout <<"\tRange: "<<wRange<<" - "<<hRange<<"\n";
	
	auto scaledView = view * glm::scale(glm::translate(glm::mat4(1.f), glm::vec3(-width/2,0,1)), glm::vec3(globalScale, globalScale, 1.f));
	
	for(auto &layer : layers)
	{
		float TexureScaling = (internalWidth*camera.scale)/width;
		float parallaxFactor = 1.f + layer.xParallax*((width/((float)internalWidth*camera.scale)) - 1.f);

		auto layerView = scaledView * glm::scale(
			glm::translate(glm::mat4(1.f),glm::vec3(layer.x+(camera.x*2+wRange)*(1.f-layer.xParallax), layer.y+(camera.y*2)*(1.f-layer.yParallax), 0.f)),
			glm::vec3(TexureScaling*parallaxFactor, TexureScaling*parallaxFactor, 1.f)
		);
		for(auto &e : layer.elements)
		{
			auto elementView = layerView * glm::translate(glm::mat4(1.f), glm::vec3(e.x, e.y, 0.f));
			setView(elementView);
			gfx->Draw(e.drawId, defId);
		}	
	}
}