program SImResBuilder;

{$mode objfpc}{$H+}

uses
  {$IFDEF UNIX}{$IFDEF UseCThreads}
  cthreads,
  {$ENDIF}{$ENDIF}
  Classes, SysUtils, CustApp
  { you can add units after this };

type

  { TSImResBuilder }

  TSImResBuilder = class(TCustomApplication)
  protected
    procedure DoRun; override;
  public
    constructor Create(TheOwner: TComponent); override;
    destructor Destroy; override;
    procedure WriteHelp; virtual;
  end;

{ TSImResBuilder }

procedure TSImResBuilder.DoRun;
var
  ErrorMsg: String;
  Source, Dest: TStrings;
  i: Integer;
begin
  // quick check parameters
  ErrorMsg:=CheckOptions('h','help');
  if ErrorMsg<>'' then begin
    ShowException(Exception.Create(ErrorMsg));
    Halt;
  end;

  // parse parameters
  if HasOption('h','help') then begin
    WriteHelp;
    Halt;
  end;

  { add your program here }
  Source := TStringList.Create;
  Source.LoadFromFile('Images.txt');
  
  for i := Pred(Source.Count) downto 0 do
      begin
            if Trim(Source[i]) = '' then
               Source.Delete(i);
      end;
  
  Dest := TStringList.Create;

  Dest.Add('.section "Picture resource table" free');
  Dest.Add('Picture_Resource_Table:');
  for i := 0 to Pred(Source.Count) do
      Dest.Add(Format('.dstruct Tab_Pic_%0:s, SAM_Resource, 0, :Pic_%0:s, Pic_%0:s, (Pic_%0:s_End - Pic_%0:s) ; Picture %1:d', [Source[i], i]));
  Dest.Add('.ends');
  Dest.Add('');

  Dest.Add('.slot 2');
  Dest.Add('');
  for i := 0 to Pred(Source.Count) do
      begin
           Dest.Add(
                    Format('.section "Picture %1:d: %0:s" superfree'#13#10 +
                           'Pic_%0:s:'#13#10 +
                           '.dstruct Pic_%0:s_Header, SAM_Picture_Header, Pic_%0:s_Tiles, Pic_%0:s_Tile_Numbers, Pic_%0:s_Palette'#13#10 +
                           'Pic_%0:s_Tiles:'#13#10 +
                           '.include "%0:s (tiles).inc"'#13#10 +
                           'Pic_%0:s_Tile_Numbers:'#13#10 +
                           '.include "%0:s (tile numbers).inc"'#13#10 +
                           'Pic_%0:s_Palette:'#13#10 +
                           '.include "%0:s (palette).inc"'#13#10 +
                           'Pic_%0:s_End'#13#10 +
                           '.ends'#13#10,
                           [Source[i], i])
           );
      end;

  Dest.SaveToFile('Images.txt.inc');
  

  // stop program loop
  Terminate;
end;

constructor TSImResBuilder.Create(TheOwner: TComponent);
begin
  inherited Create(TheOwner);
  StopOnException:=True;
end;

destructor TSImResBuilder.Destroy;
begin
  inherited Destroy;
end;

procedure TSImResBuilder.WriteHelp;
begin
  { add your help code here }
  writeln('Usage: ',ExeName,' -h');
end;

var
  Application: TSImResBuilder;
begin
  Application:=TSImResBuilder.Create(nil);
  Application.Title:='S.A.M. Image Resource Builder';
  Application.Run;
  Application.Free;
end.

