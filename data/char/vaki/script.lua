local key = constant.key
local g = global

function C_toCrouch()
	local crouchingSeq = player.currentSequence;
	if(crouchingSeq == 13 or crouchingSeq == 16) then
		return false
	end
	return true
end

function C_heightRestriction()
	local x, y = player:GetPos()
	return (y>>16) >= 70
end

function turnAround()
	g.TurnAround(15)
end

function walk(actor)
	if(g.TurnAround(15)) then
		return
	end
	if((g.GetInput() & key.any) == 0) then
		actor:GotoSequence(0)
	end
end

function crouch(actor)
	g.TurnAround(16)
	if((g.GetInput() & key.down) == 0) then
		actor:GotoSequence(14)
	end
end

function slide1(actor)
	frame = actor.currentFrame
	if(frame > 5 and frame < 12) then
		local x, y = actor:GetVel()
		local input = g.GetInput()
		if(input & key.right ~= 0) then
			x = x + 5000
		elseif(input & key.left ~= 0) then
			x = x - 5000
		end
		actor:SetVel(x,y)
	end
end

function slide2(actor)
	frame = actor.currentFrame
	if(frame > 2 and frame < 7) then
		local x, y = actor:GetVel()
		local input = global.GetInput()
		if(input & key.right ~= 0) then
			x = x + 7000
		elseif(input & key.left ~= 0) then
			x = x - 7000
		end
		actor:SetVel(x,y)
	end
end

function speedLim(actor)
	local x, y = actor:GetVel()
	if(actor:GetSide()*x > 1000*constant.multiplier) then
		actor:SetVel(1000*constant.multiplier*actor:GetSide(), y)
	end
end

print("V.Akiha script initialized")