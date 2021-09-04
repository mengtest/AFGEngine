#include "stage.h"
#include "window.h"
#include <iostream>
#include <glm/ext/matrix_transform.hpp>

Stage::Stage(GfxHandler &gfx, std::filesystem::path file, std::function <void(glm::mat4&)> setViewFun):
gfx(&gfx),
setView(setViewFun)
{
	sol::state lua;
	//Blending
	lua["additive"] = additive;
	lua["normal"] = normal;
	//Movement type
	lua["horizontal"] = horizontal;
	lua["vertical"] = vertical;
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
			layer l{};
			sol::table layerT = layerEntry.second;
			l.scale = layerT["scale"].get_or(1.f);
			l.xScroll = layerT["xScroll"].get_or(0.f);
			l.xParallax = layerT["xParallax"].get_or(0.f);
			l.yParallax = layerT["yParallax"].get_or(l.xParallax);
			l.x = layerT["x"].get_or(0.f);
			l.y = layerT["y"].get_or(0.f);
			l.mode = layerT["mode"].get_or(0);


			sol::table elementsT = layerT["elements"];
			for(auto &elementEntry : elementsT)
			{
				sol::table elementT = elementEntry.second;
				element e{};
				e.drawId = elementT["id"];
				e.x = elementT["x"].get_or(0.f);
				e.y = elementT["y"].get_or(0.f);

				sol::optional<sol::table> movementOpt = elementT["movement"];
				if(movementOpt)
				{
					sol::table mov = movementOpt->as<sol::table>();
					e.movementType = mov["type"].get_or(0);
					e.accelX = mov["accelX"].get_or(0.f);
					e.accelY = mov["accelY"].get_or(0.f);
					e.speedX = mov["speedX"].get_or(0.f);
					e.speedY = mov["speedY"].get_or(0.f);
					e.centerX = mov["centerX"].get_or(0.f);
					e.centerY = mov["centerY"].get_or(0.f);
				}

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
	int blendMode = -1;
	
	auto scaledView = view * glm::scale(glm::translate(glm::mat4(1.f), glm::vec3(-width/2,0,1)), glm::vec3(globalScale, globalScale, 1.f));
	
	for(auto &layer : layers)
	{
		float TexureScaling = (internalWidth*camera.scale)/width;
		float parallaxFactor = 1.f + layer.xParallax*((width/((float)internalWidth*camera.scale)) - 1.f);

		if(blendMode != layer.mode)
		{
			blendMode = layer.mode;
			if(layer.mode == 1)
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			else
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		auto parallaxedView = glm::translate(glm::scale(
			glm::translate(glm::mat4(1.f),glm::vec3((camera.x*2+wRange)*(1.f-layer.xParallax), (camera.y*2)*(1.f-layer.yParallax), 0.f)),
			glm::vec3(TexureScaling*parallaxFactor, TexureScaling*parallaxFactor, 1.f)
		), glm::vec3(layer.x, layer.y, 0.f));
		auto layerView = scaledView * parallaxedView;
		for(auto &e : layer.elements)
		{
			if(e.movementType & horizontal)
			{
				e.x += e.speedX;
				if(e.x > e.centerX)
					e.speedX -= e.accelX;
				else //if(e.speedY > -e.maxY)
					e.speedX += e.accelX;
			}
			if(e.movementType & vertical)
			{
				e.y += e.speedY;
				if(e.y > e.centerY)
					e.speedY -= e.accelY;
				else //if(e.speedY > -e.maxY)
					e.speedY += e.accelY;
			}
		}
		auto drawElements = [&]()
		{
			for(auto &e : layer.elements)
			{
				auto elementView = layerView * glm::translate(glm::mat4(1.f), glm::vec3(e.x, e.y, 0.f));
				setView(elementView);
				gfx->Draw(e.drawId, defId);
			}
		};
		drawElements();
		if(layer.xScroll != 0)
		{
			float w = (float)(width)/globalScale;
			layer.x += layer.xScroll;
			if(layer.x > w)
				layer.x -= w;
			else if(layer.x < 0)
				layer.x += w;
			
			layerView = scaledView * glm::translate(parallaxedView, glm::vec3(-w,0,0));
			drawElements();
		}
	}
}