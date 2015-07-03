
local UIScrollView = class("UIScrollView", function()
	return cc.ClippingRegionNode:create()
end)

UIScrollView.BG_ZORDER 				= -100

UIScrollView.DIRECTION_BOTH			= 0
UIScrollView.DIRECTION_VERTICAL		= 1
UIScrollView.DIRECTION_HORIZONTAL	= 2

function UIScrollView:ctor(params)
	self.bBounce = true
	self.nShakeVal = 5
	self.direction = UIScrollView.DIRECTION_BOTH
	self.layoutPadding = {left = 0, right = 0, top = 0, bottom = 0}
	self.speed = {x = 0, y = 0}

	if not params then
		return
	end

	if params.viewRect then
		self:setViewRect(params.viewRect)
	end
	if params.direction then
		self:setDirection(params.direction)
	end
	if params.scrollbarImgH then
		self.sbH = display.newScale9Sprite(params.scrollbarImgH, 100):addTo(self)
	end
	if params.scrollbarImgV then
		self.sbV = display.newScale9Sprite(params.scrollbarImgV, 100):addTo(self)
	end

	self:addBgColorIf(params)
	self:addBgGradientColorIf(params)
	self:addBgIf(params)

	self:addNodeEventListener(cc.NODE_ENTER_FRAME_EVENT, function(...)
			self:update_(...)
		end)
end

function UIScrollView:addBgColorIf(params)
	if not params.bgColor then
		return
	end

	-- display.newColorLayer(params.bgColor)
	cc.LayerColor:create(params.bgColor)
		:size(params.viewRect.width, params.viewRect.height)
		:pos(params.viewRect.x, params.viewRect.y)
		:addTo(self, UIScrollView.BG_ZORDER)
		:setTouchEnabled(false)
end

function UIScrollView:addBgGradientColorIf(params)
	if not params.bgStartColor or not params.bgEndColor then
		return
	end

	local layer = cc.LayerGradient:create(params.bgStartColor, params.bgEndColor)
		:size(params.viewRect.width, params.viewRect.height)
		:pos(params.viewRect.x, params.viewRect.y)
		:addTo(self, UIScrollView.BG_ZORDER)
		:setTouchEnabled(false)
	layer:setVector(params.bgVector)
end

function UIScrollView:addBgIf(params)
	if not params.bg then
		return
	end

	local bg
	if params.bgScale9 then
		bg = display.newScale9Sprite(params.bg, nil, nil, nil, params.capInsets)
	else
		bg = display.newSprite(params.bg)
	end

	bg:size(params.viewRect.width, params.viewRect.height)
		:pos(params.viewRect.x + params.viewRect.width/2,
			params.viewRect.y + params.viewRect.height/2)
		:addTo(self, UIScrollView.BG_ZORDER)
		:setTouchEnabled(false)
end

function UIScrollView:setViewRect(rect)
	if "CCRect" == tolua.type(rect) then
		rect.x = rect.origin.x
		rect.y = rect.origin.y
		rect.width = rect.size.width
		rect.height = rect.size.height
	end
	self:setClippingRegion(rect)
	self.viewRect_ = rect
	self.viewRectIsNodeSpace = false

	return self
end

function UIScrollView:getViewRect()
	return self.viewRect_
end

function UIScrollView:setLayoutPadding(top, right, bottom, left)
	if not self.layoutPadding then
		self.layoutPadding = {}
	end
	self.layoutPadding.top = top
	self.layoutPadding.right = right
	self.layoutPadding.bottom = bottom
	self.layoutPadding.left = left

	return self
end

function UIScrollView:setActualRect(rect)
	self.actualRect_ = rect
end

function UIScrollView:setDirection(dir)
	self.direction = dir

	return self
end

function UIScrollView:getDirection()
	return self.direction
end

function UIScrollView:setBounceable(bBounceable)
	self.bBounce = bBounceable

	return self
end

