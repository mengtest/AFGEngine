#include "main_pane.h"
#include <imgui.h>
#include <imgui_stdlib.h>	

MainPane::MainPane(Render* render, Framedata *framedata, FrameState &fs) : DrawWindow(render, framedata, fs),
decoratedNames(nullptr)
{
	
}

void MainPane::RegenerateNames()
{
	delete[] decoratedNames;
	
	if(frameData)
	{
		int count = frameData->sequences.size();
		decoratedNames = new std::string[count];

		for(int i = 0; i < count; i++)
		{
			decoratedNames[i] = frameData->sequences[i].name;
		}
	}
	else
		decoratedNames = nullptr;
}

void MainPane::Draw()
{	
	if(!decoratedNames)
		RegenerateNames();
		
	namespace im = ImGui;
	im::Begin("Left Pane",0);
	if (im::BeginCombo("Pattern", decoratedNames[cs.seq].c_str(), ImGuiComboFlags_HeightLargest))
	{
		auto count = frameData->sequences.size();
		for (int n = 0; n < count; n++)
		{
			const bool is_selected = (cs.seq == n);
			if (im::Selectable(decoratedNames[n].c_str(), is_selected))
			{
				cs.seq = n;
				cs.frame = 0;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				im::SetItemDefaultFocus();
		}
		im::EndCombo();
	}

	if(!frameData->sequences.empty())
	{
		auto seq = frameData->sequences[cs.seq];

		int nframes = seq.frames.size() - 1;
		if(nframes >= 0)
		{			
			float spacing = im::GetStyle().ItemInnerSpacing.x;
			im::SetNextItemWidth(im::GetWindowWidth() - 160.f);
			im::SliderInt("##frameSlider", &cs.frame, 0, nframes);
			im::SameLine();
			im::PushButtonRepeat(true);
			if(im::ArrowButton("##left", ImGuiDir_Left))
				cs.frame--;
			im::SameLine(0.0f, spacing);
			if(im::ArrowButton("##right", ImGuiDir_Right))
				cs.frame++;
			im::PopButtonRepeat();
			im::SameLine();
			im::Text("%d/%d", cs.frame+1, nframes+1);

			if(cs.frame < 0)
				cs.frame = 0;
			else if(cs.frame > nframes)
				cs.frame = nframes;
		}
		else
		{
			im::Text("This pattern has no frames.");
			if(im::Button("Add frame"))
			{
				seq.frames.push_back({});
				cs.frame = 0;
			}
			
		}

		im::BeginChild("FrameInfo", {0, im::GetWindowSize().y-im::GetFrameHeight()*3}, false, ImGuiWindowFlags_HorizontalScrollbar);
		if (im::TreeNode("Pattern data"))
		{
			if(im::InputText("Pattern name", &seq.name))
			{
				decoratedNames[cs.seq] = seq.name;
			}

			ImGui::InputText("Code name", &seq.name);
			ImGui::InputInt("fn", &seq.frameNumber);
			ImGui::InputInt("Level", &seq.level);
			ImGui::InputInt("Meter", &seq.metercost);
			ImGui::Checkbox("Loops", &seq.loops);
			ImGui::InputInt("Begin loop", &seq.beginLoop);
			ImGui::InputInt("Go to seq", &seq.gotoSeq);
			ImGui::InputInt("machine state", &seq.machineState);
			
			im::TreePop();
			im::Separator();
		}
		if(nframes >= 0)
		{
			Frame &frame = seq.frames[cs.frame];
			if(im::TreeNode("Frame data"))
			{
				ImGui::InputInt("Sprite id", &frame.spriteIndex);
				im::TreePop();
				im::Separator();
			}
			/* if (im::TreeNode("Tools"))
			{
				im::Checkbox("Make copy current frame", &copyThisFrame);
				
				if(im::Button("Append frame"))
				{
					if(copyThisFrame)
						seq->frames.push_back(frame);
					else
						seq->frames.push_back({});
				}

				im::SameLine(0,20.f); 
				if(im::Button("Insert frame"))
				{
					if(copyThisFrame)
						seq->frames.insert(seq->frames.begin()+currState.frame, frame);
					else
						seq->frames.insert(seq->frames.begin()+currState.frame, {});
				}

				im::SameLine(0,20.f);
				if(im::Button("Delete frame"))
				{
					seq->frames.erase(seq->frames.begin()+currState.frame);
					if(currState.frame >= seq->frames.size())
						cs.frame--;
				}

				im::TreePop();
				im::Separator();
			} */
		}
		im::EndChild();
	}

	im::End();
}

