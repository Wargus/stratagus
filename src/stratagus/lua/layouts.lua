local classes = require "classes"
local class = classes.class
local misc = require "misc"

local layouts = {
   _NAME = "layouts",
   _VERSION = "0.01",
   _COPYRIGHT = "Stratagus Developers",
   _DESCRIPTION = "Box layout system for Stratagus games' lua code",
   -- globals to customize the layouts can be stored in this table
   dark = misc.getdefault(_G, "dark", Color(38, 38, 78))
   clear = misc.getdefault(_G, "clear", Color(200, 200, 120))
   black = misc.getdefault(_G, "black", Color(0, 0, 0)),
   buttonWidth = 127,
   buttonHeight = 14,
   halfButtonWidth = 60,
   halfButtonHeight = 14,
   dropDownWidth = 60,
   dropDownHeight = 8,
}

-- Call is as follows: Boxes may have children. When layout is called, boxes
-- have their reserved sizes in x,y,width,height. They can then layout their
-- children as best they can inside those. Elements report their desired sizes
-- via getWidth(), getHeight(). If nothing is desired, nil is returned, if
-- percentage of parent is desired, a percentage string is returned. When
-- layout() returns, it can be added to a guichan container by calling
-- "addWidgetTo(container)"

layouts.Element = class(function(instance)
      instance._id = nil
      instance.expands = false
      instance.x = nil
      instance.y = nil
      instance.width = nil
      instance.height = nil
end)

function Element:id(name)
   self._id = name
   return self
end

function Element:setId(container, widget)
   if self._id then
      field,idx = string.match(self._id, "(.+)%[(%d+)%]")
      if field and idx then
         local tbl = container[field] or {}
         tbl[tonumber(idx)] = widget
         container[field] = tbl
      else
         container[self._id] = widget
      end
   end
end

function Element:withWidth(value)
   self.width = value
   return self
end

function Element:withHeight(value)
   self.height = value
   return self
end

function Element:expanding()
   self.expands = true
   return self
end

function Element:getWidth()
   error("Element subclass did not define getWidth")
end

function Element:getHeight()
   error("Element subclass did not define getHeight")
end

function Element:addWidgetTo(container)
   error("Element subclass did not define addWidgetTo")
end

layouts.Box = class(layouts.Element,
            function(instance, children)
               Element.init(instance)
               instance.paddingX = 0
               instance.paddingY = 0
               instance.children = children
               for i,child in ipairs(children) do
                  -- convenience...
                  if type(child) == "string" then
                     children[i] = LLabel(child)
                  end
               end
            end
)

function Box:withPadding(p, recursive)
   if type(p) == "number" then
      self.paddingX = p
      self.paddingY = p
   else
      self.paddingX = p[1]
      self.paddingY = p[2]
   end
   if recursive then
      for i,child in ipairs(self.children) do
         if child.withPadding then
            if child.paddingX == 0 and child.paddingY == 0 then
               child:withPadding(p, recursive)
            end
         end
      end
   end
   return self
end

function Box:expanding()
   self.expands = true
   return self
end

function Box:getWidth()
   if self.width == nil then
      self:calculateMinExtent()
   end
   return self.width
end

function Box:getHeight()
   if self.height == nil then
      self:calculateMinExtent()
   end
   return self.height
end

Box.DIRECTION_HORIZONTAL = 1
Box.DIRECTION_VERTICAL = 2

function Box:calculateMinExtent()
   local w = 0
   local h = 0
   local horiz = self.direction == Box.DIRECTION_HORIZONTAL
   for i,child in ipairs(self.children) do
      local cw = child:getWidth()
      if type(cw) == "number" then
         if horiz then
            w = w + cw + self.paddingX
         else
            w = math.max(w, cw)
         end
      end
      local ch = child:getHeight()
      if type(ch) == "number" then
         if horiz then
            h = math.max(h, ch)
         else
            h = h + ch + self.paddingY
         end
      end
   end
   self.x = 0
   self.y = 0
   self.width = w + (self.paddingX * 2)
   self.height = h + (self.paddingY * 2)
   DebugPrint("Min: " .. self.width .. " x " .. self.height)
end

