option base 1

ReceiveBuffer = ""
ComPort = "/dev/ttyACM0"
ComSpeed = "115200"
FileName = "serialUSB_receive_string.bas"
SendString = ""
NumberLines = floor(YMAX / TextHeight("Qly")) - 1
NumberRows  = floor(YMAX / TextWidth("W"))
SendHistory = []
SendHistoryIndex = 0
isConnected = 0
SerielID = 1
ConnectionTimer = 0

w = window()

const ESC      = Chr(27)
const KeyESC   = ESC
const KeyLeft  = ESC + Chr(0x04)
const KeyRight = ESC + Chr(0x05)
const KeyUp    = ESC + Chr(0x09)
const KeyDown  = ESC + Chr(0x0A)

const KeyBackspace = Chr(0x08)
const KeyReturn = Chr(0x0D)
const KeyF1 = ESC + Chr(0xF0 + 1)
const KeyF2 = ESC + Chr(0xF0 + 2)
const KeyF3 = ESC + Chr(0xF0 + 3)
const KeyF4 = ESC + Chr(0xF0 + 4)
const KeyF5 = ESC + Chr(0xF0 + 5)
const KeyF6 = ESC + Chr(0xF0 + 6)
const KeyF9 = ESC + Chr(0xF0 + 9)
const KeyF10 = ESC + Chr(0xF0 + 10)

Connect()
PrintGUI()

while(1)
    'CheckIsConnected()
    if(!isConnected) then Connect()
    k = inkey
    if(len(k))
        select case k
            case KeyReturn
                Send(SendString)
                SendHistory << SendString
                SendHistoryIndex = ubound(SendHistory)
                SendString = ""
            case KeyEsc
                SendString = ""
            case KeyLeft
            case KeyRight
            case KeyDown
                SendHistoryIndex = iff(SendHistoryIndex < ubound(SendHistory), SendHistoryIndex + 1, SendHistoryIndex)
                SendString = SendHistory[SendHistoryIndex]
            case KeyUp
                SendHistoryIndex = iff(SendHistoryIndex > 0, SendHistoryIndex - 1, 0)
                SendString = SendHistory[SendHistoryIndex]    
            case KeyBackspace
                SendString = chop(SendString)
            case KeyF1
                color 15,0
                locate NumberLines - 1,0
                print "\e[K";
                Input "Enter COM port: ", ComPort
                if(isConnected) then close #1
                isConnected = 0
            case KeyF2
                color 15,0
                locate NumberLines - 1,0
                print "\e[K";
                Input "Enter COM speed: ", ComSpeed
                if(isConnected) then close #1
                isConnected = 0
            case KeyF3
                'color 15,0
                'locate NumberLines - 1,0
                'print "\e[K";
                'Input "Enter file name: ", FileName
                showpage(1)
                FileName = FileSelectDialog(0, 50, 50, XMAX - 100, YMAX - 100, 0)
            case KeyF4
                tsave "teensylog_" + ticks() + ".txt", ReceiveArray
            case KeyF6
                UploadFile()
            case KeyF9
                if(isConnected)
                  close #1
                  isConnected = 0
                endif
                ReceiveBuffer = ""
                ReceiveArray = []
            case KeyF10
                if(isConnected) then close #1
                stop
           case else
                SendString = SendString + k
        end select
        PrintGUI()
    endif
    
    if(isConnected)
      l = lof(1)
      if(l)
        ReceiveBuffer =  ReceiveBuffer + INPUT(l, 1)
        split ReceiveBuffer, "\n", ReceiveArray
        if(len(ReceiveArray) > NumberLines - 1)
          n = len(ReceiveArray) - NumberLines + 1
          delete ReceiveArray, 1, n
        endif
        PrintGUI()
      endif
    endif
    
    delay(20)
    showpage
wend

sub CheckIsConnected()
  local result
  
  if(!isConnected) then return

  result = lof(1)
  if(result != isConnected)
    isConnected = result
    PrintGUI
  endif
end

sub Connect()
  if(isConnected) then return
  if(ticks() - ConnectionTimer < 500) then return
  ConnectionTimer = ticks()
  
  try
    open ComPort+":115200" as #1
    isConnected = 1
  catch
    isConnected = 0
  end try
  PrintGUI()
