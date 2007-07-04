
function get_property_methods_hook(ptype, name)
  if ptype == "s" then
    return "Get" .. name, "Set" .. name
  end
end