-- 重置位置,主要用在纵向滚动时,
function UIScrollView:resetPosition()
	if UIScrollView.DIRECTION_VERTICAL ~= self.direction then
		return
	end

	local x, y = self.scrollNode:getPosition()
	local bound = self.scrollNode:getCascadeBoundingBox()
	local disY = self.viewRect_.y + self.viewRect_.height - bound.y - bound.height
	y = y + disY
	self.scrollNode:setPosition(x, y)
end

function UIScrollView:isItemInViewRect(item)
	if "userdata" ~= type(item) then
		item = nil
	end

	if not item then
		print("UIScrollView - isItemInViewRect item is not right")
		return
	end

	local bound = item:getCascadeBoundingBox()
	-- local point = cc.p(bound.x, bound.y)
	-- local parent = item
	-- while true do
	-- 	parent = parent:getParent()
	-- 	point = parent:convertToNodeSpace(point)
	-- 	if parent == self.scrollNode then
	-- 		break
	-- 	end
	-- end
	-- bound.x = point.x
	-- bound.y = point.y
	return cc.rectIntersectsRect(self:getViewRectInWorldSpace(), bound)
end

function UIScrollView:addScrollNode(node)
	self:addChild(node)
	self.scrollNode = node

	if not self.viewRect_ then
		self.viewRect_ = self.scrollNode:getCascadeBoundingBox()
		self:setViewRect(self.viewRect_)
	end
	node:setTouchSwallowEnabled(true)
	node:setTouchEnabled(true)
	node:addNodeEventListener(cc.NODE_TOUCH_EVENT, function (event)
        return self:onTouch_(event)
    end)
    node:addNodeEventListener(cc.NODE_TOUCH_CAPTURE_EVENT, function (event)
        return self:onTouchCapture_(event)
    end)

    return self
end

function UIScrollView:getScrollNode()
	return self.scrollNode
end

function UIScrollView:onScroll(listener)
	self.scrollListener_ = listener

    return self
end

-- private

function UIScrollView:calcLayoutPadding()
	local boundBox = self.scrollNode:getCascadeBoundingBox()

	self.layoutPadding.left = boundBox.x - self.actualRect_.x
	self.layoutPadding.right =
		self.actualRect_.x + self.actualRect_.width - boundBox.x - boundBox.width
	self.layoutPadding.top = boundBox.y - self.actualRect_.y
	self.layoutPadding.bottom =
		self.actualRect_.y + self.actualRect_.height - boundBox.y - boundBox.height
end

function UIScrollView:update_(dt)
	self:drawScrollBar()
end

function UIScrollView:onTouchCapture_(event)
	if ("began" == event.name or "moved" == event.name or "ended" == event.name)
		and self:isTouchInViewRect(event) then
		return true
	else
		return false
	end
end

function UIScrollView:onTouch_(event)
	if "began" == event.name and not self:isTouchInViewRect(event) then
		printInfo("UIScrollView - touch didn't in viewRect")
		return false
	end

	if "began" == event.name then
		self.prevX_ = event.x
		self.prevY_ = event.y
		self.bDrag_ = false
		local x,y = self.scrollNode:getPosition()
		self.position_ = {x = x, y = y}

		transition.stopTarget(self.scrollNode)
		self:callListener_{name = "began", x = event.x, y = event.y}

		self:enableScrollBar()
		-- self:changeViewRectToNodeSpaceIf()

		self.scaleToWorldSpace_ = self:scaleToParent_()

		return true
	elseif "moved" == event.name then
		if self:isShake(event) then
			return
		end

		self.bDrag_ = true
		self.speed.x = event.x - event.prevX
		self.speed.y = event.y - event.prevY

		if self.direction == UIScrollView.DIRECTION_VERTICAL then
			self.speed.x = 0
		elseif self.direction == UIScrollView.DIRECTION_HORIZONTAL then
			self.speed.y = 0
		else
			-- do nothing
		end

		self:scrollBy(self.speed.x, self.speed.y)
		self:callListener_{name = "moved", x = event.x, y = event.y}
	elseif "ended" == event.name then
		if self.bDrag_ then
			self.bDrag_ = false
			self:scrollAuto()

			self:callListener_{name = "ended", x = event.x, y = event.y}

			self:disableScrollBar()
		else
			self:callListener_{name = "clicked", x = event.x, y = event.y}
		end
	end
