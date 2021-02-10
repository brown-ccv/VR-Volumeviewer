%This is a snipplet script to read elevation data from google and to create
%a mesh for the OSOM model

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

%Read elevation from google if required set the API_KEY to your own. !!!!It
%takes a long time and might also cost some money so use carfully !!!!
%The api also needs to be enabled for the API_KEY

%%Query data for each datapoint
%Z = zeros(size(lat_rho));
%for x = 1:size(lat_rho,1)           
%    response = getElevations(squeeze(lat_rho(x,:)),squeeze(lon_rho(x,:)),'key', 'API_KEY');
%    Z(x,:)= response;  
%end
%%save data
%save('OSOM/elevationFromGoogle.mat','Z');

%load data from file
load('OSOM/elevationFromGoogle.mat')

%filter elevation
Z = medfilt2(Z);
Z = imgaussfilt(Z);

%Merge with OSOM bathymetry data
ind2 = find(bathymetry < 5);
bathymetry(ind2) = -Z(ind2);

%%display
%[X,Y] = meshgrid(1:size(bathymetry,2),1:size(bathymetry,1));
%mesh(X,Y,bathymetry)

%downscale
A2 = imresize(bathymetry, 1.0/downscaleFactor);

%create mesh and read texture geodata
[X,Y] = meshgrid(1:size(A2,2),1:size(A2,1));
[map,raster] = readgeoraster('OSOM/OSOM_texture.tif');
[row, col] = latlon2pix(raster, lat_rho, lon_rho);

%resize texture coordinates 
row = imresize(row, 1.0/downscaleFactor);
col = imresize(col, 1.0/downscaleFactor);

%scale texture coordinates to 0 - 1
row = row/size(map,1);
col = col/size(map,2);

saveobjmesh('test.obj',Y,X, -A2 * 0.5 + 55,row,col);
%Note for further processing of the mesh the texture need to be applied in
%meshlab for this do the following:
% 1) Rotate the texture(OSOM/OSOM_texture.tif) 90 degree counterclockwise
% and save as png
% 1b) Optional : Remove the Seafloor coloring by changing it in Photoshop to white (select using the magicwand tool and fill with white)
% 2) Open the obj in Meshlab and apply the texture using : Filter ->
% Texture -> add Texture
% 3) Save the model with the texture


