# tile-renderer @METOCEAN

Project: backend for rendering and serving PNG/WEBP tiles from NetCDF files.

URL contains the following parameters:

- `dataset`: name of the dataset to render
- `instance`: time when the dataset was collected/processed
- `variable`: name of the variable in the dataset to render
- `time`: time step from the dataset to render
- `z`: zoom level of the tile
- `x`: x coordinate of the tile
- `y`: y coordinate of the tile
- `style`: style of the layer to render

`x/y/z` are the standard [Slippy Map](https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames) tile coordinates.

# third party projects used
- LuPNG: https://github.com/jansol/LuPng
- nlohmann json.hpp: https://github.com/nlohmann/json
- grody:  simple HTTP server. proprietary MetOcean project.

# end of life

This project is no longer maintained. It has been replaced by tiles preprocessing (private project **Splitter**) and realtime tile rendering in browsers [DEMO](https://metoceanapi.github.io/wxtiles-mbox/##ecmwf.global/air.humidity.at-2m/2023-07-16T12:00:00Z)
[(source on github: wxtiles-mbox)](https://github.com/metoceanapi/wxtiles-mbox)