end

function UIScrollView:isTouchInViewRect(event)
	-- dump(self.viewRect_, "viewRect:")
	local viewRect = self:convertToWorldSpace(cc.p(self.viewRect_.x, self.viewRect_.y))
	viewRect.width = self.viewRect_.width
	viewRect.height = self.viewRect_.height
	-- dump(viewRect, "new viewRect:")

	return cc.rectContainsPoint(viewRect, cc.p(event.x, event.y))
end

function UIScrollView:isTouchInScrollNode(event)
	local cascadeBound = self:getScrollNodeRect()
	return cc.rectContainsPoint(cascadeBound, cc.p(event.x, event.y))
end

function UIScrollView:scrollTo(p, y)
	local x_, y_
	if "table" == type(p) then
		x_ = p.x or 0
		y_ = p.y or 0
	else
		x_ = p
		y_ = y
	end

	self.position_ = cc.p(x_, y_)
	self.scrollNode:setPosition(self.position_)
end

function UIScrollView:moveXY(orgX, orgY, speedX, speedY)
	if self.bBounce then
		-- bounce enable
		return orgX + speedX, orgY + speedY
	end

	local cascadeBound = self:getScrollNodeRect()
	local viewRect = self:getViewRectInWorldSpace()
	local x, y = orgX, orgY
	local disX, disY

	if speedX > 0 then
		if cascadeBound.x < viewRect.x then
			disX = viewRect.x - cascadeBound.x
			disX = disX / self.scaleToWorldSpace_.x
			x = orgX + math.min(disX, speedX)
		end
	else
		if cascadeBound.x + cascadeBound.width > viewRect.x + viewRect.width then
			disX = viewRect.x + viewRect.width - cascadeBound.x - cascadeBound.width
			disX = disX / self.scaleToWorldSpace_.x
			x = orgX + math.max(disX, speedX)
		end
	end

	if speedY > 0 then
		if cascadeBound.y < viewRect.y then
			disY = viewRect.y - cascadeBound.y
			disY = disY / self.scaleToWorldSpace_.y
			y = orgY + math.min(disY, speedY)
		end
	else
		if cascadeBound.y + cascadeBound.height > viewRect.y + viewRect.height then
			disY = viewRect.y + viewRect.height - cascadeBound.y - cascadeBound.height
			disY = disY / self.scaleToWorldSpace_.y
			y = orgY + math.max(disY, speedY)
		end
	end

	return x, y
end

function UIScrollView:scrollBy(x, y)
	self.position_.x, self.position_.y = self:moveXY(self.position_.x, self.position_.y, x, y)
	-- self.position_.x = self.position_.x + x
	-- self.position_.y = self.position_.y + y
	self.scrollNode:setPosition(self.position_)

	if self.actualRect_ then
		self.actualRect_.x = self.actualRect_.x + x
		self.actualRect_.y = self.actualRect_.y + y
	end
end

function UIScrollView:scrollAuto()
	if self:twiningScroll() then
		return
	end
	self:elasticScroll()
end

-- fast drag
function UIScrollView:twiningScroll()
	if self:isSideShow() then
		-- printInfo("UIScrollView - side is show, so elastic scroll")
		return false
	end

	if math.abs(self.speed.x) < 10 and math.abs(self.speed.y) < 10 then
		-- printInfo("#DEBUG, UIScrollView - isn't twinking scroll:"
		-- 	.. self.speed.x .. " " .. self.speed.y)
		return false
	end

	local disX, disY = self:moveXY(0, 0, self.speed.x*6, self.speed.y*6)

	transition.moveBy(self.scrollNode,
		{x = disX, y = disY, time = 0.3,
		easing = "sineOut",
		onComplete = function()
			self:elasticScroll()
		end})
end

