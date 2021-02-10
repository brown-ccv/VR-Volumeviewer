This is the function to export the texture using google earth engine. To use it sign in to an account for google earth engine (https://earthengine.google.com/) and use the google earth engine code interface(https://code.earthengine.google.com/). Simply run the code and start the task to save the texture to google drive.



/**
 * Function to mask clouds based on the pixel_qa band of Landsat 8 SR data.
 * @param {ee.Image} image input Landsat 8 SR image
 * @return {ee.Image} cloudmasked Landsat 8 image
 */
function maskL8sr(image) {
  // Bits 3 and 5 are cloud shadow and cloud, respectively.
  var cloudShadowBitMask = (1 << 3);
  var cloudsBitMask = (1 << 5);
  // Get the pixel QA band.
  var qa = image.select('pixel_qa');
  // Both flags should be set to zero, indicating clear conditions.
  var mask = qa.bitwiseAnd(cloudShadowBitMask).eq(0)
                 .and(qa.bitwiseAnd(cloudsBitMask).eq(0));
  return image.updateMask(mask);
}

var dataset = ee.ImageCollection('LANDSAT/LC08/C01/T1_SR')
                  .filterDate('2016-01-01', '2016-12-31')
                  .map(maskL8sr);

var visParams = {
  bands: ['B4', 'B3', 'B2'],
  min: 0,
  max: 3000,
  gamma: 1.4,
};

Map.setCenter( -71.4495,41.4501, 8);
Map.addLayer(dataset.median(), visParams);

var rect = ee.Geometry.Rectangle([[-72.7,40.4], [-69.9,42.2]]);

Map.addLayer(rect, {color: 'FF0000'}, 'planar polygon');

//.convert to 8bit rgb image
var scaled = dataset.median().select(['B4', 'B3', 'B2']).unitScale(0, 3000).multiply(255).toByte();

// Export the image, specifying scale and region.
Export.image.toDrive({
  image: scaled,
  description: 'imageToDriveExample',
  scale: 60,
  region: rect
});