end


sub Send(s)
  ReceiveBuffer = ReceiveBuffer + "\e[33m" + "> " + s + "\e[30m\n"
  print #1, s
end

sub UploadFile()
    open FileName for INPUT as #2
    while(!eof(2))
      input #2, c
      print #1, c
    wend
    close #2
    ReceiveBuffer = ReceiveBuffer + "\e[32mFile uploaded\e[30m\n"
end

sub PrintGUI()
    color 7,1
    cls
    
    for s in ReceiveArray
      print "\e[0m";
      color 14,1
      print s
    next
    
    print "\e[0m"
    
    color 0, 7
    locate 0, 0
    print "TEENSY Serial Terminal v1.0     |\e[35mF1\e[30m Port|\e[35mF2\e[30m Speed|\e[35mF3\e[30m File|\e[35mF4\e[30m Save|\e[35mF6\e[30m Upload|\e[35mF9\e[30m Reset|\e[35mF10\e[30m Quit\e[K"
    
    color 15, 0
    locate NumberLines - 1, 0
    print "Send: "; SendString; "\e[K";"\e[7m \e[27m"
    
    color 0, 7
    locate NumberLines, 0
    print "|"; ComPort;"|"; ComSpeed; "|"; right(FileName,30); "|";
    if(isConnected) then
      print "\e[32mConnected\e[K";
    else
      print "\e[31mDisconnected\e[K";
    endif
end


