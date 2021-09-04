#include "main_pane.h"
#include <imgui.h>
#include <imgui_stdlib.h>	
#include "imgui_utils.h"


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
			decoratedNames[i] = frameData->GetDecoratedName(i);
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
	if(!frameData->loaded)
	{
		im::Text("Load a file first.");
		im::End();
		return;
	}
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
		auto &seq = frameData->sequences[cs.seq];

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
				decoratedNames[cs.seq] = frameData->GetDecoratedName(cs.seq);
			}

			im::InputText("Function name", &seq.function);
			ImGui::InputInt("Level", &seq.props.level);
			ImGui::InputInt("Land frame", &seq.props.landFrame);
			ImGui::InputInt("Z order", &seq.props.zOrder);

			unsigned int flagIndex = -1;
			BitField("Set 1", &seq.props.flags, &flagIndex);
			/* switch (flagIndex)
			{
				case 0: Tooltip("Can move"); break;
			} */

			im::TreePop();
			im::Separator();
		}
		if(nframes >= 0)
		{
			Frame &frame = seq.frames[cs.frame];
			if(im::TreeNode("Frame data"))
			{
				DrawFrame(frame);
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

void MainPane::DrawFrame(Frame &frame)
{
	const char* const jList[] = {
		"None",
		"Frame",
		"Check loop counter",
		"Sequence"
	};

	const char* const movType[] = {
		"Ignore",
		"Set vel - Set acc",
		"Add vel - Set acc",
		"Add vel - Add acc"
	};

	const char* const states[] = {
		"Stand",
		"Crouch",
		"Air",
		"Custom"
	};

	const char* const comboType[] = {
		"Never",
		"Always",
		"Any hit",
		"Hurt only",
		"Block only"
	};

	ImGui::InputInt("Sprite id", &frame.frameProp.spriteIndex);
	ImGui::InputInt("Duration", &frame.frameProp.duration);
	ImGui::InputInt("Jump to", &frame.frameProp.jumpTo);
	ImGui::Combo("Jump type", &frame.frameProp.jumpType, jList, IM_ARRAYSIZE(jList));
	ImGui::Checkbox("Relative jump", &frame.frameProp.relativeJump);
	ImGui::InputScalar("Loop N times", ImGuiDataType_S16, &frame.frameProp.loopN,
		NULL, NULL, NULL, 0);

	ImGui::InputInt2("Vel", frame.frameProp.vel);
	ImGui::InputInt2("Acc", frame.frameProp.accel);
	ImGui::Combo("X:", &frame.frameProp.movementType[0], movType, IM_ARRAYSIZE(movType));
	ImGui::Combo("Y:", &frame.frameProp.movementType[1], movType, IM_ARRAYSIZE(movType));

	int selectedNCancel = frame.frameProp.cancelType[0];
	if(ImGui::Combo("Normal cancel", &selectedNCancel, comboType, IM_ARRAYSIZE(comboType)))
		frame.frameProp.cancelType[0] = selectedNCancel;
	int selectedSCancel = frame.frameProp.cancelType[1];
	if(ImGui::Combo("Special cancel", &selectedSCancel, comboType, IM_ARRAYSIZE(comboType)))
		frame.frameProp.cancelType[1] = selectedSCancel;

	ImGui::InputFloat2("Offset", frame.frameProp.spriteOffset);
	ImGui::Combo("State", &frame.frameProp.state, states, IM_ARRAYSIZE(states));

	ImGui::InputScalar("Counterhit", ImGuiDataType_S16, &frame.frameProp.chType,
		NULL, NULL, NULL, 0);

	unsigned int flagIndex = -1;
	BitField("Set 1", &frame.frameProp.flags, &flagIndex);
	switch (flagIndex)
	{
		case 0: Tooltip("Can move"); break;
		case 1: Tooltip("Don't transition to walking."); break;
		case 2: Tooltip("KEEP_VEL"); break;
		case 3: Tooltip("KEEP_ACC"); break;
		case 4: Tooltip("CROUCH_BLOCK"); break;
		case 5: Tooltip("STAND_BLOCK"); break;
		case 6: Tooltip("AIR_BLOCK"); break;
		case 7: Tooltip("UNUSED_SINGLE_HIT"); break;
		case 8: Tooltip("CANCELLABLE"); break;
		case 9: Tooltip("CANCEL_WHIFF"); break;
		case 10: Tooltip("UNUSED_JUMP_C_HIT"); break;
		case 11: Tooltip("UNUSED_JUMP_C_BLOCK"); break;
		case 12: Tooltip("Unused"); break;

		case 15: Tooltip("IGNORE_INPUT"); break;
		case 16: Tooltip("RESET_INFLICTED_VEL"); break;
		case 31: Tooltip("Start hit"); break;
	}

	ImGui::Separator();
	ImGui::InputTextMultiline("Frame script", &frame.frameScript);
}