function UIScrollView:elasticScroll()
	local cascadeBound = self:getScrollNodeRect()
	local disX, disY = 0, 0
	local viewRect = self:getViewRectInWorldSpace()

	-- dump(cascadeBound, "UIScrollView - cascBoundingBox:")
	-- dump(viewRect, "UIScrollView - viewRect:")

	if self.direction ~= UIScrollView.DIRECTION_VERTICAL then
		if cascadeBound.width < viewRect.width then
			disX = viewRect.x - cascadeBound.x
		else
			if cascadeBound.x > viewRect.x then
				disX = viewRect.x - cascadeBound.x
			elseif cascadeBound.x + cascadeBound.width < viewRect.x + viewRect.width then
				disX = viewRect.x + viewRect.width - cascadeBound.x - cascadeBound.width
			end
		end
	end

	if self.direction ~= UIScrollView.DIRECTION_HORIZONTAL then
		if cascadeBound.height < viewRect.height then
			disY = viewRect.y + viewRect.height - cascadeBound.y - cascadeBound.height
		else
			if cascadeBound.y > viewRect.y then
				disY = viewRect.y - cascadeBound.y
			elseif cascadeBound.y + cascadeBound.height < viewRect.y + viewRect.height then
				disY = viewRect.y + viewRect.height - cascadeBound.y - cascadeBound.height
			end
		end
	end
	
	if 0 == disX and 0 == disY then
		return
	end

	transition.moveBy(self.scrollNode,
		{x = disX, y = disY, time = 0.3,
		easing = "backout",
		onComplete = function()
			self:callListener_{name = "scrollEnd"}
		end})
end

function UIScrollView:getScrollNodeRect()
	local bound = self.scrollNode:getCascadeBoundingBox()
	-- bound.x = bound.x - self.layoutPadding.left
	-- bound.y = bound.y - self.layoutPadding.bottom
	-- bound.width = bound.width + self.layoutPadding.left + self.layoutPadding.right
	-- bound.height = bound.height + self.layoutPadding.bottom + self.layoutPadding.top

	return bound
end

function UIScrollView:getViewRectInWorldSpace()
	local rect = self:convertToWorldSpace(
		cc.p(self.viewRect_.x, self.viewRect_.y))
	rect.width = self.viewRect_.width
	rect.height = self.viewRect_.height

	return rect
end

-- 是否显示到边缘
function UIScrollView:isSideShow()
	local bound = self.scrollNode:getCascadeBoundingBox()
	if bound.x > self.viewRect_.x
		or bound.y > self.viewRect_.y
		or bound.x + bound.width < self.viewRect_.x + self.viewRect_.width
		or bound.y + bound.height < self.viewRect_.y + self.viewRect_.height then
		return true
	end

	return false
end

function UIScrollView:callListener_(event)
	if not self.scrollListener_ then
		return
	end
	event.scrollView = self

	self.scrollListener_(event)
end

function UIScrollView:enableScrollBar()
	local bound = self.scrollNode:getCascadeBoundingBox()
	if self.sbV then
		self.sbV:setVisible(false)
		transition.stopTarget(self.sbV)
		self.sbV:setOpacity(128)
		local size = self.sbV:getContentSize()
		if self.viewRect_.height < bound.height then
			local barH = self.viewRect_.height*self.viewRect_.height/bound.height
			if barH < size.width then
				-- 保证bar不会太小
				barH = size.width
			end
			self.sbV:setContentSize(size.width, barH)
			self.sbV:setPosition(
				self.viewRect_.x + self.viewRect_.width - size.width/2, self.viewRect_.y + barH/2)
		end
	end
	if self.sbH then
		self.sbH:setVisible(false)
		transition.stopTarget(self.sbH)
		self.sbH:setOpacity(128)
		local size = self.sbH:getContentSize()
		if self.viewRect_.width < bound.width then
			local barW = self.viewRect_.width*self.viewRect_.width/bound.width
			if barW < size.height then
				barw = size.height
			end
			self.sbH:setContentSize(barW, size.height)
			self.sbH:setPosition(self.viewRect_.x + barW/2,
				self.viewRect_.y + size.height/2)
		end
	end
