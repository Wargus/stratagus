cd ..
find -name "*.lua" | sort > luafiles
xgettext -f luafiles -d bos -k_ -o languages/bos.pot
