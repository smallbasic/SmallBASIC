'app-plug-in
'menu _Survey

button1 = "0"
radio1 = "0"
radio2 = "0"
radio3 = "1"
text1 = ""
okButton = "0"

button 10, 5,  240, 20, vlabel, "How would you rate this version of SmallBASIC?", "label"
button 10, 30, 150, 20, radio1, "Excellent", "radio"
button 10, 60, 150, 20, radio2, "Fantastic", "radio"
button 10, 90, 150, 20, radio3, "Unbelievable", "radio"
button 10, 120,130, 20, vlabel, "Please enter your name...", "label"
text   10, 140, 120, 20, text1
button 10, 170, 70, 20, okButton, "OK"
doform 10,10,350,950,2

if (radio1 = "1") then
   ? "Excellent answer "+text1
elseif (radio2 = "1") then
   ? "Fantastic answer "+text1
elseif (radio3 = "1") then
   ? "Unbelievable answer "+text1
fi



