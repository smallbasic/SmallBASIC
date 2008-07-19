'app-plug-in
'menu _Survey

button1 = "0"
radioGroup = "Excellent"
text1 = "Your name"
okButton = "0"

color 0,7
cls
button 10, 5,  -1, -1, vlabel, "How would you rate this version of SmallBASIC?", "label"
button 10, 30, -1, -1, radioGroup, "Excellent", "radio"
button 10, -1, -1, -1, radioGroup, "Fantastic", "radio"
button 10, -1, -1, -1, radioGroup, "Unbelievable", "radio"
button 10, -1, -1, -1, vlabel, "Please enter your name...", "label"
text   10, -1, 100, 20, text1

color 4,7
button 10, -1, -1, -1, okButton, "OK"
doform 

select case lower(radioGroup)
case "excellent"
  ? "Excellent answer "+text1
case "fantastic"
  ? "Fantastic answer "+text1
case "unbelievable"
  ? "Unbelievable answer "+text1
case else 
  ? "something else" + radioGroup + " " + text1
end select 
 