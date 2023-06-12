% Attention ici on calcule la densité en chargeant les données issues de sigA
% on sauvegarde les densités dans un fichier de type sigB sans l'extension sigB
clear;
tic
Racine='E:\RogerCerf\NonLin30SEP2018\P15\';
Liste_SIGA6=filesinside(Racine,{'SIGA'});
Liste_sigA6=filesinside(Racine,{'sigA'});
ListeSIGA=[Liste_SIGA6;Liste_sigA6];

nbSIGA=numel(ListeSIGA);
clear Liste_SIGA6  Liste_sigA6  Racine
toc
debut=now;
cnt_fait=0;
for numSIGA=1:nbSIGA
    nomfichierIN=ListeSIGA{numSIGA};
    % fabrication du nom de sortie
    [cheminIN,nomIN,extIN]=fileparts(nomfichierIN);
    kcs=findstr(cheminIN,'\');
    cheminOUT=cheminIN(1:kcs(end));
    nomOUT=nomIN;
    nomfichierOUT=[cheminOUT nomOUT];
    if ~exist(nomfichierOUT,'file')
    cnt_fait=cnt_fait+1;
    % lecture du fichier d'entrée (sigA)
    fd=fopen(nomfichierIN,'r');
    for i=1:5
        tline = fgetl(fd);
    end;
    memo=ftell(fd);
    cnt=0;
    while ~feof(fd)
        tline = fgetl(fd);
        cnt=cnt+1;
    end;
    fseek(fd,memo,'bof');
    z=eval(['[' tline ']']);
    data=zeros(numel(z),cnt);
    for i=1:cnt
        tline = fgetl(fd);
        data(:,i)=eval(['[' tline ']']);
    end;
    fclose(fd);
    % j'ai toutes les données en mémoire
    % je calcul la densité
    for i=1:cnt
        data(:,i)=data(:,i)./data(end,i);
        data(:,i)=round(data(:,i)*320*320/2);% compatible avec les anciennes données
    end;
    data(137,:)=[];
    % sauvegarde des densités (sigB)
    fd=fopen(nomfichierOUT,'wb');
    fwrite(fd,data,'int32');
    fclose(fd);
    maintenant=now;
    fait_t=maintenant-debut;
    temps_un_fichier=fait_t/cnt_fait;
    temps_total_estime=temps_un_fichier*nbSIGA;
        disp([datestr(maintenant) '  -end-> ' datestr(debut+temps_total_estime) '   ' nomfichierOUT]);
    else
        disp([datestr(now) '  ' nomfichierOUT '  existe déjà']);
    end;
end;