function Box:layout()
   DebugPrint("XY: " .. self.x .. " - " .. self.y)
   local horiz = self.direction == Box.DIRECTION_HORIZONTAL

   local padding
   local totalSpace
   if horiz then
      padding = self.paddingX
      totalSpace = self.width - padding * 2
   else
      padding = self.paddingY
      totalSpace = self.height - padding * 2
   end

   local availableSpace = totalSpace - (padding * #self.children)
   local expandingChildren = 0

   for i,child in ipairs(self.children) do
      child.parent = self
      local s
      if horiz then
         s = child:getWidth()
      else
         s = child:getHeight()
      end
      if child.expands then
         expandingChildren = expandingChildren + 1
      end
      if type(s) == "string" then
         local pct = string.match(s, "[0-9]+")
         availableSpace = math.max(availableSpace - (totalSpace * (pct / 100)), 0)
      elseif type(s) == "number" then
         availableSpace = math.max(availableSpace - s, 0)
      elseif s == nil then
         -- boxes with no preference expand to fill available space
         if not child.expands then
            child.expands = true
            expandingChildren = expandingChildren + 1
         end
      else
         error("Invalid child extent: need string, number, or nil")
      end
   end

   local childW = self.width - self.paddingX * 2
   local childH = self.height - self.paddingY * 2
   local expandingChildrenS = 0
   if expandingChildren > 0 then
      expandingChildrenS = availableSpace / expandingChildren
   end
   local xOff = self.x + self.paddingX
   local yOff = self.y + self.paddingY

   for i,child in ipairs(self.children) do
      local s
      if horiz then
         s = child:getWidth()
      else
         s = child:getHeight()
      end
      if type(s) == "string" then
         local pct = string.match(s, "[0-9]+")
         local newS = totalSpace * (pct / 100)
         if child.expands then
            newS = newS + expandingChildrenS
         end
         if horiz then
            childW = newS
         else
            childH = newS
         end
      elseif type(s) == "number" then
         if child.expands then
            s = s + expandingChildrenS
         end
         if horiz then
            childW = s
         else
            childH = s
         end
      elseif w == nil then
         if horiz then
            childW = expandingChildrenS
         else
            childH = expandingChildrenS
         end
      end

      DebugPrint(xOff, yOff, childW, childH)
      child.x = xOff
      child.y = yOff
      child.width = childW
      child.height = childH
      DebugPrint("child: " .. child.width .. "x" .. child.height .. "+" .. child.x .. "+" .. child.y)
      if horiz then
         xOff = xOff + childW + padding
      else
         yOff = yOff + childH + padding
      end
   end
end

function Box:addWidgetTo(container, sizeFromContainer)
   if sizeFromContainer then
      if sizeFromContainer == true then
         self.x = 0 -- containers are relative inside
         self.y = 0
      else
         self.x = sizeFromContainer[1]
         self.y = sizeFromContainer[2]
      end
      self.width = container:getWidth()
      self.height = container:getHeight()
      DebugPrint("startsize:" .. self.width .. "x" .. self.height .. "+" .. self.x .. "+" .. self.y)
   end
   self:layout()
   for i,child in ipairs(self.children) do
      child:addWidgetTo(container)
   end
end

layouts.HBox = class(layouts.Box,
             function(instance, children)
                Box.init(instance, children)
                instance.direction = Box.DIRECTION_HORIZONTAL
             end
)

layouts.VBox = class(layouts.Box,
             function(instance, children)
                Box.init(instance, children)
                instance.direction = Box.DIRECTION_VERTICAL
             end
)

layouts.LLabel = class(layouts.Element,
               function(instance, text, font, center, vCenter)
                  Element.init(instance)
                  instance.label = Label("" .. text)
                  if type(font) == "string" then
                     instance.label:setFont(Fonts[font])
                  else
                     instance.label:setFont(font or Fonts["large"])
                  end
                  instance.label:adjustSize()
                  instance.center = center
                  instance.vCenter = vCenter
               end
)

function LLabel:getWidth()
   return self.label:getWidth()
end

function LLabel:getHeight()
   return self.label:getHeight()
end

function LLabel:layout()
   if self.center or center == nil then -- center text by default
      self.x = self.x + (self.width - self.label:getWidth()) / 2
   end
   if self.vCenter then
      self.y = self.y + (self.height - self.label:getHeight()) / 2
   end
end

function LLabel:addWidgetTo(container)
   self:layout()
   self:setId(container, self.label)
   container:add(self.label, self.x, self.y)
end

layouts.LFiller = class(Element)

function LFiller:getWidth()
   return nil
end

function LFiller:getHeight()
   return nil
end

function LFiller:addWidgetTo(container)
   -- nothing
end

layouts.LText = class(layouts.LLabel,
              function(instance, text)
                 LLabel.init(instance, text, Fonts["game"])
              end
)

layouts.LLargeText = class(LLabel)

layouts.LImageButton = class(layouts.Element,
                     function(instance, caption, hotkey, callback)
                        Element.init(instance)
                        instance.b = ImageButton(caption)
                        instance.b:setHotKey(hotkey)
                        instance.b:setBorderSize(0)
                        instance.b.callback = callback
                        if callback then
                           instance.b:setMouseCallback(function(evtname)
                                 if evtname == "mousePress" then
                                    PlaySound("click")
                                 end
                           end)
                           instance.b:setActionCallback(function()
                                 PlaySound("click")
                                 callback()
                           end)
                        end
                     end
)

function LImageButton:getWidth()
   return self.b:getWidth()
end

function LImageButton:getHeight()
   return self.b:getHeight()
end

function LImageButton:addWidgetTo(container)
   self.b:setSize(self.width, self.height)
   self.setId(container, self.b)
   container:add(self.b, self.x, self.y)
end

layouts.LButton = class(layouts.LImageButton,
                function(instance, caption, hotkey, callback)
                  LImageButton.init(instance, caption, hotkey, callback)
                  if layouts.buttonImages then
                     local race = GetPlayerData(GetThisPlayer(), "RaceName")
                     instance.b:setNormalImage(layouts.buttonImages[race].normal)
                     instance.b:setPressedImage(layouts.buttonImages[race].pressed)
                     instance.b:setDisabledImage(layouts.buttonImages[race].disabled)
                  end
                end
)

function LButton:getWidth()
   return layouts.buttonWidth
end

function LButton:getHeight()
   return layouts.buttonHeight
end

function LButton:addWidgetTo(container)
   self.b:setSize(self.width, self.height)
   self:setId(container, self.b)
   container:add(self.b, self.x, self.y)
end

layouts.LHalfButton = class(LButton)

function LHalfButton:getWidth()
   return layouts.halfButtonWidth
end

function LHalfButton:getHeight()
   return layouts.halfButtonHeight
end

layouts.LSlider = class(layouts.Element,
                function(instance, min, max, callback)
                   Element.init(instance)
                   instance.s = Slider(min, max)
                   instance.s:setBaseColor(dark)
                   instance.s:setForegroundColor(clear)
                   instance.s:setBackgroundColor(clear)
                   instance.s.callback = callback
                   if callback then
                      instance.s:setActionCallback(function(s) callback(instance.s, s) end)
                   end
                end
)

function LSlider:getWidth()
   return nil
end

function LSlider:getHeight()
   return nil
end

function LSlider:setValue(val)
   self.s:setValue(val)
   return self
end

function LSlider:addWidgetTo(container)
   self.s:setSize(self.width, self.height)
   self:setId(container, self.s)
   container:add(self.s, self.x, self.y)
end

layouts.LListBox = class(layouts.Element,
                 function(instance, w, h, list, callback)
                    Element.init(instance)
                    instance.bq = ListBoxWidget(60, 60)
                    instance.bq:setList(list)
                    instance.bq:setBaseColor(black)
                    instance.bq:setForegroundColor(clear)
                    instance.bq:setBackgroundColor(dark)
                    instance.bq:setFont(Fonts["game"])
                    instance.bq.callback = callback
                    if callback then
                      instance.bq:setActionCallback(callback)
                    end
                    list = list or {}
                    instance.bq.itemslist = list
                    instance.width = w
                    instance.height = h
                 end
)

function LListBox:getWidth()
   return self.width
end

function LListBox:getHeight()
   return self.height
end

function LListBox:addWidgetTo(container)
   self.bq:setSize(self.width, self.height)
   self:setId(container, self.bq)
   container:add(self.bq, self.x, self.y)
end

layouts.LCheckBox = class(layouts.Element,
                  function(instance, caption, callback)
                     Element.init(instance)
                     instance.b = ImageCheckBox(caption)
                     instance.b:setForegroundColor(clear)
                     instance.b:setBackgroundColor(dark)
                     if layouts.checkboxImages then
                        local race = GetPlayerData(GetThisPlayer(), "RaceName")
                        instance.b:setUncheckedNormalImage(layouts.checkboxImages[race].uncheckedNormal)
                        instance.b:setUncheckedPressedImage(layouts.checkboxImages[race].uncheckedPressed)
                        instance.b:setCheckedNormalImage(layouts.checkboxImages[race].checkedNormal)
                        instance.b:setCheckedPressedImage(layouts.checkboxImages[race].checkedPressed)
                     end
                     instance.b.callback = callback
                     if callback then
                        instance.b:setActionCallback(function(s) callback(instance.b, s) end)
                     end
                     instance.b:setFont(Fonts["game"])
                     instance.b:adjustSize()
                  end
)

function LCheckBox:getWidth()
   return self.b:getWidth()
end

function LCheckBox:getHeight()
   return self.b:getHeight()
end

function LCheckBox:addWidgetTo(container)
   self.b:setSize(self.width, self.height)
   self:setId(container, self.b)
   container:add(self.b, self.x, self.y)
end

function LCheckBox:setMarked(flag)
   self.b:setMarked(flag)
   return self
end

layouts.LTextInputField = class(layouts.Element,
                        function(instance, text, callback)
                           Element.init(instance)
                           instance.b = TextField(text)
                           instance.b.callback = callback
                           if callback then
                              instance.b:setActionCallback(callback)
                           end
                           instance.b:setFont(Fonts["game"])
                           instance.b:setBaseColor(clear)
                           instance.b:setForegroundColor(clear)
                           instance.b:setBackgroundColor(dark)
                        end
)

function LTextInputField:getWidth()
   return nil
end

function LTextInputField:getHeight()
   return 10
end

function LTextInputField:addWidgetTo(container)
   self.b:setSize(self.width, self.height)
   self:setId(container, self.b)
   container:add(self.b, self.x, self.y)
end

layouts.LDropDown = class(layouts.Element,
                  function(instance, list, callback)
                     Element.init(instance)
                     local dd = ImageDropDownWidget()
                     if layouts.dropdownImages then
                        local race = GetPlayerData(GetThisPlayer(), "RaceName")
                        dd:setItemImage(layouts.dropdownImages[race].itemImage)
                        dd:setDownNormalImage(layouts.dropdownImages[race].normalImage)
                        dd:setDownPressedImage(layouts.dropdownImages[race].pressedImage)
                     end
                     dd:setFont(Fonts["game"])
                     dd:setList(list)
                     dd.list = list
                     dd.callback = callback
                     dd:setActionCallback(function(s) callback(dd, s) end)
                     dd:setBaseColor(dark)
                     dd:setForegroundColor(clear)
                     dd:setBackgroundColor(dark)
                     instance.dd = dd
                  end
)

function LDropDown:getWidth()
   return self.width or layouts.dropDownWidth
end

function LDropDown:getHeight()
   return layouts.dropDownHeight
end

function LDropDown:addWidgetTo(container)
   self.dd:setSize(self.width, self.height)
   self:setId(container, self.dd)
   container:add(self.dd, self.x, self.y)
end

layouts.LTextBox = class(layouts.Element,
                 function(instance, text)
                    Element.init(instance)
                    instance.b = TextBox(text)
                    instance.b:setFont(Fonts["game"])
                    instance.b:setBaseColor(clear)
                    instance.b:setForegroundColor(clear)
                    instance.b:setBackgroundColor(dark)
                    instance.scroll = ScrollArea()
                    instance.scroll:setContent(instance.b)
                    instance.scroll:setBaseColor(clear)
                    instance.scroll:setForegroundColor(clear)
                    instance.scroll:setBackgroundColor(dark)
                 end
)

function LTextBox:getWidth()
   return nil
end

function LTextBox:getHeight()
   return nil
end

function LTextBox:addWidgetTo(container)
   self.scroll:setSize(self.width, self.height)
   self.b:setSize(self.width, self.height)
   self:setId(container, self.b)
   container:add(self.scroll, self.x, self.y)
end

return layouts
