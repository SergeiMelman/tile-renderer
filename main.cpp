// main.cpp
#include "grody/webserver.h"
void legend_handler(struct client * client);
void tile_handler(struct client * client);
int on_connection_created(struct client * client);
void on_connection_destruction(struct client * client);

#include "datafile.h"
#include "tile.h"
#include "dataquery.h"
#include "types.h"
#include <string>

void
testTile()
{
    DataFile ncfile;
    ncfile.load("gfs20170124_18z.nc");
    // Get Latlon using File (cached)
    const LonLat & latlon = ncfile.getLonLat();
    const TileCoords tile = TileCoords::make(latlon, {3, 4, 5});
}

void
testGetVariableData()
{
    DataFile ncfile;
    ncfile.load("gfs20170124_18z.nc");

    auto variableData = ncfile.getVariableData({"UGRD_10maboveground", "VGRD_10maboveground"}, 2);
}

void
handlersTest()
{
    struct client client = {};

    extern const char * const j_str;
    client.req.body = j_str;
    client.req.url = "/v1/wxtiles/tile/ncep-gfs-global-temp2m/temperature-msl-classic/"
                     "20170124_18/2017-01-25T00:00:00Z/0/3/2/4.png";

    on_connection_created(&client);

    tile_handler(&client);

    on_connection_destruction(&client);
}

#include <chrono>
#include "font.h"

void
fontTest()
{
    constexpr int WIDTH = 160;
    constexpr int HEIGHT = 44;

    unsigned char image[HEIGHT][WIDTH] = {};

    Font font = Font::make("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 25);
    auto start = std::chrono::system_clock::now();
    for(int ik = 0; ik < 1; ++ik) {
        auto gls = font.makeString(L"Serg++");
        for(auto & gl : gls.glyphs) {
            FT_Bitmap * bitmap = &gl->bitmap;
            FT_Int x = gl->left;
            FT_Int y = HEIGHT - gl->top;
            // draw bitmap
            for(int i = x, p = 0, x_max = x + bitmap->width; i < x_max; i++, p++)
                for(int j = y, q = 0, y_max = y + bitmap->rows; j < y_max; j++, q++) {
                    if(i < 0 || j < 0 || i * 2 + 1 >= WIDTH || j >= HEIGHT)
                        continue;
                    image[j][i * 2] = bitmap->buffer[q * bitmap->width + p];
                    image[j][i * 2 + 1] = bitmap->buffer[q * bitmap->width + p];
                }
        }
        // show image
        for(int i = 0; i < HEIGHT; i++) {
            for(int j = 0; j < WIDTH; j++)
                putchar(image[i][j] == 0 ? ' ' : image[i][j] < 128 ? '+' : '*');
            putchar('\n');
        }
        memset(image, 0, sizeof(image));
    }
    std::chrono::duration<double> diff = std::chrono::system_clock::now() - start;
    std::cout << "Rendering time: " << diff.count() << std::endl;
}

void
exceptionTest()
{
    try {
        throw std::runtime_error("osaidjfasodijf;dsajf");
    } catch(...) {
        std::exception_ptr p = std::current_exception();
        if(p)
            std::cout << p.__cxa_exception_type()->name();
    }
    try {
        ;
    } catch(const std::exception & e) {
        std::cout << "Catch! + " << e.what() << std::endl;
    }
}

#include <thread>
#include "tools.h"
#include "types.h"
#include "clut.h"
int
main(int argc, char * argv[])
{
    {
        //        float ss = argc + 1.0f/argv[0][0];
        //        return int(ss - int(ss));

        //    std::thread::get_id();

        //    testTile();
        //    testGetVariableData();
        //    handlersTest();
        //    fontTest();
        //    exceptionTest();

        //    RGBA c2{0, 0, 0, 1};
        //    RGBA c1;
        //    //c2.blend(c1);
        //    c2.r = argv[0][0];
        //    c1 = ~c2;

        //        std::vector<int> v(1000000);
        //        std::vector<int> v1(1000000, 0);
        //        for(auto &i : v)
        //            i = rand();
        //        auto start = std::chrono::system_clock::now();
        //        auto pb = v1.data();
        //        for(int t = 20000; t--;) {
        //            auto p = pb;
        //            for(auto d : v)
        //                *p++ += (d * d);
        //        }
        //        std::chrono::duration<double> diff = std::chrono::system_clock::now() - start;
        //        std::cout << "Rendering time: " << diff.count() << std::endl;
        //        std::cout << v1[argc] << std::endl;
        //        return 0;

        //        RGBA a{1, 2, 3, 4}, b{};
        //        const int cnt = 8000000;
        //        auto start = std::chrono::system_clock::now();
        //        for(int i = cnt*100; --i;) {
        //            b.blend({a.r, a.g--, a.b, a.a++});
        //            //b.blendFast(a);a.g--; a.a++;

        //        }
        //        std::chrono::duration<double> diff = std::chrono::system_clock::now() - start;
        //        std::cout << "Rendering time: " << diff.count() << std::endl;
        //        std::cout << cnt << " " << b.r << b.g << b.b << b.a << std::endl;
        //        return 0;
    }

    struct request_handler handlers[] = {{"GET", "/v1/wxtiles/legend/", legend_handler},
        {"GET", "/v1/wxtiles/tile/", tile_handler}, {}};

    struct server_settings settings = {};
    server_settings_defaults(&settings);
    settings.client_body_max_size = 1024 * 1024;
    settings.port = (argc == 2) ? std::stoi(argv[1]) : (argc == 3) ? std::stoi(argv[2]) : 8085;

    std::cout << "Started on port:" << settings.port << std::endl;

    if(run_webserver_forever(single_thread_mode /*thread_mode*/ /*fork_mode*/, &settings,
           handlers, on_connection_created, on_connection_destruction)) {
        return 1;
    }

    return 0;
}
