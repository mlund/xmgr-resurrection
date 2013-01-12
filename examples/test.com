echo "Create sets in graphs 0, 1, and 2"
sleep 1
echo "Arrange graphs (2x2 matrix)"
arrange 2, 2
redraw
sleep 1
echo "Make sure graphs are active"
sleep 1
activate s0 1024
activate g1.s0 1024
activate g1.s1 1024
activate g2.s1 1024
echo "Set g0.s0.x = set index"
sleep 1
s0.x = index
with g1
echo "Set g1.s0.x = g1.s1.x = set index"
sleep 1
s0.x = index
s1.x = index
g2.s1.x = index
focus g0
echo "Back to graph g0 to load s0.y"
sleep 1
s0.y = 2*cos(2*pi*x/100) + 5 * sin(2*pi*x/35)
echo "load g1.s0.y"
sleep 1
g1.s0.y = 2*cos(2*pi*x/100) + 5 * sin(2*pi*x/35)
echo "load g1.s1.y"
sleep 1
g1.s1.y = g0.s0.y/3
echo "load g2.s1.y"
sleep 1
g2.s1.y = 2*cos(2*pi*g2.s1.x/100) + 5 * sin(2*pi*g2.s1.x/35)
echo "Autoscale g0, g1, g2"
with g2
autoscale
with g0
autoscale
with g1
autoscale
with g0
echo "Mess with g0.s0.y"
s0.y=g1.s0.y - g1.s1.y
redraw
echo "Compute an fft on g0.s0"
fft(s0, 0)
echo "move g0.s1 to g3.s0"
move g0.s1 to g3.s0
with g3
autoscale
with g2
echo "More computations"
g2.s1.y = 2*g2.s1.x
autoscale
g2.s1.y = 2*cos(2*pi*g2.s1.x/100) + 5 * sin(2*pi*g2.s1.x/35)
autoscale
with g0
echo "Fit a line to g0.s0"
sleep 1
regress(s0, 1)
redraw
echo "12 pt. running average of g0.s0"
sleep 1
runavg(s0, 12)
echo "Add some random variation to s0"
sleep 1
s0.y = s0.y + rand
echo "Spline fit of g0.s0, 100 points"
sleep 1
spline(s0, s0.x.min, s0.x.max, 100)
redraw
frame fill on
frame background color 7
with g1
frame fill on
frame background color 7
with g2
frame fill on
frame background color 7
with g3
frame fill on
frame background color 7
redraw
echo "Batch file completed"
