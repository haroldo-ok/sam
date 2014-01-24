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
  Ident: String;
  i: Integer;
begin
  // quick check parameters
  ErrorMsg:=CheckOptions('h','help');
  if ErrorMsg<>'' then begin
    ShowException(Exception.Create(ErrorMsg));
    Halt;
  end;

  // parse parameters
  if HasOption('h','help') or (ParamCount < 1) then begin
    WriteHelp;
    Halt;
  end;

  { add your program here }
  Source := TStringList.Create;
  Source.LoadFromFile(ParamStr(1));
  
  for i := Pred(Source.Count) downto 0 do
      begin
            if Trim(Source[i]) = '' then
               Source.Delete(i);
      end;
  
  Dest := TStringList.Create;
  
  Ident := ParamStr(1);
  Ident := Copy(Ident, 1, Pos('.', Ident)-1);

  Dest.Add('.slot 0');
  Dest.Add('');
  Dest.Add(Format('.section "%s resource table" free', [Ident]));
  Dest.Add(Format('%s_Resource_Table:',[Ident]));
  for i := 0 to Pred(Source.Count) do
      Dest.Add(Format('.dstruct Tab_%2:s_%0:s, SAM_Resource, 0, :%2:s_%0:s, %2:s_%0:s, (%2:s_%0:s_End - %2:s_%0:s) ; %2:s %1:d', [StringReplace(Source[i], '.', '_', [rfReplaceAll]), i, Ident]));
  Dest.Add('.ends');
  Dest.Add('');

  Dest.Add(Format('.slot %s', [ParamStr(2)]));
  Dest.Add('');
  for i := 0 to Pred(Source.Count) do
      begin
           Dest.Add(
                    Format('.section "%2:s %1:d: %0:s" superfree'#13#10 +
                           '%2:s_%3:s:'#13#10 +
                           '.incbin "%0:s"'#13#10 +
                           '%2:s_%3:s_End:'#13#10 +
                           '.ends'#13#10,
                           [Source[i], i, Ident, StringReplace(Source[i], '.', '_', [rfReplaceAll])])
           );
      end;

  Dest.SaveToFile(ParamStr(1) + '.inc');
  

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
  writeln('Usage: ',ExeName,' -h                           For help');
  writeln('       ',ExeName,' list_file.txt slot_number    For generating a resource table');
end;

var
  Application: TSImResBuilder;
begin
  Application:=TSImResBuilder.Create(nil);
  Application.Title:='S.A.M. Resource Builder';
  Application.Run;
  Application.Free;
end.

