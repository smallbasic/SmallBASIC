object frmView: TfrmView
  Left = 252
  Top = 212
  Width = 615
  Height = 358
  Caption = 'Viewer'
  Color = clBtnFace
  Font.Charset = GREEK_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Verdana'
  Font.Style = []
  OldCreateOrder = False
  Position = poOwnerFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 607
    Height = 292
    Align = alClient
    BevelOuter = bvNone
    BorderWidth = 6
    Caption = 'Panel1'
    TabOrder = 0
    object txtText: TRichEdit
      Left = 6
      Top = 6
      Width = 595
      Height = 280
      Align = alClient
      Color = clInfoBk
      Font.Charset = GREEK_CHARSET
      Font.Color = clWindowText
      Font.Height = -12
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      PlainText = True
      ScrollBars = ssBoth
      TabOrder = 0
      WordWrap = False
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 292
    Width = 607
    Height = 32
    Align = alBottom
    BevelOuter = bvNone
    BorderWidth = 6
    TabOrder = 1
    object btnClose: TButton
      Left = 110
      Top = 2
      Width = 89
      Height = 25
      Caption = 'Close'
      Default = True
      ModalResult = 2
      TabOrder = 0
    end
    object btnSave: TButton
      Left = 6
      Top = 2
      Width = 89
      Height = 25
      Caption = 'Save'
      Default = True
      TabOrder = 1
      OnClick = btnSaveClick
    end
  end
end
