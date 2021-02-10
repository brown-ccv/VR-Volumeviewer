function [] = createVolumeViewerData(OsomGridFile,OsomDataFile, OutputFolder, dataDescriptor)
%CREATEVOLUMEVIEWERDATA Creates a volume from OSOM data to be used for the volumeviewer
%   Detailed explanation goes here
% On Input:
%
%    OsomGridFile	nc-file containing the layout for the grid, e.g. osom_grid4_mindep_smlp_mod7.nc
%
%    OsomDataFile   nc file containing the data for the viewer, e.g. ocean_his_0365.nc
%                    
%    dataDescriptor description of the data in the nc-file, e.g 'temp'
%
%    OutputFolder	Outputfolder in which the data is generated

    verticalLevels = 15;
    downscaleFactor = 2;
    
    h = ncread(OsomGridFile,'h');
    x_rho = ncread(OsomGridFile,'x_rho');
    y_rho = ncread(OsomGridFile,'y_rho');
    %lon_rho = ncread(OsomGridFile,'lon_rho');
    %lat_rho = ncread(OsomGridFile,'lat_rho');
    mask_rho = ncread(OsomGridFile,'mask_rho');

    data = ncread(OsomDataFile,dataDescriptor);

    %compute ROMS vertical stretched coordinates
    zeta = ncread(OsomDataFile,'zeta');
    Vtransform = ncread(OsomDataFile,'Vtransform');
    Vstretching = ncread(OsomDataFile,'Vstretching');
    theta_s = ncread(OsomDataFile,'theta_s');
    theta_b = ncread(OsomDataFile,'theta_b');
    hc = ncread(OsomDataFile,'hc');
    ocean_time = ncread(OsomDataFile,'ocean_time');
    [z,s,C] = scoord_du_new2(h, zeta(:,:,1), Vtransform, Vstretching, theta_s, theta_b, hc, verticalLevels, 0);

    %getting minimal and maximum depth
    min_z = floor(min(min(min(z))));
    max_z = ceil(max(max(max(z))));

    
    %set nbSlices (per meter)
    query_depths = [min_z:max_z];
    slices = size(query_depths,2);
    
    %getting minimum and max data 
    min_data = floor(min(min(min(min(data)))));
    max_data = floor(max(max(max(max(data)))));

    %scale data to the range of 0 and 1
    data = (data - min_data)/(max_data - min_data);

    [OsomDataFilePath,OsomDataFileName,OsomDataFileExt] = fileparts(OsomDataFile);
    if ~exist(OutputFolder, 'dir')
       mkdir(OutputFolder)
    end
    
    if ~exist([OutputFolder '/data'], 'dir')
       mkdir([OutputFolder '/data'])
    end
    
    start_time = datetime('2006-01-01 00:00:00','InputFormat','yyyy-MM-dd HH:mm:ss');
         
    %writing datafiles
    for time = 1:size(data,4)
        %intiialize with zero
        outData = zeros(slices, size(x_rho,2), size(x_rho,1));
        for x = 1:size(x_rho,1)
            for y = 1:size(x_rho,2)
                %get depth column
                t_data = squeeze(data(x,y,:,time));
                %if data is set
                idx = ~isnan(t_data);   
                if(sum(idx) > 0)
                    %read set depth
                    d  = z(idx,x,y);
                    %read values
                    val  = t_data(idx);
                    %interpolate at the different query depths
                    t_new = interp1(d,val,query_depths, 'linear', 0);
                    outData(:,y,x) = t_new;
                end
            end
        end
        outData = outData(1:downscaleFactor:end,1:downscaleFactor:end,1:downscaleFactor:end); 
        outData = permute(outData,[3 2 1]) ;
        
        file = fopen([OutputFolder '/data/' dataDescriptor '_' OsomDataFileName '_timestep' int2str(time) '.raw'],'w');
        fwrite(file,outData,'float32');
        fclose(file);

        file = fopen([OutputFolder '/data/' dataDescriptor '_' OsomDataFileName '_timestep' int2str(time) '.desc'],'w');
        fprintf(file,'%u,%u,%u,%.6f,%.6f\n',size(outData,1),size(outData,2),size(outData,3), min_data,max_data );
        current_time = start_time + seconds(ocean_time(time)) ;
        fprintf(file,'%i\n',posixtime(current_time));
        fprintf(file,'%s\n',datestr(current_time,'mm/dd/yyyy-HH:MM:SS'));
        fclose(file);
    end
    
    file = fopen([OutputFolder '/' dataDescriptor '_' OsomDataFileName '.txt'],'w');
    fprintf(file,'animated\n');
    for time = 1:size(data,4)
        fprintf(file,'%s\n', ['volume ' 'data/' dataDescriptor  '_' OsomDataFileName '_timestep' int2str(time) '.desc 1 1 5 0 0 0 raycast 1' ]);
    end
    fclose(file);
end

