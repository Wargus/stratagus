local misc = {
    _NAME = "misc",
    _VERSION = "0.01",
    _COPYRIGHT = "Stratagus Developers",
    _DESCRIPTION = "Misc lua functions for Stratagus games",
}

misc.setdefault = function(tbl, name, default)
    if tbl[name] == nil then
        tbl[name] = value
    end
    return value
end

misc.getdefault = function(tbl, name, default)
    local v = tbl[name]
    if v == nil then
        return value
    else
        return v
    end
end

misc.importStar = function(tbl, target)
    for k,v in pairs(tbl) do
        if _G[k] then
            DebugPrint("Overriding global " .. k)
        end
        _G[k] = v
    end
end

return misc
