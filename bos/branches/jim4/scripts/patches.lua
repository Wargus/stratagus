
function patchType(name, file, tileWidth, tileHeight, flags)
  return Map.PatchManager:newPatchType(name, file, tileWidth, tileHeight, flags)
end

function patch(name, x, y)
  return Map.PatchManager:add(name, x, y)
end


local defaultFlags = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
}

patchType("te0", "patches/te_0.png", 16, 16, defaultFlags)
patchType("te1", "patches/te_1.png", 16, 16, defaultFlags)
patchType("te2", "patches/te_2.png", 16, 16, defaultFlags)
patchType("te3", "patches/te_3.png", 16, 16, defaultFlags)
patchType("te4", "patches/te_4.png", 16, 16, defaultFlags)
patchType("te5", "patches/te_5.png", 16, 16, defaultFlags)