end

function UIScrollView:disableScrollBar()
	if self.sbV then
		transition.fadeOut(self.sbV,
			{time = 0.3,
			onComplete = function()
				self.sbV:setOpacity(128)
				self.sbV:setVisible(false)
			end})
	end
	if self.sbH then
		transition.fadeOut(self.sbH,
			{time = 1.5,
			onComplete = function()
				self.sbH:setOpacity(128)
				self.sbH:setVisible(false)
			end})
	end
end

function UIScrollView:drawScrollBar()
	if not self.bDrag_ then
		return
	end
	if not self.sbV and not self.sbH then
		return
	end

	local bound = self.scrollNode:getCascadeBoundingBox()
	if self.sbV then
		self.sbV:setVisible(true)
		local size = self.sbV:getContentSize()

		local posY = (self.viewRect_.y - bound.y)*(self.viewRect_.height - size.height)/(bound.height - self.viewRect_.height)
			+ self.viewRect_.y + size.height/2
		local x, y = self.sbV:getPosition()
		self.sbV:setPosition(x, posY)
	end
	if self.sbH then
		self.sbH:setVisible(true)
		local size = self.sbH:getContentSize()

		local posX = (self.viewRect_.x - bound.x)*(self.viewRect_.width - size.width)/(bound.width - self.viewRect_.width)
			+ self.viewRect_.x + size.width/2
		local x, y = self.sbH:getPosition()
		self.sbH:setPosition(posX, y)
	end
end

function UIScrollView:addScrollBarIf()

	if not self.sb then
		self.sb = cc.DrawNode:create():addTo(self)
	end

	drawNode = cc.DrawNode:create()
    drawNode:drawSegment(points[1], points[2], radius, borderColor)
end

function UIScrollView:changeViewRectToNodeSpaceIf()
	if self.viewRectIsNodeSpace then
		return
	end

	-- local nodePoint = self:convertToNodeSpace(cc.p(self.viewRect_.x, self.viewRect_.y))
	local posX, posY = self:getPosition()
	local ws = self:convertToWorldSpace(cc.p(posX, posY))
	self.viewRect_.x = self.viewRect_.x + ws.x
	self.viewRect_.y = self.viewRect_.y + ws.y
	self.viewRectIsNodeSpace = true
end

function UIScrollView:isShake(event)
	if math.abs(event.x - self.prevX_) < self.nShakeVal
		and math.abs(event.y - self.prevY_) < self.nShakeVal then
		return true
	end
end

function UIScrollView:scaleToParent_()
	local parent
	local node = self
	local scale = {x = 1, y = 1}

	while true do
		scale.x = scale.x * node:getScaleX()
		scale.y = scale.y * node:getScaleY()
		parent = node:getParent()
		if not parent then
			break
		end
		node = parent
	end

	return scale
end

