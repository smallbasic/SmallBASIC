#!/usr/local/bin/sbasic -g
' CHART examples

const VN=10
dim v(1 to VN)

randomize timer

cls
' CHART values
for i=1 to VN
  v(i)=int(rnd*100)
next

color 0,15
' The 3 line-charts
for i = 0 to 3
  cls
  ? "Line-chart example - type=";i
  chart LINECHART, v(), i, 1, 17, xmax-2, ymax-2
  pause
next

' the 3 bar-charts
for i = 0 to 3
  cls
  ? "Bar-chart example - type=";i
  chart BARCHART, v, i, 1, 17, xmax-2, ymax-2
  pause
next


