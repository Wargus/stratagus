
function patchType(name, file, tileWidth, tileHeight, flags, theme)
  if (theme == nil) then
    theme = ""
  end
  return Map.PatchManager:newPatchType(name, file, tileWidth, tileHeight, flags, theme)
end

function patch(name, x, y)
  return Map.PatchManager:add(name, x, y)
end


function LoadPatches(path)
  local list = ListFilesInDirectory(path)
  for k,v in ipairs(list) do
    if (string.find(v, ".lua$")) then
      DebugPrint("Loading patch: " .. v)
      Load(path .. v)
    end
  end
end

LoadPatches("patches/")
LoadPatches("~patches/")

