local key = {
	up = 0x1,
	down = 0x2,
	left = 0x4,
	right = 0x8,
	any = 0xF 
} 

function C_toCrouch()
	local crouchingSeq = actor.currentSequence();
	if(crouchingSeq == 13 or crouchingSeq == 16) then
		return false
	end
	return true
end

function C_heightRestriction()
	local x, y = actor.getPos()
	return (y>>16) >= 70
end

function turnAround()
	actor.turnAround(15)
end

function walk()
	if(actor.turnAround(15)) then
		return
	end
	if((actor.getInput() & key.any) == 0) then
		actor.gotoSequence(0)
	end
end

function crouch()
	actor.turnAround(16)
	if((actor.getInput() & key.down) == 0) then
		actor.gotoSequence(14)
	end
end

function slide1()
	frame = actor.currentFrame()
	if(frame > 5 and frame < 12) then
		local x, y = actor.getVel()
		local input = actor.getInput()
		if(input & key.right ~= 0) then
			x = x + 5000
		elseif(input & key.left ~= 0) then
			x = x - 5000
		end
		actor.setVel(x,y)
	end
end

function slide2()
	frame = actor.currentFrame()
	if(frame > 2 and frame < 7) then
		local x, y = actor.getVel()
		local input = actor.getInput()
		if(input & key.right ~= 0) then
			x = x + 7000
		elseif(input & key.left ~= 0) then
			x = x - 7000
		end
		actor.setVel(x,y)
	end
end

function speedLim()
	local x, y = actor.getVel()
	if(actor.getSide()*x > 1000*actor.multiplier) then
		actor.setVel(1000*actor.multiplier*actor.getSide(), y)
	end
end

print("V.Akiha script initialized")