object FSearch: TFSearch
  Left = 349
  Top = 373
  BorderStyle = bsDialog
  Caption = 'Find'
  ClientHeight = 193
  ClientWidth = 351
  Color = clBtnFace
  Font.Charset = GREEK_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Verdana'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 10
    Width = 40
    Height = 13
    Caption = 'Search'
  end
  object Label2: TLabel
    Left = 8
    Top = 42
    Width = 45
    Height = 13
    Caption = 'Replace'
  end
  object btnSearch: TButton
    Left = 8
    Top = 160
    Width = 81
    Height = 25
    Caption = 'Search'
    Default = True
    TabOrder = 7
    OnClick = btnSearchClick
  end
  object btnCancel: TButton
    Left = 264
    Top = 160
    Width = 81
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    TabOrder = 9
    OnClick = btnCancelClick
  end
  object txtSearch: TEdit
    Left = 64
    Top = 8
    Width = 281
    Height = 21
    BevelKind = bkFlat
    BorderStyle = bsNone
    TabOrder = 0
  end
  object chkBack: TCheckBox
    Left = 8
    Top = 80
    Width = 153
    Height = 17
    Caption = 'Backward'
    TabOrder = 2
  end
  object chkCase: TCheckBox
    Left = 8
    Top = 104
    Width = 145
    Height = 17
    Caption = 'Case Sensitive'
    TabOrder = 3
  end
  object chkEntire: TCheckBox
    Left = 8
    Top = 128
    Width = 145
    Height = 17
    Caption = 'Entire Text'
    TabOrder = 4
  end
  object chkSelOnly: TCheckBox
    Left = 176
    Top = 80
    Width = 161
    Height = 17
    Caption = 'Selection Only'
    TabOrder = 5
  end
  object chkWords: TCheckBox
    Left = 176
    Top = 104
    Width = 153
    Height = 17
    Caption = 'Whole Words'
    TabOrder = 6
  end
  object txtReplace: TEdit
    Left = 64
    Top = 40
    Width = 279
    Height = 21
    BevelKind = bkFlat
    BorderStyle = bsNone
    TabOrder = 1
  end
  object btnReplace: TButton
    Left = 104
    Top = 160
    Width = 81
    Height = 25
    Caption = 'Replace'
    TabOrder = 8
    OnClick = btnReplaceClick
  end
end
