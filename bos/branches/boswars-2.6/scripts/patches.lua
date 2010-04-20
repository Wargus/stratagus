
function patchType(name, file, tileWidth, tileHeight, flags)
  return Map.PatchManager:newPatchType(name, file, tileWidth, tileHeight, flags)
end

function patch(name, x, y)
  return Map.PatchManager:add(name, x, y)
end


function LoadPatches(path)
  local list = ListFilesInDirectory(path)
  for k,v in ipairs(list) do
    if (string.find(v, ".lua$")) then
      print("Loading patch: " .. v)
      Load(path .. v)
    end
  end
end

LoadPatches("patches/")
LoadPatches("~patches/")