--[[--

scrollView的填充方法，可以自动把一个table里的node有序的填充到scrollview里。

~~~ lua

--填充100个相同大小的图片。
    local view =  cc.ui.UIScrollView.new({viewRect=CCRect(0,0,display.width,display.height),direction=2});
    self:addChild(view);
    local t = {};
    for i = 1, 100 do
      local png  = cc.ui.UIImage.new("box_bai.png");
      t[#t+1] = png;
      cc.ui.UILabel.new({text = i, size = 24, color = ccc3(100,100,100)})
      :align(display.CENTER, png:getContentSize().width/, png:getContentSize().height/2):addTo(png);
    end
--填充scrollview，参数itemSize为填充项的大小(填充项大小必须相同)
    view:fill(t,{itemSize=cc.size(SIZE(t[#t]))});

~~~

注意：nodes 是table结构，且一定要是{node1,node2,node3,...}不能是{a=node1,b=node2,c=node3,...}

@param nodes node集
@param params 参见fill函数头定义。

]]

function UIScrollView:fill(nodes,params)
  --多参数的继承用法,把param2的参数增加覆盖到param1中。
  local extend = function(param1,param2)
    if not param2 then
      return param1;
    end
    for k , v in pairs(param2) do
      param1[k] = param2[k];
    end
    return param1;
  end

  local params = extend({
    --自动间距
    autoGap = true,
    --宽间距
    widthGap = 0,
    --高间距
    heightGap = 0,
    --自动行列
    autoTable = true,
    --行数目
    rowCount = 3,
    --列数目
    cellCount = 3,
    --填充项大小
    itemSize = CCSize(50,50)
  },params);

  if #nodes == 0 then
    return nil;
  end

  --基本坐标工具方法
  local SIZE = function(node) return node:getContentSize(); end
  local W = function(node) return node:getContentSize().width; end
  local H = function(node) return node:getContentSize().height; end
  local S_SIZE = function(node,w,h) return node:setContentSize(CCSize(w,h)); end
  local S_XY = function(node,x,y) node:setPosition(x,y); end
  local AX = function(node) return node:getAnchorPoint().x; end
  local AY = function(node) return node:getAnchorPoint().y; end
  --三元运算符
  local CALC_3 = function(exp, result1, result2) if(exp==true)then return result1; else return result2; end end

  --创建一个容器node
  local innerContainer = display.newNode();
  --初始容器大小为视图大小
  S_SIZE(innerContainer,self:getViewRect().width,self:getViewRect().height);
  self:addScrollNode(innerContainer);
  --  innerContainer:addTo(self:getScrollNode());
  
  --如果是纵向布局
  if self.direction == cc.ui.UIScrollView.DIRECTION_VERTICAL then

    --自动布局
    if params.autoTable then
      params.cellCount = math.floor(W(self)/params.itemSize.width);
    end

    --自动间隔
    if params.autoGap then
      params.widthGap = (W(self)-(params.cellCount*params.itemSize.width))/(params.cellCount+1);
      params.heightGap = params.widthGap;
    end

    --填充量
    params.rowCount = CALC_3(#nodes%params.cellCount==0,math.floor(#nodes/params.cellCount),math.floor(#nodes/params.cellCount)+1);
    S_SIZE(innerContainer,W(self),(params.itemSize.height+params.heightGap)*params.rowCount+params.heightGap);

    for i = 1 ,#(nodes) do

      local n = nodes[i];
      local x = 0.0;
      local y = 0.0;

      x = params.widthGap + math.floor((i-1) % params.cellCount) * (params.widthGap+params.itemSize.width);
      y = H(innerContainer)-(math.floor((i-1)/params.cellCount)+1)*(params.heightGap+params.itemSize.height);
      x = x + W(n) * AX(n);
      y = y + H(n) * AY(n);

      S_XY(n,x,y);
      n:addTo(innerContainer);

    end
    --如果是横向布局
    --  elseif(self.direction==cc.ui.UIScrollView.DIRECTION_HORIZONTAL) then
  else
    if(params.autoTable)then
      params.rowCount = math.floor(H(self)/params.itemSize.height);
    end

    if(params.autoGap)then
      params.heightGap = (H(self)-(params.rowCount*params.itemSize.height))/(params.rowCount+1);
      params.widthGap = params.heightGap;
    end

    params.cellCount = CALC_3(#nodes%params.rowCount==0,math.floor(#nodes/params.rowCount),math.floor(#nodes/params.rowCount)+1);
    S_SIZE(innerContainer,(params.itemSize.width+params.widthGap)*params.cellCount+params.widthGap,H(self));

    for i = 1, #(nodes) do

      local n = nodes[i];
      local x = 0.0;
      local y = 0.0;

      --不管描点如何，总是有标准居中方式设置坐标。
      x = params.widthGap +  math.floor((i-1) / params.rowCount) * (params.widthGap+params.itemSize.width);
      y = H(innerContainer)-(math.floor((i-1) % params.rowCount) +1)*(params.heightGap+params.itemSize.height);
      x = x + W(n) * AX(n);
      y = y + H(n) * AY(n);

      S_XY(n,x,y);
      n:addTo(innerContainer);

    end

  end

end

return UIScrollView
