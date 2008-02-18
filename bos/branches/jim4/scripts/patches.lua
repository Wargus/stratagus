
function patchType(name, file, tileWidth, tileHeight, flags)
  return Map.PatchManager:newPatchType(name, file, tileWidth, tileHeight, flags)
end

function patch(name, x, y)
  return Map.PatchManager:add(name, x, y)
end


function LoadPatches(list, path)
  for k,v in ipairs(list) do
    if (string.find(v, ".lua$")) then
      print("Loading patch: " .. v)
      Load(path .. v)
    end
  end
end

local list
local path

path = "scripts/patches/"
list = ListFilesInDirectory(path)
LoadPatches(list, path)

path = "~patches/"
list = ListFilesInDirectory(path)
LoadPatches(list, path)

