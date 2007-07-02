cd ..
xgettext -d bos -k_ -o languages/bos.pot `find -name "*.lua" | sort`
cd engine
xgettext -d engine -C -k_ -o ../languages/engine.pot `find -name "*.cpp" -or -name "*.h" | sort`
