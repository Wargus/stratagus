cd ..
find -name "*.lua" > luafiles
xgettext -f luafiles -d bos -k_ -o languages/bos.pot_