func FileSelectDialog(Type, x, y, w, h, ButtonSize)
' Displays a file select dialog. If type is 0 (LOAD) then the function
' will always return a existing file. If type is 1 (SAVE) the returned
' file might exist. If user cancel the file dialog, the function will
' return -1
'
' Type        : 0 = Load; 1 = Save
' x,y         : position on screen in pixel
' w,h         : width and height in pixel
' ButtonSize  : size of the buttons in pixel, 0 = auto
' Return value: file selected by user

    local listbox, textbox, buttons, f, GUI, ReturnValue = 0, win
    local GetFileList, cmpfunc_strings, index, Directory, FileList
    local BGColor, TextColor, ElementColor, GraphicCursor, ii
    
    BGColor = rgb(100, 100, 100)
    TextColor = rgb(255, 255, 255)
    ElementColor = rgb(80, 80, 80)
    
    x = x + 2
    y = y + 2
    w = w - 4
    h = h - 6

    win = window()
    win.graphicsScreen2()
    color 15,0
    cls
    color TextColor, BGColor
       
    
    Directory = cwd()
    
    if(ButtonSize == 0) then ButtonSize = 3 * Textwidth("W")    
    
    func cmpfunc_strings(x, y)
        x = lower(x)
        y = lower(y)
        
        if x == y
            return 0
        elseif x > y
            return 1
        else
            return -1
        endif
    end
    
    func GetFileList()
        local FileList
        FileList = files("*")
        
        for ii = 1 to ubound(FileList)
            if(isdir(FileList[ii])) then FileList[ii] = enclose(FileList[ii], "[]")                
        next
        
        sort FileList use cmpfunc_strings(x, y)
        
        insert FileList, 1, "[..]"
             
        return FileList
    end    

    listbox.type = "listbox"
    listbox.x = x
    listbox.y = y + 2*ButtonSize
    listbox.height = h - 3.5 * ButtonSize
    listbox.width = w - ButtonSize
    listbox.color = TextColor
    listbox.backgroundColor = ElementColor
    listbox.value = GetFileList()
    listbox.selectedIndex = -1
    listbox.length = ubound(listbox.value) - 1
    listbox.noFocus = 1
    f.inputs << listbox
    
    textbox.type = "text"
    textbox.x = x
    textbox.y = y + ButtonSize
    textbox.width = w - ButtonSize
    textbox.value = cwd
    textbox.color = TextColor
    textbox.backgroundColor = ElementColor
    textbox.length = 500 ' number of characters
    textbox.noFocus = 0
    f.inputs << textbox
    
    buttons.type = "button"
    buttons.x = x + w - ButtonSize
    buttons.y = y + 2*ButtonSize
    buttons.width = ButtonSize
    buttons.height = ButtonSize
    buttons.label = "/\\"
    buttons.backgroundcolor = ElementColor
    buttons.color = TextColor
    buttons.noFocus = 1
    f.inputs << buttons

    buttons.type = "button"
    buttons.x = x + w - ButtonSize
    buttons.y = y + h - 2.5*ButtonSize
    buttons.width = ButtonSize
    buttons.height = ButtonSize
    buttons.label = "\\/"
    buttons.backgroundcolor = ElementColor
    buttons.color = TextColor
    buttons.noFocus = 1
    f.inputs << buttons

    buttons.type = "button"
    buttons.x = x + w - 6.5 * ButtonSize
    buttons.y = y + h - ButtonSize
    buttons.width = ButtonSize * 3
    buttons.height = ButtonSize
    buttons.label = "SELECT"
    buttons.backgroundcolor = ElementColor
    buttons.color = TextColor
    buttons.noFocus = 1
    f.inputs << buttons

    buttons.type = "button"
    buttons.x = x + w - 3 * ButtonSize
    buttons.y = y + h - ButtonSize
    buttons.width = ButtonSize * 3
    buttons.height = ButtonSize
    buttons.label = "CANCEL"
    buttons.backgroundcolor = ElementColor
    buttons.color = TextColor
    buttons.noFocus = 1
    f.inputs << buttons    
    
    rect x - 2, y - 2 STEP w + 4, h + 6 color BGColor filled
    at x,y
    if(Type == 0)
        print "LOAD FILE:"
    else
        print "SAVE FILE:"
    endif
    
    GUI = form(f)
    
    while(ReturnValue == 0)
      
      GUI.doEvents()
      
      ' Check for value of the active input field
      if (len(GUI.value) > 0) then

        select case GUI.value
            case "/\\"
                if(GUI.inputs[1].selectedIndex > 0)
                    GUI.inputs[1].selectedIndex = GUI.inputs[1].selectedIndex - 1
                    index = GUI.inputs[1].selectedIndex + 1
                    
                    f = disclose(GUI.inputs[1].value[index], "[]")
                    if(f != "")         ' directory                  
                        GUI.inputs[2].value = cwd + f
                    else                ' file
                        GUI.inputs[2].value = cwd + GUI.inputs[1].value[index]
                    endif
                    
                    GUI.refresh(0)
                endif
            case "\\/"
                if(GUI.inputs[1].selectedIndex < GUI.inputs[1].length)
                    GUI.inputs[1].selectedIndex = GUI.inputs[1].selectedIndex + 1
                    
                    index = GUI.inputs[1].selectedIndex + 1
                    f = disclose(GUI.inputs[1].value[index], "[]")
                    
                    if(f != "")         ' directory                  
                        GUI.inputs[2].value = cwd + f
                    else                ' file
                        GUI.inputs[2].value = cwd + GUI.inputs[1].value[index]
                    endif
                    
                    GUI.refresh(0)
                endif
            case "SELECT"
                if(isdir(GUI.inputs[2].value))
                    chdir(GUI.inputs[2].value)
                    GUI.inputs[1].value = GetFileList()
                    GUI.refresh(0)
                elseif(isfile(GUI.inputs[2].value))
                    ReturnValue = GUI.inputs[2].value
                elseif(Type == 1)                     ' if Save
                    ReturnValue = GUI.inputs[2].value
                else
                    win.alert("Not a valid file or directory")
                endif
            case "CANCEL"
                ReturnValue = -1
            case else   ' user clicked on a file in the file listbox
                f = disclose(GUI.value, "[]")
                if(f != "") then 
                    chdir f
                    GUI.inputs[1].value = GetFileList()
                    GUI.inputs[2].value = cwd
                    GUI.refresh(0)
                else
                    index = GUI.inputs[1].selectedIndex + 1
                    GUI.inputs[2].value = cwd + GUI.inputs[1].value[index]
                    GUI.refresh(0)
                endif
                    
        end select
        
        
      endif
      
        
    wend

    GUI.close()
    
    chdir Directory
    win.graphicsScreen1()
    
    return ReturnValue

end

