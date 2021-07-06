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

			ImGui::InputInt("fn", &seq.frameNumber);
			ImGui::InputInt("Level", &seq.level);
			ImGui::InputInt("Meter", &seq.metercost);
			ImGui::Checkbox("Loops", &seq.loops);
			ImGui::InputInt("Begin loop", &seq.beginLoop);
			ImGui::InputInt("Go to seq", &seq.gotoSeq);
			const char* const states[] = {
				"GROUNDED",
				"AIRBORNE",
				"BUSY_GRND",
				"BUSY_AIR",
				"PAIN_GRND",
				"PAIN_AIR"
			};
			im::Combo("Machine state", &seq.machineState, states, IM_ARRAYSIZE(states));
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
		
		if (im::TreeNode("ACT"))
		{
			for( int i = 0; i < 32; i++)
			{
				im::PushID(i); 
				im::Text("A %s", frameData->motionListDataA[i].motionStr.c_str());
				im::SameLine();
				im::Text("G %s", frameData->motionListDataG[i].motionStr.c_str());
				im::PopID();
			};
			im::TreePop();
			im::Separator();
		}
		im::EndChild();
	}

	im::End();
}

void MainPane::DrawFrame(Frame &frame)
{
	ImGui::InputInt("Sprite id", &frame.spriteIndex);
	ImGui::InputInt("Duration", &frame.frameProp.duration);
	ImGui::InputFloat2("Vel", frame.frameProp.vel);
	ImGui::InputFloat2("Acc", frame.frameProp.accel);
	ImGui::InputInt4("Dmg", frame.frameProp.damage);
	ImGui::InputFloat("Proration", &frame.frameProp.proration);
	ImGui::InputInt("Hitstun", &frame.frameProp.hitstun);
	ImGui::InputInt("Blockstun", &frame.frameProp.blockstun);
	ImGui::InputInt("Hitstun", &frame.frameProp.hitstun);
	ImGui::InputInt("CH stop", &frame.frameProp.ch_stop);
	ImGui::InputInt("Hitstop", &frame.frameProp.hitstop);
	ImGui::InputFloat2("Push", frame.frameProp.push);
	ImGui::InputFloat2("Pushback", frame.frameProp.pushback);
	ImGui::InputInt("Pain", &frame.frameProp.painType);
	ImGui::InputFloat2("Offset", frame.frameProp.spriteOffset);

	const char* const states[] = {
		"STANDING",
		"CROUCHED",
		"AIRBORNE",
		"OTG"
	};
	ImGui::Combo("State", &frame.frameProp.state, states, IM_ARRAYSIZE(states));

	unsigned int flagIndex = -1;
	BitField("Set 1", &frame.frameProp.flags, &flagIndex);
	switch (flagIndex)
	{
		case 0: Tooltip("FRICTION"); break;
		case 1: Tooltip("GRAVITY"); break;
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
	}
}