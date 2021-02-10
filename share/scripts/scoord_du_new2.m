function [z,s,C]=scoord_du_new2(h, zeta, Vtransform, Vstretching,theta_s, theta_b, hc,N, kgrid);
%
% SCOORD:  Compute and plot ROMS vertical stretched coordinates
%
% [z,s,C]=scoord_du_new2(h, zeta, Vtransform, Vstretching, theta_s, theta_b, hc, N, kgrid)
%
% Given a batymetry (h), surface elevation (zeta) and terrain-following stretching parameters,
% this function computes the depths of RHO- or W-points for all grid points. Check the
% following link for details:
%
%    https://www.myroms.org/wiki/index.php/Vertical_S-coordinate
%
% On Input:
%
%    h             Bottom depth 2D array, h(1:Lp,1:Mp), m, positive
%    zeta          surface elevation
%    Vtransform    Vertical transformation equation:
%                    Vtransform = 1,   original transformation
%
%                      z(x,y,s,t)=Zo(x,y,s)+zeta(x,y,t)*[1+Zo(x,y,s)/h(x,y)]
%
%                      Zo(x,y,s)=hc*s+[h(x,y)-hc]*C(s)
%
%                    Vtransform = 2,   new transformation
%
%                      z(x,y,s,t)=zeta(x,y,t)+[zeta(x,y,t)+h(x,y)]*Zo(x,y,s)
%
%                       Zo(x,y,s)=[hc*s(k)+h(x,y)*C(k)]/[hc+h(x,y)]
%    Vstretching   Vertical stretching function:
%                    Vstretching = 1,  original (Song and Haidvogel, 1994)
%                    Vstretching = 2,  A. Shchepetkin (UCLA-ROMS, 2005)
%                    Vstretching = 3,  R. Geyer BBL refinement
%                    Vstretching = 4,  A. Shchepetkin (UCLA-ROMS, 2010)
%    theta_s       S-coordinate surface control parameter (scalar)
%    theta_b       S-coordinate bottom control parameter (scalar)
%    hc            Width (m) of surface or bottom boundary layer in which
%                    higher vertical resolution is required during
%                    stretching (scalar)
%    N             Number of vertical levels (scalar)
%    kgrid         Depth grid type logical switch:
%                    kgrid = 0,        depths of RHO-points
%                    kgrid = 1,        depths of W-points
%
% On Output:
%
%    z             Depths (m) of RHO- or W-points (matrix)
%    s             S-coordinate independent variable, [-1 <= s <= 0] at
%                    vertical RHO- or W-points (vector)
%    C             Nondimensional, monotonic, vertical stretching function,
%                    C(s), 1D array, [-1 <= C(s) <= 0]z
%

% svn $Id: scoord.m 754 2015-01-07 23:23:40Z arango $
%===========================================================================%
%  Copyright (c) 2002-2015 The ROMS/TOMS Group                              %
%    Licensed under a MIT/X style license                                   %
%    See License_ROMS.txt                           Hernan G. Arango        %
%===========================================================================%
% 10/4/2018: DU modified scoord.m in ROMS_matlab_11112015 to remove plotting, verbose output, and
%            to allow input of zeta (original version assumed zero zeta).
%            this version does the computation for all gridpoints rather than those along
%            a row or column as the original function did.
z=[];
s=[];
C=[];

%----------------------------------------------------------------------------
%  Set several parameters.
%----------------------------------------------------------------------------

if (hc > min(min(h)) & Vtransform == 1),
  disp(' ');
  disp([setstr(7),'*** Error:  SCOORD - critical depth exceeds minimum' ...
       ' bathymetry value.',setstr(7)]);
  disp([setstr(7),'                     Vtranform = ',num2str(Vtransform),setstr(7)]);
  disp([setstr(7),'                     hc        = ',num2str(hc),setstr(7)]);
  disp([setstr(7),'                     hmax      = ',num2str(min(min(h))), ...
        setstr(7)]);
  disp(' ');
  return
end,

if (Vtransform < 1 | Vtransform > 2),
  disp(' ');
  disp([setstr(7),'*** Error:  SCOORD - Illegal parameter Vtransform = ' ...
        num2str(Vtransfrom), setstr(7)]);
  return
end,

if (Vstretching < 1 | Vstretching > 4),
  disp(' ');
  disp([setstr(7),'*** Error:  SCOORD - Illegal parameter Vstretching = ' ...
        num2str(Vstretching), setstr(7)]);
  return
end,

[Lp Mp]=size(h);
hmin=min(min(h));
hmax=max(max(h));
havg=0.5*(hmax+hmin);

%----------------------------------------------------------------------------
% Compute vertical stretching function, C(k):
%----------------------------------------------------------------------------

[s,C]=stretching(Vstretching, theta_s, theta_b, hc, N, kgrid, 0);

if (kgrid == 1),
  Nlev=N+1;
else,
  Nlev=N;
end,


if (Vtransform == 1),

  for k=Nlev:-1:1,
    zhc(k)=hc*s(k);
    z1 (k)=zhc(k)+(hmin-hc)*C(k);
    z2 (k)=zhc(k)+(havg-hc)*C(k);
    z3 (k)=zhc(k)+(hmax-hc)*C(k);
  end,

elseif (Vtransform == 2),

  for k=Nlev:-1:1,
    if (hc > hmax),
      zhc(k)=hmax*(hc*s(k)+hmax*C(k))/(hc+hmax);
    else
      zhc(k)=0.5*min(hc,hmax)*(s(k)+C(k));
    end,
    z1 (k)=hmin*(hc*s(k)+hmin*C(k))/(hc+hmin);
    z2 (k)=havg*(hc*s(k)+havg*C(k))/(hc+havg);
    z3 (k)=hmax*(hc*s(k)+hmax*C(k))/(hc+hmax);
  end,

end,

%============================================================================
% Compute depths at all gridpoints
%============================================================================

  if (Vtransform == 1),

    z=zeros(Nlev,Lp,Mp);
    for k=1:Nlev,
      z0=hc.*(s(k)-C(k))+h*C(k);
      z(k,:,:)=z0+zeta.*(1.0+z0./h);
    end,

  elseif (Vtransform == 2),

    z=zeros(Nlev,Lp,Mp);
    for k=1:Nlev,
      z0=(hc.*s(k)+C(k).*h)./(h+hc);
      z(k,:,:)=zeta+(zeta+h).*z0;
    end,

  end,

return


