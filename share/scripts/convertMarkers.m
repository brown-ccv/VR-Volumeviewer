%This is a snipplet script to convert latitute / longitude landmarks into
%the OSOM coordinates

%OSOM File used
OsomGridFile = 'OSOM/osom_grid4_mindep_smlp_mod7.nc'
OsomDataFile = 'OSOM/ocean_his_0365.nc'
OsomDataFilebathyFile = 'OSOM/ngbay_grd.nc';   

%This needs to be the same as from the data generator
downscaleFactor = 2;

%Read Lat Lon positions and bathymetry 
lon_rho = ncread(OsomGridFile,'lon_rho');
lat_rho = ncread(OsomGridFile,'lat_rho');
bathymetry = ncread(OsomDataFilebathyFile,'h');   

%compute Mesh position
[X,Y] = meshgrid(1:size(lat_rho,2),1:size(lat_rho,1));

%reshape arrays for interpolation
inter_lat = reshape(lat_rho,1,size(lat_rho,1) * size(lat_rho,2) );
inter_lon = reshape(lon_rho,1,size(lon_rho,1) * size(lon_rho,2) );
inter_X = reshape(X,1,size(X,1) * size(X,2) );
inter_Y = reshape(Y,1,size(Y,1) * size(Y,2) );

%create scattered interpolant for lat lon
F_y = scatteredInterpolant(inter_lat',inter_lon',inter_X');
F_x = scatteredInterpolant(inter_lat',inter_lon',inter_Y');

%create gridded interpolant for bathymetry
F_z=griddedInterpolant(bathymetry); 

%Providence
coord_x = 41.8240;
coord_Y = -71.4128;
x = F_x (coord_x,coord_Y); 
y = F_y (coord_x,coord_Y); 
z = interp2(1:size(bathymetry,1),1:size(bathymetry,2), bathymetry', y, x);
[x/2 y/2 z/2+55]

%New York
coord_x = 40.7128;
coord_Y = -74.0060;
x = F_x (coord_x,coord_Y);
y = F_y (coord_x,coord_Y); 
z = interp2(1:size(bathymetry,1),1:size(bathymetry,2), bathymetry', y, x);
[x/2 y/2 z/2+55]

%Narragansett
coord_x = 41.4501;
coord_Y = -71.4495;
x = F_x (coord_x,coord_Y); 
y = F_y (coord_x,coord_Y); 
z = interp2(1:size(bathymetry,1),1:size(bathymetry,2), bathymetry', y, x);
[x/2 y/2 z/2+55]

%Block Island
coord_x = 41.1617;
coord_Y = -71.5843;
x = F_x (coord_x,coord_Y); 
y = F_y (coord_x,coord_Y); 
z = interp2(1:size(bathymetry,1),1:size(bathymetry,2), bathymetry', y, x);
[x/2 y/2 z/2+55]

%gARDINERS Island
coord_x = 41.0944;
coord_Y = -72.1001;
x = F_x (coord_x,coord_Y); 
y = F_y (coord_x,coord_Y); 
z = interp2(1:size(bathymetry,1),1:size(bathymetry,2), bathymetry', y, x);
[x/2 y/2 z/2+55]    
%New London
coord_x = 41.3557;
coord_Y = -72.0995;
x = F_x (coord_x,coord_Y); 
y = F_y (coord_x,coord_Y); 
z = interp2(1:size(bathymetry,1),1:size(bathymetry,2), bathymetry', y, x);
[x/2 y/2 z/2+55]    

%New Bedford
coord_x = 41.6362;
coord_Y = -70.9342;
x = F_x (coord_x,coord_Y); 
y = F_y (coord_x,coord_Y); 
z = interp2(1:size(bathymetry,1),1:size(bathymetry,2), bathymetry', y, x);
[x/2 y/2 z/2+55]  

%martha's vineyard
coord_x = 41.3805;
coord_Y = -70.6455;
x = F_x (coord_x,coord_Y); 
y = F_y (coord_x,coord_Y); 
z = interp2(1:size(bathymetry,1),1:size(bathymetry,2), bathymetry', y, x);
[x/2 y/2 z/2+55]  

%Fall River 
coord_x = 41.7015;
coord_Y = -71.1550;
x = F_x (coord_x,coord_Y); 
y = F_y (coord_x,coord_Y); 
z = interp2(1:size(bathymetry,1),1:size(bathymetry,2), bathymetry', y, x);
[x/2 y/2 z/2+55]  





