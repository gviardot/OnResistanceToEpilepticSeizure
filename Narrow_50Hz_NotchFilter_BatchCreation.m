clear all;
fclose all;
close all;
tic;
%% filtrage et creation batch

% paramètres du filtre

frqech=256;%Hz
n = 5;
r = 12;
Wst = [49.75 50.25]/(frqech/2);
[b,a] = cheby2(n,r,Wst,'stop');

nbcore=6;% depend on your microprocessor
% préparation Batch file
for numbatch=1:nbcore
    nomout{numbatch}=['C:\MainDir\NonLin12AUG2017\AutoBatch_' num2str(numbatch) '_' datestr(now,30) '.bat'];
    fdbatch(numbatch)=fopen(nomout{numbatch},'w');
end;

listenump=[8 17 19];
for numtest=1:3
    
nump=listenump(numtest);
% define data to process
    racine=['C:\MainDir\NonLin12AUG2017\P' num2str(nump)];
    disp(racine);
    toc
    tout=filesinside(racine);
    cntfile=0;
    for numfile=1:numel(tout)
        if numel(strfind(lower(tout{numfile}),'.sig'))==1
            if exist(tout{numfile},'file')==2   % c'est un fichier
                % lecture fichier *.sig
                nomIN=tout{numfile};
                fd=fopen(nomIN,'r');
                signal=fread(fd,inf,'int16');
                fclose(fd);
                % filtrage du 50Hz
                signalfiltre = filtfilt(b,a,signal);
                % normalisation du signal
                signalfiltre=floor(signalfiltre);
                signalfiltre=signalfiltre-min(signalfiltre);
                signalfiltre=(2^16-1)*signalfiltre./max(signalfiltre);
                signalfiltre=signalfiltre-(2^15);
                % sauvegarde du signal filtré
                nomOUT=[nomIN(1:end-4) '_FCB4975Hz5025Hz.sig'];
                fd=fopen(nomOUT,'w');
                fwrite(fd,signalfiltre,'int16');
                fclose(fd);
                % ajout ligne dans un des batch
                fprintf(fdbatch(1+mod(cntfile,nbcore)),'C:\\MainDir\\NonLin12AUG2017\\Matlab_AtypicBackUp_CerfAndMore\\Intgp1dimFLOAT %s 15 16 1 1\n',nomOUT);
                cntfile=cntfile+1;
            end;
        end;
    end;
end;    
for numbatch=1:nbcore
    fclose(fdbatch(numbatch));
end;

disp('fin');
toc
