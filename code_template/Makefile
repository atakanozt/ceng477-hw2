all: rasterizer

rasterizer:
	g++ *.cpp -g -std=c++11 -Wall -o rasterizer

debug: rasterizer
	lldb ./rasterizer -- ../input_outputs/culling_enabled_inputs/horse_and_mug.xml

clean:
	rm -f rasterizer